#include "crypto/signatures.h"
#include <stdlib.h>
#include <string.h>

#if defined(HAVE_SECP256K1)
#include <secp256k1.h>
#include <secp256k1_recovery.h>
#else
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/sha.h>
#include <openssl/obj_mac.h>
#include <openssl/evp.h>
#endif

int shawncoin_sign(const shawncoin_keypair_t *key, const unsigned char *hash32, unsigned char *sig, size_t *sig_len) {
    if (!key || !hash32 || !sig || !sig_len || *sig_len < 64) return 0;
#if defined(HAVE_SECP256K1)
    secp256k1_ecdsa_signature secp_sig;
    if (secp256k1_ecdsa_sign(key->ctx, &secp_sig, hash32, key->seckey, NULL, NULL) != 1)
        return 0;
    if (secp256k1_ecdsa_signature_serialize_compact(key->ctx, sig, &secp_sig) != 1)
        return 0;
    *sig_len = 64;
    return 1;
#else
    EC_KEY *ec = (EC_KEY *)shawncoin_keypair_get_ec_key(key);
    if (!ec) return 0;
    ECDSA_SIG *s = ECDSA_do_sign(hash32, 32, ec);
    if (!s) return 0;
    const BIGNUM *r, *rs;
    ECDSA_SIG_get0(s, &r, &rs);
    int r_len = BN_num_bytes(r), s_len = BN_num_bytes(rs);
    if (r_len > 32 || s_len > 32) { ECDSA_SIG_free(s); return 0; }
    memset(sig, 0, 64);
    BN_bn2bin(r, sig + (32 - r_len));
    BN_bn2bin(rs, sig + 32 + (32 - s_len));
    *sig_len = 64;
    ECDSA_SIG_free(s);
    return 1;
#endif
}

int shawncoin_verify(const unsigned char *pubkey33, const unsigned char *hash32, const unsigned char *sig, size_t sig_len) {
    if (!pubkey33 || !hash32 || !sig) return 0;
#if defined(HAVE_SECP256K1)
    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY);
    if (!ctx) return 0;
    secp256k1_pubkey pub;
    if (secp256k1_ec_pubkey_parse(ctx, &pub, pubkey33, 33) != 1) {
        secp256k1_context_destroy(ctx);
        return 0;
    }
    secp256k1_ecdsa_signature secp_sig;
    if (sig_len != 64 || secp256k1_ecdsa_signature_parse_compact(ctx, &secp_sig, sig) != 1) {
        secp256k1_context_destroy(ctx);
        return 0;
    }
    int ret = secp256k1_ecdsa_verify(ctx, &secp_sig, hash32, &pub);
    secp256k1_context_destroy(ctx);
    return ret;
#else
    EC_KEY *ec = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!ec) return 0;
    const EC_GROUP *grp = EC_KEY_get0_group(ec);
    EC_POINT *pub = EC_POINT_new(grp);
    if (!pub || EC_POINT_oct2point(grp, pub, pubkey33, 33, NULL) != 1) {
        if (pub) EC_POINT_free(pub);
        EC_KEY_free(ec);
        return 0;
    }
    EC_KEY_set_public_key(ec, pub);
    EC_POINT_free(pub);
    if (sig_len != 64) { EC_KEY_free(ec); return 0; }
    BIGNUM *r = BN_bin2bn(sig, 32, NULL), *s = BN_bin2bn(sig + 32, 32, NULL);
    if (!r || !s) { BN_free(r); BN_free(s); EC_KEY_free(ec); return 0; }
    ECDSA_SIG *ec_sig = ECDSA_SIG_new();
    if (!ec_sig) { BN_free(r); BN_free(s); EC_KEY_free(ec); return 0; }
    ECDSA_SIG_set0(ec_sig, r, s);
    int ok = ECDSA_do_verify(hash32, 32, ec_sig, ec) == 1;
    ECDSA_SIG_free(ec_sig);
    EC_KEY_free(ec);
    return ok;
#endif
}
