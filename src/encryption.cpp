#include "encryption.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <iostream>

bool deriveKeyAndIV(const std::string &password, unsigned char *key, unsigned char *iv)
{
    const unsigned char salt[] = "mysalt";
    return PKCS5_PBKDF2_HMAC(password.c_str(), password.size(),
                             salt, sizeof(salt),
                             10000, EVP_sha256(),
                             32, key) &&
           PKCS5_PBKDF2_HMAC(password.c_str(), password.size(),
                             salt, sizeof(salt),
                             10000, EVP_sha256(),
                             16, iv);
}

std::vector<unsigned char> encryptChunk(const std::vector<char> &data, const std::string &password)
{
    unsigned char key[32], iv[16];
    if (!deriveKeyAndIV(password, key, iv))
    {
        std::cerr << "\033[1;31mError: Failed to derive encryption key!\033[0m\n";
        return {};
    }
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
    std::vector<unsigned char> encrypted(data.size() + EVP_MAX_BLOCK_LENGTH);
    int outLen1 = 0, outLen2 = 0;
    EVP_EncryptUpdate(ctx, encrypted.data(), &outLen1,
                      (unsigned char *)data.data(), data.size());
    EVP_EncryptFinal_ex(ctx, encrypted.data() + outLen1, &outLen2);
    encrypted.resize(outLen1 + outLen2);
    EVP_CIPHER_CTX_free(ctx);
    return encrypted;
}

std::vector<char> decryptChunk(const std::vector<unsigned char> &data, const std::string &password)
{
    unsigned char key[32], iv[16];
    if (!deriveKeyAndIV(password, key, iv))
    {
        std::cerr << "\033[1;31mError: Failed to derive decryption key!\033[0m\n";
        return {};
    }
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
    std::vector<char> decrypted(data.size() + EVP_MAX_BLOCK_LENGTH);
    int outLen1 = 0, outLen2 = 0;
    EVP_DecryptUpdate(ctx, (unsigned char *)decrypted.data(), &outLen1,
                      data.data(), data.size());
    EVP_DecryptFinal_ex(ctx, (unsigned char *)decrypted.data() + outLen1, &outLen2);
    decrypted.resize(outLen1 + outLen2);
    EVP_CIPHER_CTX_free(ctx);
    return decrypted;
}