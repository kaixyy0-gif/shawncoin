#include "crypto/hash.h"
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <string.h>

void shawncoin_sha256(const unsigned char *data, size_t len, unsigned char *out) {
    unsigned char tmp[SHA256_DIGEST_LENGTH];
    SHA256(data, len, tmp);
    memcpy(out, tmp, SHA256_DIGEST_LENGTH);
}

void shawncoin_sha256d(const unsigned char *data, size_t len, unsigned char *out) {
    unsigned char tmp[SHA256_DIGEST_LENGTH];
    SHA256(data, len, tmp);
    SHA256(tmp, SHA256_DIGEST_LENGTH, out);
}

void shawncoin_ripemd160(const unsigned char *data, size_t len, unsigned char *out) {
    unsigned char tmp[RIPEMD160_DIGEST_LENGTH];
    unsigned int out_len = 0;
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx && EVP_DigestInit_ex(ctx, EVP_ripemd160(), NULL)) {
        EVP_DigestUpdate(ctx, data, len);
        EVP_DigestFinal_ex(ctx, tmp, &out_len);
    }
    if (ctx) EVP_MD_CTX_free(ctx);
    memcpy(out, tmp, RIPEMD160_DIGEST_LENGTH);
}

void shawncoin_hash160(const unsigned char *data, size_t len, unsigned char *out) {
    unsigned char sha[SHA256_DIGEST_LENGTH];
    SHA256(data, len, sha);
    shawncoin_ripemd160(sha, SHA256_DIGEST_LENGTH, out);
}
