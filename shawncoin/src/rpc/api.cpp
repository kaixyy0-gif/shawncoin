#include "rpc/api.hpp"
#include "core/blockchain.hpp"
#include "core/transaction.hpp"
#include "core/mempool.hpp"
#include "mining/miner.hpp"
#include "wallet/hdwallet.hpp"
#include "wallet/wallet.hpp"
#include "crypto/address.hpp"
#include "util/logger.hpp"
#include "util/util.hpp"
#include <sstream>
#include <fstream>
#include <cstring>
#include <ctime>

// Simple JSON parser for RPC (avoid external dependency)
#include <map>
#include <variant>

namespace shawncoin {

class SimpleJson {
public:
    using value = std::variant<std::string, int64_t, double, bool, std::nullptr_t, std::vector<SimpleJson>, std::map<std::string, SimpleJson>>;
    
    value data;
    
    SimpleJson() : data(nullptr) {}
    SimpleJson(const std::string& s) : data(s) {}
    SimpleJson(int64_t i) : data(i) {}
    SimpleJson(int i) : data(static_cast<int64_t>(i)) {}
    SimpleJson(double d) : data(d) {}
    SimpleJson(bool b) : data(b) {}
    SimpleJson(std::nullptr_t) : data(nullptr) {}
    
    // Assignment operators to resolve ambiguity
    SimpleJson& operator=(const std::string& s) { data = s; return *this; }
    SimpleJson& operator=(int64_t i) { data = i; return *this; }
    SimpleJson& operator=(int i) { data = static_cast<int64_t>(i); return *this; }
    SimpleJson& operator=(uint32_t i) { data = static_cast<int64_t>(i); return *this; }
    SimpleJson& operator=(uint64_t i) { data = static_cast<int64_t>(i); return *this; }
    SimpleJson& operator=(double d) { data = d; return *this; }
    SimpleJson& operator=(bool b) { data = b; return *this; }
    
    // Helper to create objects with initializer lists
    static SimpleJson create_object(std::initializer_list<std::pair<const std::string, SimpleJson>> init) {
        SimpleJson j;
        j.data = std::map<std::string, SimpleJson>(init);
        return j;
    }
    
    static SimpleJson parse(const std::string& json) {
        // Basic parsing - just handles simple RPC calls
        SimpleJson result;
        std::map<std::string, SimpleJson> obj;
        
        // Extract method
        size_t method_pos = json.find("\"method\"");
        if (method_pos != std::string::npos) {
            size_t colon = json.find(":", method_pos);
            if (colon != std::string::npos) {
                size_t start = json.find("\"", colon);
                if (start != std::string::npos) {
                    size_t end = json.find("\"", start + 1);
                    if (end != std::string::npos) {
                        std::string method = json.substr(start + 1, end - start - 1);
                        obj["method"] = method;
                    }
                }
            }
        }
        
        result.data = obj;
        return result;
    }
    
    bool contains(const std::string& key) const {
        if (auto* obj = std::get_if<std::map<std::string, SimpleJson>>(&data)) {
            return obj->find(key) != obj->end();
        }
        return false;
    }
    
    SimpleJson operator[](const std::string& key) const {
        if (auto* obj = std::get_if<std::map<std::string, SimpleJson>>(&data)) {
            auto it = obj->find(key);
            if (it != obj->end()) return it->second;
        }
        return SimpleJson();
    }
    
    bool is_string() const { return std::holds_alternative<std::string>(data); }
    bool is_object() const { return std::holds_alternative<std::map<std::string, SimpleJson>>(data); }
    bool is_array() const { return std::holds_alternative<std::vector<SimpleJson>>(data); }
    
    template<typename T>
    T get() const {
        if constexpr (std::is_same_v<T, std::string>) {
            if (auto* s = std::get_if<std::string>(&data)) return *s;
        }
        if constexpr (std::is_integral_v<T>) {
            if (auto* i = std::get_if<int64_t>(&data)) return static_cast<T>(*i);
        }
        return T{};
    }
    
    std::string dump() const {
        std::ostringstream oss;
        dump_recursive(oss);
        return oss.str();
    }
    
