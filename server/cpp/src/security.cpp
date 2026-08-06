#include "security.hpp"

#include <openssl/evp.h>

#include <cstdio>

namespace http {

std::string sha256_hex(const std::string& data) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return "";

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(ctx, data.data(), data.size()) != 1 ||
        EVP_DigestFinal_ex(ctx, digest, &len) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }
    EVP_MD_CTX_free(ctx);

    std::string hex;
    hex.reserve(len * 2);
    char buf[3];
    for (unsigned int i = 0; i < len; ++i) {
        std::snprintf(buf, sizeof(buf), "%02x", digest[i]);
        hex += buf;
    }
    return hex;
}

}  // namespace http
