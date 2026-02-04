#include "crypto/keys.h"
#include "crypto/secure_random.h"
#include <stdlib.h>
#include <string.h>

#if defined(HAVE_SECP256K1)
#include <secp256k1.h>
#else
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#endif

struct shawncoin_keypair {
#if defined(HAVE_SECP256K1)
    secp256k1_context *ctx;
    unsigned char seckey[32];
#else
    void *ec_key; /* EC_KEY* */
#endif
};

#if defined(HAVE_SECP256K1)
shawncoin_keypair_t *shawncoin_keypair_create(void) {
    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    if (!ctx) return NULL;
    shawncoin_keypair_t *kp = (shawncoin_keypair_t *)malloc(sizeof(shawncoin_keypair_t));
    if (!kp) { secp256k1_context_destroy(ctx); return NULL; }
    kp->ctx = ctx;
    for (;;) {
        if (shawncoin_secure_rand(kp->seckey, 32) != 1) { shawncoin_keypair_destroy(kp); return NULL; }
        if (secp256k1_ec_seckey_verify(ctx, kp->seckey) == 1)
            break;
    }
    return kp;
}
#else
static EC_KEY *ec_key_new(void) {
    EC_KEY *key = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!key) return NULL;
    if (EC_KEY_generate_key(key) != 1) { EC_KEY_free(key); return NULL; }
    return key;
}

shawncoin_keypair_t *shawncoin_keypair_create(void) {
    EC_KEY *ec = ec_key_new();
    if (!ec) return NULL;
    shawncoin_keypair_t *kp = (shawncoin_keypair_t *)malloc(sizeof(shawncoin_keypair_t));
    if (!kp) { EC_KEY_free(ec); return NULL; }
    kp->ec_key = ec;
    return kp;
}
#endif

#if defined(HAVE_SECP256K1)
shawncoin_keypair_t *shawncoin_keypair_from_priv(const unsigned char *priv32) {
    if (!priv32) return NULL;
    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    if (!ctx) return NULL;
    if (secp256k1_ec_seckey_verify(ctx, priv32) != 1) { secp256k1_context_destroy(ctx); return NULL; }
    shawncoin_keypair_t *kp = (shawncoin_keypair_t *)malloc(sizeof(shawncoin_keypair_t));
    if (!kp) { secp256k1_context_destroy(ctx); return NULL; }
    kp->ctx = ctx;
    memcpy(kp->seckey, priv32, 32);
    return kp;
}
#else
shawncoin_keypair_t *shawncoin_keypair_from_priv(const unsigned char *priv32) {
    if (!priv32) return NULL;
    EC_KEY *ec = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!ec) return NULL;
    BIGNUM *bn = BN_bin2bn(priv32, 32, NULL);
    if (!bn) { EC_KEY_free(ec); return NULL; }
    if (EC_KEY_set_private_key(ec, bn) != 1) { BN_free(bn); EC_KEY_free(ec); return NULL; }
    const EC_GROUP *grp = EC_KEY_get0_group(ec);
    EC_POINT *pub = EC_POINT_new(grp);
    if (!pub || EC_POINT_mul(grp, pub, bn, NULL, NULL, NULL) != 1) {
        if (pub) EC_POINT_free(pub);
        BN_free(bn);
        EC_KEY_free(ec);
        return NULL;
    }
    EC_KEY_set_public_key(ec, pub);
    EC_POINT_free(pub);
    BN_free(bn);
    shawncoin_keypair_t *kp = (shawncoin_keypair_t *)malloc(sizeof(shawncoin_keypair_t));
    if (!kp) { EC_KEY_free(ec); return NULL; }
    kp->ec_key = ec;
    return kp;
}
#endif

#if defined(HAVE_SECP256K1)
int shawncoin_pubkey_get(const shawncoin_keypair_t *key, unsigned char *out33) {
    if (!key || !out33) return 0;
    secp256k1_pubkey pubkey;
    if (secp256k1_ec_pubkey_create(key->ctx, &pubkey, key->seckey) != 1) return 0;
    size_t len = 33;
    return secp256k1_ec_pubkey_serialize(key->ctx, out33, &len, &pubkey, SECP256K1_EC_COMPRESSED) == 1 ? 1 : 0;
}
#else
int shawncoin_pubkey_get(const shawncoin_keypair_t *key, unsigned char *out33) {
    if (!key || !out33) return 0;
    EC_KEY *ec = (EC_KEY *)key->ec_key;
    size_t len = 33;
    return EC_POINT_point2oct(EC_KEY_get0_group(ec), EC_KEY_get0_public_key(ec),
                              POINT_CONVERSION_COMPRESSED, out33, len, NULL) == (int)len ? 1 : 0;
}
#endif

#if defined(HAVE_SECP256K1)
int shawncoin_privkey_get(const shawncoin_keypair_t *key, unsigned char *out32) {
    if (!key || !out32) return 0;
    memcpy(out32, key->seckey, 32);
    return 1;
}
#else
int shawncoin_privkey_get(const shawncoin_keypair_t *key, unsigned char *out32) {
    if (!key || !out32) return 0;
    const BIGNUM *bn = EC_KEY_get0_private_key((EC_KEY *)key->ec_key);
    if (!bn) return 0;
    int n = BN_num_bytes(bn);
    if (n > 32) return 0;
    memset(out32, 0, 32 - n);
    BN_bn2bin(bn, out32 + (32 - n));
    return 1;
}
#endif

void *shawncoin_keypair_get_ec_key(const shawncoin_keypair_t *key) {
#if defined(HAVE_SECP256K1)
    (void)key;
    return NULL;
#else
    return key ? key->ec_key : NULL;
#endif
}

#if defined(HAVE_SECP256K1)
void shawncoin_keypair_destroy(shawncoin_keypair_t *key) {
    if (key) {
        if (key->ctx) secp256k1_context_destroy(key->ctx);
        shawncoin_memwipe(key->seckey, 32);
        memset(key, 0, sizeof(*key));
        free(key);
    }
}
#else
void shawncoin_keypair_destroy(shawncoin_keypair_t *key) {
    if (key) {
        if (key->ec_key) EC_KEY_free((EC_KEY *)key->ec_key);
        memset(key, 0, sizeof(*key));
        free(key);
    }
}
#endif
