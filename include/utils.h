#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <termios.h>
#include <string>
#include <openssl/evp.h>
#include <openssl/sha.h>

std::string getHiddenInput(const std::string &prompt);

bool deriveKeyAndIV(const std::string &password, unsigned char *key, unsigned char *iv);

std::vector<unsigned char> encryptChunk(const std::vector<char> &data, const std::string &password);

std::vector<char> decryptChunk(const std::vector<unsigned char> &data, const std::string &password);

#endif
