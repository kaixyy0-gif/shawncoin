#ifndef SHAWNCOIN_CRYPTO_SIGNATURES_H
#define SHAWNCOIN_CRYPTO_SIGNATURES_H

#include "crypto/keys.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ECDSA sign hash (32 bytes) with key. Output: *sig_len bytes (max 72). Returns 1 on success. */
int shawncoin_sign(const shawncoin_keypair_t *key, const unsigned char *hash32, unsigned char *sig, size_t *sig_len);

/* ECDSA verify: pubkey 33 bytes, hash 32 bytes, sig of sig_len. Returns 1 if valid. */
int shawncoin_verify(const unsigned char *pubkey33, const unsigned char *hash32, const unsigned char *sig, size_t sig_len);

#ifdef __cplusplus
}
#endif

#endif /* SHAWNCOIN_CRYPTO_SIGNATURES_H */
