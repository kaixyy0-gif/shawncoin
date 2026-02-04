#include "wallet/wallet.hpp"
#include "wallet/mnemonic.hpp"
#include "core/utxo.hpp"
#include "crypto/keys.h"
#include "crypto/signatures.h"
#include "crypto/hash.h"
#include "crypto/address.hpp"
#include <fstream>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include "util/util.hpp"
#include <cstring>

namespace shawncoin {

Wallet::Wallet() = default;

bool Wallet::create(const std::string& passphrase) {
    std::lock_guard<std::mutex> lock(mutex_);
    mnemonic_ = generateMnemonic12();
    if (mnemonic_.empty()) return false;
    std::vector<uint8_t> seed = mnemonicToSeed(mnemonic_, passphrase);
    return hd_.setSeed(seed);
}

bool Wallet::restore(const std::string& mnemonic, const std::string& passphrase) {
    std::lock_guard<std::mutex> lock(mutex_);
    mnemonic_ = mnemonic;
    std::vector<uint8_t> seed = mnemonicToSeed(mnemonic_, passphrase);
    return hd_.setSeed(seed);
}

std::string Wallet::generateNewAddress() {
    std::lock_guard<std::mutex> lock(mutex_);
    uint32_t index = (uint32_t)keys_.size();
    std::string addr = hd_.deriveAddress(index);
    if (!addr.empty()) {
        // store the hash160 for quick balance checks
        std::vector<uint8_t> h = shawncoin::addressToPubKeyHash(addr);
        keys_[addr] = h;
    }
    return addr;
}

// previous signTransaction overload removed; use signTransaction(tx, utxo)

// Helper: find private key for an address by scanning derived addresses
static std::vector<uint8_t> findPrivateKeyForAddress(HDWallet& hd, size_t maxIndex, const std::string& address) {
    for (size_t i = 0; i < maxIndex; ++i) {
        std::string a = hd.deriveAddress((uint32_t)i);
        if (a == address) {
            return hd.derivePrivateKey((uint32_t)i);
        }
    }
    return {};
}

bool Wallet::signTransaction(Transaction& tx, const UTXOSet* utxo) {
    if (!utxo) return false;
    // Make a copy with empty sigs for signing
    Transaction tmp = tx;
    for (auto& in : tmp.inputs) {
        in.signature.clear();
        in.pubkey.clear();
    }
    std::vector<uint8_t> ser;
    serializeTransaction(tmp, ser);
    unsigned char hash[32];
    shawncoin_sha256d(ser.data(), ser.size(), hash);

    // For each input, find corresponding UTXO and sign using the derived private key
    for (size_t i = 0; i < tx.inputs.size(); ++i) {
        const auto& in = tx.inputs[i];
        OutPoint op{ in.prev_tx_hash, in.output_index };
        auto opt = utxo->get(op);
        if (!opt) return false;
        const Script& script = opt->script_pubkey;
        // Expect P2PKH script format
        if (!(script.size() == 25 && script[0] == 0x76 && script[1] == 0xa9 && script[2] == 0x14 && script[23] == 0x88 && script[24] == 0xac))
            return false;
        std::vector<uint8_t> hash160(20);
        std::copy(script.begin() + 3, script.begin() + 23, hash160.begin());

        // Find address string for this hash160 among generated addresses
        std::string addrFound;
        size_t maxIndex = keys_.size();
        for (size_t j = 0; j < maxIndex; ++j) {
            std::string a = hd_.deriveAddress((uint32_t)j);
            std::vector<uint8_t> h = addressToPubKeyHash(a);
            if (h.size() == 20 && memcmp(h.data(), hash160.data(), 20) == 0) {
                addrFound = a;
                break;
            }
        }
        if (addrFound.empty()) return false;

        // Derive private key for this address
        std::vector<uint8_t> priv = findPrivateKeyForAddress(hd_, keys_.size(), addrFound);
        if (priv.size() != 32) return false;
        shawncoin_keypair_t* kp = shawncoin_keypair_from_priv(priv.data());
        if (!kp) return false;
        unsigned char sig[128]; size_t siglen = sizeof(sig);
        if (shawncoin_sign(kp, hash, sig, &siglen) != 1) {
            shawncoin_keypair_destroy(kp);
            return false;
        }
        unsigned char pub[33];
        if (shawncoin_pubkey_get(kp, pub) != 1) {
            shawncoin_keypair_destroy(kp);
            return false;
        }
        shawncoin_keypair_destroy(kp);

        tx.inputs[i].signature.assign(sig, sig + siglen);
        tx.inputs[i].pubkey.assign(pub, pub + 33);
    }
    return true;
}

size_t Wallet::getAddressCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return keys_.size();
}

std::string Wallet::deriveAddressAt(uint32_t index) const {
    // hd_ is not thread-safe for concurrent mutable ops, but deriveAddress is const-like here
    return hd_.deriveAddress(index);
}

bool Wallet::hasHash160(const std::vector<uint8_t>& h) const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& kp : keys_) {
        if (kp.second.size() == h.size() && memcmp(kp.second.data(), h.data(), h.size()) == 0)
            return true;
    }
    // fall back to deriving addresses up to keys_.size()
    size_t maxIndex = keys_.size();
    for (size_t i = 0; i < maxIndex; ++i) {
        std::string a = hd_.deriveAddress((uint32_t)i);
        std::vector<uint8_t> hh = addressToPubKeyHash(a);
        if (hh.size() == h.size() && memcmp(hh.data(), h.data(), h.size()) == 0) return true;
    }
    return false;
}

