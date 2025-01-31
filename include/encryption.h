#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <string>
#include <vector>
#include <openssl/evp.h>

bool deriveKeyAndIV(const std::string &password, unsigned char *key, unsigned char *iv);
std::vector<unsigned char> encryptChunk(const std::vector<char> &data, const std::string &password);
std::vector<char> decryptChunk(const std::vector<unsigned char> &data, const std::string &password);

#endif // ENCRYPTION_H