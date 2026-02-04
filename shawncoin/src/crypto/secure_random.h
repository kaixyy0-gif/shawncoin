#ifndef SHAWNCOIN_CRYPTO_SECURE_RANDOM_H
#define SHAWNCOIN_CRYPTO_SECURE_RANDOM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Fill buffer with cryptographically secure random bytes. Returns 1 on success, 0 on failure. */
int shawncoin_secure_rand(unsigned char *buf, size_t len);

/* Constant-time comparison to prevent timing attacks. Returns 1 if equal, 0 otherwise. */
int shawncoin_memcmp_const(const void *a, const void *b, size_t len);

/* Securely wipe memory (e.g. before free) */
void shawncoin_memwipe(void *ptr, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SHAWNCOIN_CRYPTO_SECURE_RANDOM_H */
