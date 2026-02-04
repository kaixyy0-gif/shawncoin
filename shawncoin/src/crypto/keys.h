#ifndef SHAWNCOIN_CRYPTO_KEYS_H
#define SHAWNCOIN_CRYPTO_KEYS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHAWNCOIN_PUBKEY_LEN 33
#define SHAWNCOIN_PRIVKEY_LEN 32

/* Opaque key context - use void* in C, implementation uses EC_KEY or secp256k1 */
typedef struct shawncoin_keypair shawncoin_keypair_t;

/* Generate new key pair (secp256k1). Returns NULL on failure. */
shawncoin_keypair_t *shawncoin_keypair_create(void);

/* Load private key from 32 bytes. Returns NULL on failure. */
shawncoin_keypair_t *shawncoin_keypair_from_priv(const unsigned char *priv32);

/* Get compressed public key (33 bytes). Returns 1 on success. */
int shawncoin_pubkey_get(const shawncoin_keypair_t *key, unsigned char *out33);

/* Get private key (32 bytes). Returns 1 on success. */
int shawncoin_privkey_get(const shawncoin_keypair_t *key, unsigned char *out32);

/* Internal: get EC_KEY* for OpenSSL path (C only). Returns NULL if using secp256k1. */
void *shawncoin_keypair_get_ec_key(const shawncoin_keypair_t *key);

void shawncoin_keypair_destroy(shawncoin_keypair_t *key);

#ifdef __cplusplus
}
#endif

#endif /* SHAWNCOIN_CRYPTO_KEYS_H */