    static SimpleJson object() {
        SimpleJson j;
        j.data = std::map<std::string, SimpleJson>();
        return j;
    }
    
    static SimpleJson array() {
        SimpleJson j;
        j.data = std::vector<SimpleJson>();
        return j;
    }
    
    size_t size() const {
        if (auto* arr = std::get_if<std::vector<SimpleJson>>(&data)) {
            return arr->size();
        }
        if (auto* obj = std::get_if<std::map<std::string, SimpleJson>>(&data)) {
            return obj->size();
        }
        return 0;
    }
    
    void push_back(const SimpleJson& value) {
        if (auto* arr = std::get_if<std::vector<SimpleJson>>(&data)) {
            arr->push_back(value);
        } else {
            // Convert to array if not already
            data = std::vector<SimpleJson>{value};
        }
    }
    
    SimpleJson operator[](int index) const {
        if (auto* arr = std::get_if<std::vector<SimpleJson>>(&data)) {
            if (index >= 0 && index < static_cast<int>(arr->size())) {
                return (*arr)[index];
            }
        }
        return SimpleJson();
    }
    
private:
    void dump_recursive(std::ostringstream& oss) const {
        if (auto* s = std::get_if<std::string>(&data)) {
            oss << '"' << *s << '"';
        } else if (auto* i = std::get_if<int64_t>(&data)) {
            oss << *i;
        } else if (auto* d = std::get_if<double>(&data)) {
            oss << *d;
        } else if (auto* b = std::get_if<bool>(&data)) {
            oss << (*b ? "true" : "false");
        } else if (std::holds_alternative<std::nullptr_t>(data)) {
            oss << "null";
        } else if (auto* arr = std::get_if<std::vector<SimpleJson>>(&data)) {
            oss << "[";
            for (size_t i = 0; i < arr->size(); ++i) {
                if (i > 0) oss << ",";
                (*arr)[i].dump_recursive(oss);
            }
            oss << "]";
        } else if (auto* obj = std::get_if<std::map<std::string, SimpleJson>>(&data)) {
            oss << "{";
            bool first = true;
            for (const auto& [k, v] : *obj) {
                if (!first) oss << ",";
                oss << '"' << k << "\":";
                v.dump_recursive(oss);
                first = false;
            }
            oss << "}";
        }
    }
};

using json = SimpleJson;

std::string apiBlockchainInfo(RpcContext* ctx) {
    if (!ctx || !ctx->chain) return "{}";
    std::ostringstream out;
    out << "{\"chain\":\"shawncoin\",\"blocks\":" << ctx->chain->getHeight()
        << ",\"bestblockhash\":\"" << uint256ToHex(ctx->chain->getBestBlockHash())
        << "\",\"mempool_size\":" << (ctx->mempool ? ctx->mempool->size() : 0) << "}";
    return out.str();
}

std::string apiBlockByHash(const std::string& hash, RpcContext* ctx) {
    if (!ctx || !ctx->chain) return "{}";
    uint256 h = hexToUint256(hash);
    auto block = ctx->chain->getBlock(h);
    if (!block) return "{\"error\":\"block not found\"}";
    std::ostringstream out;
    out << "{\"hash\":\"" << hash << "\",\"height\":0,\"txcount\":" << block->transactions.size() << "}";
    return out.str();
}

std::string apiBlockByHeight(uint64_t height, RpcContext* ctx) {
    if (!ctx || !ctx->chain) return "{}";
    auto block = ctx->chain->getBlockByHeight(height);
    if (!block) return "{\"error\":\"block not found\"}";
    std::ostringstream out;
    out << "{\"hash\":\"" << uint256ToHex(block->getHash()) << "\",\"height\":" << height
        << ",\"txcount\":" << block->transactions.size() << "}";
    return out.str();
}

std::string handleRpcRequest(const std::string& request, RpcContext* ctx) {
    json req;
    try {
        req = json::parse(request);
    } catch (const std::exception&) {
        json err;
        err["jsonrpc"] = "2.0";
        err["error"] = SimpleJson::create_object({ {"code", static_cast<int64_t>(-32700)}, {"message", "Parse error"} });
        err["id"] = SimpleJson(nullptr);
        return err.dump();
    }

    json resp;
    resp["jsonrpc"] = "2.0";
    json id = nullptr;
    if (req.contains("id")) id = req["id"];

    if (!req.contains("method") || !req["method"].is_string()) {
        resp["error"] = SimpleJson::create_object({ {"code", static_cast<int64_t>(-32600)}, {"message", "Invalid Request"} });
        resp["id"] = id;
        return resp.dump();
    }

    std::string method = req["method"].get<std::string>();
    // Log incoming RPC method for debugging
    SHAWNCOIN_LOG(Info, "rpc", "Received RPC method: %s", method.c_str());
    json params = json::object();
    if (req.contains("params")) params = req["params"];

    try {
        if (method == "wallet.create") {
            if (!ctx || !ctx->wallet) throw std::runtime_error("no wallet");
            std::string pass = "";
            if (params.is_object() && params.contains("passphrase")) pass = params["passphrase"].get<std::string>();
            bool ok = ctx->wallet->create(pass);
            if (!ok) throw std::runtime_error("failed to create wallet");
            resp["result"] = "wallet created";
            resp["id"] = id;
            return resp.dump();
        }

        if (method == "wallet.getnewaddress") {
            if (!ctx || !ctx->wallet) throw std::runtime_error("no wallet");
            std::string addr = ctx->wallet->generateNewAddress();
            resp["result"] = addr;
            resp["id"] = id;
            return resp.dump();
        }

        if (method == "wallet.listunspent") {
            if (!ctx || !ctx->wallet || !ctx->chain) throw std::runtime_error("no wallet/chain");
            json arr = json::array();
            auto snap = ctx->chain->utxo().snapshot();
            for (const auto& p : snap) {
                const OutPoint& op = p.first;
                const UTXOSet::Entry& e = p.second;
                const Script& s = e.script_pubkey;
                if (s.size() == 25 && s[0] == 0x76 && s[1] == 0xa9 && s[2] == 0x14 && s[23] == 0x88 && s[24] == 0xac) {
                    std::vector<uint8_t> hash160(20);
                    std::copy(s.begin() + 3, s.begin() + 23, hash160.begin());
                    // include only UTXOs that belong to our wallet
                    if (!ctx->wallet->hasHash160(hash160)) continue;
                    json obj;
                    obj["txid"] = uint256ToHex(op.hash);
                    obj["vout"] = op.index;
                    obj["amount"] = e.amount;
                    arr.push_back(obj);
                }
            }
            resp["result"] = arr;
            resp["id"] = id;
            return resp.dump();
        }

        if (method == "wallet.sendtoaddress") {
            if (!ctx || !ctx->wallet || !ctx->chain || !ctx->mempool) throw std::runtime_error("no wallet/chain/mempool");
            if (!params.is_array() || params.size() < 2) throw std::runtime_error("params: [address, amount_satoshis]");
            std::string dest = params[0].get<std::string>();
            uint64_t amount = params[1].get<uint64_t>();
            uint64_t fee = 0;
            if (params.size() >= 3) fee = params[2].get<uint64_t>();

            // gather UTXOs belonging to wallet
            auto snap = ctx->chain->utxo().snapshot();
            struct U { OutPoint op; UTXOSet::Entry e; };
            std::vector<U> utxos;
            for (const auto& p : snap) {
                const OutPoint& op = p.first;
                const UTXOSet::Entry& e = p.second;
                const Script& s = e.script_pubkey;
                if (s.size() == 25 && s[0] == 0x76 && s[1] == 0xa9 && s[2] == 0x14 && s[23] == 0x88 && s[24] == 0xac) {
                    std::vector<uint8_t> h(20);
                    std::copy(s.begin() + 3, s.begin() + 23, h.begin());
                    // check if matches any wallet address
                    // use wallet->generateNewAddress index-based search via hd_
                    // We'll use getBalance-like check: derive addresses up to keys_.size()
                    size_t maxIndex = ctx->wallet->getAddressCount();
                    for (size_t i = 0; i < maxIndex; ++i) {
                        std::string a = ctx->wallet->deriveAddressAt((uint32_t)i);
                        std::vector<uint8_t> hh = addressToPubKeyHash(a);
                        if (hh.size() == 20 && memcmp(hh.data(), h.data(), 20) == 0) {
                            utxos.push_back({op, e});
                            break;
                        }
                    }
                }
            }
            // select UTXOs greedily
            uint64_t total = 0;
            std::vector<U> selected;
            for (const auto& u : utxos) {
                selected.push_back(u);
                total += u.e.amount;
                if (total >= amount + fee) break;
            }
            if (total < amount + fee) throw std::runtime_error("insufficient funds");

            // Build transaction
            Transaction tx;
            tx.version = 1;
            // inputs
            for (const auto& sU : selected) {
                TxInput in;
                in.prev_tx_hash = sU.op.hash;
                in.output_index = sU.op.index;
                tx.inputs.push_back(in);
            }
            // outputs: destination
            TxOutput out;
            out.amount = amount;
            // build P2PKH script for dest
            std::vector<uint8_t> dest_hash = addressToPubKeyHash(dest);
            out.script_pubkey = {0x76, 0xa9, 0x14};
            if (dest_hash.size() == 20) out.script_pubkey.insert(out.script_pubkey.end(), dest_hash.begin(), dest_hash.end()); else out.script_pubkey.insert(out.script_pubkey.end(), 20, 0);
            out.script_pubkey.push_back(0x88); out.script_pubkey.push_back(0xac);
            tx.outputs.push_back(out);

            uint64_t change = total - amount - fee;
            if (change > 0) {
                // send change to first wallet address
                std::string changeAddr = ctx->wallet->generateNewAddress();
                TxOutput c;
                c.amount = change;
                std::vector<uint8_t> ch = addressToPubKeyHash(changeAddr);
                c.script_pubkey = {0x76,0xa9,0x14};
                if (ch.size() == 20) c.script_pubkey.insert(c.script_pubkey.end(), ch.begin(), ch.end()); else c.script_pubkey.insert(c.script_pubkey.end(), 20, 0);
                c.script_pubkey.push_back(0x88); c.script_pubkey.push_back(0xac);
                tx.outputs.push_back(c);
            }

            // sign
            if (!ctx->wallet->signTransaction(tx, &ctx->chain->utxo())) throw std::runtime_error("failed to sign tx");

            // add to mempool
            bool ok = ctx->mempool->add(tx, fee);
            if (!ok) throw std::runtime_error("mempool rejected tx");

            // return tx hex
            std::vector<uint8_t> buf;
            serializeTransaction(tx, buf);
            resp["result"] = shawncoin::hexEncode(buf.data(), buf.size());
            resp["id"] = id;
            return resp.dump();
        }

        if (method == "wallet.getbalance") {
            if (!ctx || !ctx->wallet || !ctx->chain) throw std::runtime_error("no wallet/chain available");
            uint64_t bal = ctx->wallet->getBalance(&ctx->chain->utxo());
            resp["result"] = bal;
            resp["id"] = id;
            return resp.dump();
        }

        if (method == "wallet.backup") {
            if (!ctx || !ctx->wallet) throw std::runtime_error("no wallet");
            if (!params.is_object() || !params.contains("path")) throw std::runtime_error("missing path");
            std::string path = params["path"].get<std::string>();
            ctx->wallet->backup(path);
            resp["result"] = "backup written";
            resp["id"] = id;
            return resp.dump();
        }

        if (method == "wallet.restore") {
            if (!ctx || !ctx->wallet) throw std::runtime_error("no wallet");
            if (!params.is_object() || !params.contains("mnemonic")) throw std::runtime_error("missing mnemonic");
            std::string mnemonic = params["mnemonic"].get<std::string>();
            std::string pass = "";
            if (params.contains("passphrase")) pass = params["passphrase"].get<std::string>();
            bool ok = ctx->wallet->restore(mnemonic, pass);
            if (!ok) throw std::runtime_error("failed to restore");
            resp["result"] = "wallet restored";
            resp["id"] = id;
            return resp.dump();
        }

        if (method == "mining.start") {
            if (!ctx || !ctx->miner) throw std::runtime_error("no miner");
            int t = 1;
            if (params.is_object() && params.contains("threads")) t = params["threads"].get<int>();
            ctx->miner->start((uint32_t)t);
            resp["result"] = "mining started";
            resp["id"] = id;
            return resp.dump();
        }

        if (method == "mining.stop") {
            if (!ctx || !ctx->miner) throw std::runtime_error("no miner");
            ctx->miner->stop();
            resp["result"] = "mining stopped";
            resp["id"] = id;
            return resp.dump();
        }

        if (method == "mining.getstatus") {
            if (!ctx || !ctx->miner) throw std::runtime_error("no miner");
            json s;
            s["isMining"] = ctx->miner->isMining();
            s["hashrate"] = ctx->miner->getHashrate();
            resp["result"] = s;
            resp["id"] = id;
            return resp.dump();
        }

        if (method == "blockchain.info") {
            std::string info = apiBlockchainInfo(ctx);
            try {
                json j = json::parse(info);
                resp["result"] = j;
            } catch (...) {
                resp["result"] = info;
            }
            resp["id"] = id;
            return resp.dump();
        }

        if (method == "mining.getblocktemplate") {
            if (!ctx || !ctx->chain) throw std::runtime_error("no chain");
            json t;
            uint64_t height = ctx->chain->getHeight() + 1;
            std::string prev = shawncoin::uint256ToHex(ctx->chain->getBestBlockHash());
            uint32_t target = shawncoin::EASY_MINE_DIFFICULTY;
            size_t mempool_tx = ctx->mempool ? ctx->mempool->size() : 0;
            t["previous_hash"] = prev;
            t["height"] = height;
            t["version"] = 1;
            t["time"] = (uint64_t)std::time(nullptr);
            t["bits"] = target; // compact representation
            t["target"] = target;
            t["coinbasevalue"] = (uint64_t)getBlockSubsidy(height);
            t["mempool_tx_count"] = mempool_tx;
            // Provide a suggested coinbase payout address (may advance keypool)
            if (ctx->wallet) t["coinbase_address"] = ctx->wallet->generateNewAddress();
            if (ctx->mempool) {
                auto txs = ctx->mempool->getBlockTemplate();
                t["txs"] = json::array();
                for (const auto& tx : txs) {
                    std::vector<uint8_t> buf;
                    serializeTransaction(tx, buf);
                    std::string hx = shawncoin::hexEncode(buf.data(), buf.size());
                    t["txs"].push_back(hx);
                }
            }
            resp["result"] = t;
            resp["id"] = id;
            return resp.dump();
        }

        if (method == "mining.submit") {
            if (!ctx || !ctx->chain) throw std::runtime_error("no chain");
            if (!params.is_object() || !params.contains("blockhex")) throw std::runtime_error("missing blockhex");
            std::string blockhex = params["blockhex"].get<std::string>();
            std::vector<uint8_t> raw = shawncoin::hexDecode(blockhex);
            if (raw.empty()) throw std::runtime_error("invalid hex");
            shawncoin::Block block;
            if (!shawncoin::deserializeBlock(raw.data(), raw.size(), block)) throw std::runtime_error("failed to deserialize block");
            uint64_t submitHeight = ctx->chain->getHeight() + 1;
            bool ok = ctx->chain->addBlock(block, submitHeight);
            if (!ok) {
                resp["result"] = "rejected";
            } else {
                resp["result"] = "accepted";
                resp["height"] = submitHeight;
            }
            resp["id"] = id;
            return resp.dump();
        }

        resp["error"] = SimpleJson::create_object({ {"code", static_cast<int64_t>(-32601)}, {"message", "Method not found"} });
        resp["id"] = id;
        return resp.dump();
    } catch (const std::exception& ex) {
        resp["error"] = SimpleJson::create_object({ {"code", static_cast<int64_t>(-32602)}, {"message", std::string(ex.what())} });
        resp["id"] = id;
        return resp.dump();
    }
}

} // namespace shawncoin

