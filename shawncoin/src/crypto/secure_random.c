#include "crypto/secure_random.h"
#include <openssl/rand.h>
#include <string.h>

int shawncoin_secure_rand(unsigned char *buf, size_t len) {
    if (!buf || len == 0) return 0;
    return RAND_bytes(buf, (int)len) == 1 ? 1 : 0;
}

int shawncoin_memcmp_const(const void *a, const void *b, size_t len) {
    const unsigned char *pa = (const unsigned char *)a;
    const unsigned char *pb = (const unsigned char *)b;
    volatile unsigned char diff = 0;
    size_t i;
    for (i = 0; i < len; i++)
        diff |= pa[i] ^ pb[i];
    return diff == 0 ? 1 : 0;
}

void shawncoin_memwipe(void *ptr, size_t len) {
    if (ptr && len > 0)
        memset(ptr, 0, len);
}
