#include "storage/database.hpp"
#include <map>
#include <mutex>
#include <memory>

namespace shawncoin {

#if defined(HAVE_ROCKSDB)
#include <rocksdb/db.h>
#include <rocksdb/options.h>

class RocksDBDatabase : public Database {
public:
    bool open(const std::string& path) override {
        rocksdb::Options opts;
        opts.create_if_missing = true;
        rocksdb::Status s = rocksdb::DB::Open(opts, path, &db_);
        return s.ok();
    }
    void close() override { delete db_; db_ = nullptr; }
    bool get(const std::string& key, std::string& value) const override {
        rocksdb::Status s = db_->Get(rocksdb::ReadOptions(), key, &value);
        return s.ok();
    }
    bool put(const std::string& key, const std::string& value) override {
        return db_->Put(rocksdb::WriteOptions(), key, value).ok();
    }
    bool del(const std::string& key) override {
        return db_->Delete(rocksdb::WriteOptions(), key).ok();
    }
private:
    rocksdb::DB* db_ = nullptr;
};

std::unique_ptr<Database> createDatabase() {
    return std::make_unique<RocksDBDatabase>();
}

#else

class StubDatabase : public Database {
public:
    bool open(const std::string& path) override { (void)path; return true; }
    void close() override {}
    bool get(const std::string& key, std::string& value) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = map_.find(key);
        if (it == map_.end()) return false;
        value = it->second;
        return true;
    }
    bool put(const std::string& key, const std::string& value) override {
        std::lock_guard<std::mutex> lock(mutex_);
        map_[key] = value;
        return true;
    }
    bool del(const std::string& key) override {
        std::lock_guard<std::mutex> lock(mutex_);
        return map_.erase(key) > 0;
    }
private:
    mutable std::mutex mutex_;
    std::map<std::string, std::string> map_;
};

std::unique_ptr<Database> createDatabase() {
    return std::make_unique<StubDatabase>();
}

#endif

} // namespace shawncoin
