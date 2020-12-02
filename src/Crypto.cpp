#include <Crypto.h>

int Crypto::SHA1Encrypt(uvector &key, uvector &input, uvector &output) {
    unsigned char output_buf[EVP_MAX_MD_SIZE];
    unsigned int output_len;
    HMAC_CTX *ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, (unsigned char *)&key[0], key.size(), EVP_sha1(), NULL);
    HMAC_Update(ctx, (unsigned char *)&input[0], input.size());
    HMAC_Final(ctx, output_buf, &output_len);
    HMAC_CTX_free(ctx);

    output = uvector();
    for(int i = 0; i < output_len; i++)
        output.push_back(output_buf[i]);
    return 0;
}