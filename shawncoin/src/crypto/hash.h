#ifndef SHAWNCOIN_CRYPTO_HASH_H
#define SHAWNCOIN_CRYPTO_HASH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHA256_DIGEST_LENGTH 32
#define RIPEMD160_DIGEST_LENGTH 20

/* SHA-256: output 32 bytes */
void shawncoin_sha256(const unsigned char *data, size_t len, unsigned char *out);

/* Double SHA-256 (SHA256d) as used for block/tx hashes */
void shawncoin_sha256d(const unsigned char *data, size_t len, unsigned char *out);

/* RIPEMD-160: output 20 bytes */
void shawncoin_ripemd160(const unsigned char *data, size_t len, unsigned char *out);

/* Hash160 = RIPEMD160(SHA256(data)) - used for address from pubkey */
void shawncoin_hash160(const unsigned char *data, size_t len, unsigned char *out);

#ifdef __cplusplus
}
#endif

#endif /* SHAWNCOIN_CRYPTO_HASH_H */