uint64_t Wallet::getBalance(const UTXOSet* utxo) const {
    if (!utxo) return 0;
    uint64_t sum = 0;
    auto snap = utxo->snapshot();
    for (const auto& it : snap) {
        const UTXOSet::Entry& e = it.second;
        const Script& s = e.script_pubkey;
        // Recognize simple P2PKH: OP_DUP OP_HASH160 PUSH20 <20> OP_EQUALVERIFY OP_CHECKSIG
        if (s.size() == 25 && s[0] == 0x76 && s[1] == 0xa9 && s[2] == 0x14 && s[23] == 0x88 && s[24] == 0xac) {
            std::vector<uint8_t> hash160(20);
            std::copy(s.begin() + 3, s.begin() + 23, hash160.begin());
            // check if this hash160 matches any of our addresses
            for (const auto& kp : keys_) {
                const std::vector<uint8_t>& our = kp.second;
                if (!our.empty() && our.size() == 20 && memcmp(our.data(), hash160.data(), 20) == 0) {
                    sum += e.amount;
                    break;
                }
            }
        }
    }
    return sum;
}

void Wallet::backup(const std::string& path) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ofstream f(path);
    if (f) f << mnemonic_ << "\n";
}

static bool encryptWithPass(const std::string& pass, const std::string& plaintext, std::string& out_salt_hex, std::string& out_iv_hex, std::string& out_ct_hex) {
    unsigned char salt[16];
    if (RAND_bytes(salt, sizeof(salt)) != 1) return false;
    std::vector<uint8_t> keyiv(48);
    if (PKCS5_PBKDF2_HMAC(pass.c_str(), (int)pass.size(), salt, sizeof(salt), 10000, EVP_sha256(), (int)keyiv.size(), keyiv.data()) != 1)
        return false;
    unsigned char key[32], iv[16];
    memcpy(key, keyiv.data(), 32);
    memcpy(iv, keyiv.data() + 32, 16);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    int outlen = 0; std::vector<unsigned char> outbuf(plaintext.size() + 32);
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) { EVP_CIPHER_CTX_free(ctx); return false; }
    if (EVP_EncryptUpdate(ctx, outbuf.data(), &outlen, (const unsigned char*)plaintext.data(), (int)plaintext.size()) != 1) { EVP_CIPHER_CTX_free(ctx); return false; }
    int total = outlen;
    if (EVP_EncryptFinal_ex(ctx, outbuf.data() + outlen, &outlen) != 1) { EVP_CIPHER_CTX_free(ctx); return false; }
    total += outlen;
    EVP_CIPHER_CTX_free(ctx);

    out_salt_hex = shawncoin::hexEncode(salt, sizeof(salt));
    out_iv_hex = shawncoin::hexEncode(iv, sizeof(iv));
    out_ct_hex = shawncoin::hexEncode(outbuf.data(), total);
    // wipe key material
    OPENSSL_cleanse(key, sizeof(key));
    OPENSSL_cleanse(iv, sizeof(iv));
    OPENSSL_cleanse(keyiv.data(), keyiv.size());
    return true;
}

static bool decryptWithPass(const std::string& pass, const std::string& salt_hex, const std::string& iv_hex, const std::string& ct_hex, std::string& out_plain) {
    std::vector<uint8_t> salt = shawncoin::hexDecode(salt_hex);
    std::vector<uint8_t> iv = shawncoin::hexDecode(iv_hex);
    std::vector<uint8_t> ct = shawncoin::hexDecode(ct_hex);
    if (salt.size() != 16 || iv.size() != 16 || ct.empty()) return false;
    std::vector<uint8_t> keyiv(48);
    if (PKCS5_PBKDF2_HMAC(pass.c_str(), (int)pass.size(), salt.data(), (int)salt.size(), 10000, EVP_sha256(), (int)keyiv.size(), keyiv.data()) != 1)
        return false;
    unsigned char key[32]; memcpy(key, keyiv.data(), 32);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    std::vector<unsigned char> outbuf(ct.size() + 32);
    int outlen = 0;
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv.data()) != 1) { EVP_CIPHER_CTX_free(ctx); return false; }
    if (EVP_DecryptUpdate(ctx, outbuf.data(), &outlen, ct.data(), (int)ct.size()) != 1) { EVP_CIPHER_CTX_free(ctx); return false; }
    int total = outlen;
    if (EVP_DecryptFinal_ex(ctx, outbuf.data() + outlen, &outlen) != 1) { EVP_CIPHER_CTX_free(ctx); return false; }
    total += outlen;
    EVP_CIPHER_CTX_free(ctx);

    out_plain.assign((char*)outbuf.data(), total);
    OPENSSL_cleanse(key, sizeof(key));
    OPENSSL_cleanse(keyiv.data(), keyiv.size());
    return true;
}

bool Wallet::saveToFile(const std::string& path, const std::string& passphrase) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string salt, iv, ct;
    if (!encryptWithPass(passphrase, mnemonic_, salt, iv, ct)) return false;
    std::ofstream f(path, std::ios::trunc);
    if (!f) return false;
    f << "salt:" << salt << "\n";
    f << "iv:" << iv << "\n";
    f << "ct:" << ct << "\n";
    return true;
}

bool Wallet::loadFromFile(const std::string& path, const std::string& passphrase) {
    std::ifstream f(path);
    if (!f) return false;
    std::string line; std::string salt, iv, ct;
    while (std::getline(f, line)) {
        if (line.rfind("salt:", 0) == 0) salt = line.substr(5);
        else if (line.rfind("iv:", 0) == 0) iv = line.substr(3);
        else if (line.rfind("ct:", 0) == 0) ct = line.substr(3);
    }
    if (salt.empty() || iv.empty() || ct.empty()) return false;
    std::string plain;
    if (!decryptWithPass(passphrase, salt, iv, ct, plain)) return false;
    return restore(plain, "");
}

} // namespace shawncoin
