#ifndef __CRYPTO__
#define __CRYPTO__

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <Type.h>

namespace Crypto {
    int SHA1Encrypt(uvector&, uvector&, uvector&);
};

#endif