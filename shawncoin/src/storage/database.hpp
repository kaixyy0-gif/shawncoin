#ifndef SHAWNCOIN_STORAGE_DATABASE_HPP
#define SHAWNCOIN_STORAGE_DATABASE_HPP

#include "../core/types.hpp"
#include "../core/block.hpp"
#include <string>
#include <memory>
#include <optional>

namespace shawncoin {

/** Database interface (RocksDB implementation when HAVE_ROCKSDB). */
class Database {
public:
    virtual ~Database() = default;
    virtual bool open(const std::string& path) = 0;
    virtual void close() = 0;
    virtual bool get(const std::string& key, std::string& value) const = 0;
    virtual bool put(const std::string& key, const std::string& value) = 0;
    virtual bool del(const std::string& key) = 0;
};

/** Create database (RocksDB if available, else null). */
std::unique_ptr<Database> createDatabase();

} // namespace shawncoin

#endif // SHAWNCOIN_STORAGE_DATABASE_HPP
