#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <termios.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

namespace fs = std::filesystem;

void printBanner()
{
    std::cout << "\033[1;36m";
    std::cout << "============================\n";
    std::cout << "     FILE SPLITTER CLI      \n";
    std::cout << "============================\n";
    std::cout << "\033[0m";
}

std::string getHiddenInput(const std::string &prompt)
{
    std::cout << prompt;
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::string input;
    std::getline(std::cin, input);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << "\n";
    return input;
}

bool deriveKeyAndIV(const std::string &password, unsigned char *key, unsigned char *iv)
{
    const unsigned char salt[] = "mysalt";
    return PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), salt, sizeof(salt), 10000, EVP_sha256(), 32, key) &&
           PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), salt, sizeof(salt), 10000, EVP_sha256(), 16, iv);
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
    std::vector<unsigned char> encrypted(data.size() + 16);
    int outLen1 = 0, outLen2 = 0;
    EVP_EncryptUpdate(ctx, encrypted.data(), &outLen1, (unsigned char *)data.data(), data.size());
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
    std::vector<char> decrypted(data.size());
    int outLen1 = 0, outLen2 = 0;
    EVP_DecryptUpdate(ctx, (unsigned char *)decrypted.data(), &outLen1, data.data(), data.size());
    EVP_DecryptFinal_ex(ctx, (unsigned char *)decrypted.data() + outLen1, &outLen2);
    decrypted.resize(outLen1 + outLen2);
    EVP_CIPHER_CTX_free(ctx);
    return decrypted;
}

void splitFile(const std::string &filePath, size_t chunkSize, const std::string &password)
{
    std::ifstream inputFile(filePath, std::ios::binary);
    if (!inputFile)
    {
        std::cerr << "\033[1;31mError: Unable to open file!\033[0m\n";
        return;
    }

    fs::path inputPath(filePath);
    std::string outputDir = inputPath.stem().string() + "_chunks";
    fs::create_directory(outputDir);

    std::vector<char> buffer(chunkSize);
    size_t partNumber = 0;
    while (inputFile)
    {
        inputFile.read(buffer.data(), chunkSize);
        std::streamsize bytesRead = inputFile.gcount();
        if (bytesRead <= 0)
            break;

        auto encryptedData = encryptChunk({buffer.begin(), buffer.begin() + bytesRead}, password);
        std::ofstream outputFile(outputDir + "/chunk_" + std::to_string(partNumber), std::ios::binary);
        if (!outputFile)
        {
            std::cerr << "\033[1;31mError: Unable to create chunk file!\033[0m\n";
            return;
        }
        outputFile.write((char *)encryptedData.data(), encryptedData.size());
        partNumber++;
    }
    std::cout << "\033[1;32mFile split and encrypted into " << partNumber << " chunks in '" << outputDir << "'.\033[0m\n";
}

void joinFiles(const std::string &chunkDir, const std::string &outputFilePath, const std::string &password)
{
    std::vector<std::string> chunkFiles;
    for (const auto &entry : fs::directory_iterator(chunkDir))
    {
        chunkFiles.push_back(entry.path().string());
    }

    std::sort(chunkFiles.begin(), chunkFiles.end());

    std::ofstream outputFile(outputFilePath, std::ios::binary);
    if (!outputFile)
    {
        std::cerr << "\033[1;31mError: Unable to create output file!\033[0m\n";
        return;
    }

    for (const auto &chunk : chunkFiles)
    {
        std::ifstream inputFile(chunk, std::ios::binary);
        if (!inputFile)
        {
            std::cerr << "\033[1;31mError: Unable to open chunk file " << chunk << "\033[0m\n";
            return;
        }
        std::vector<unsigned char> encryptedData((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
        auto decryptedData = decryptChunk(encryptedData, password);
        outputFile.write(decryptedData.data(), decryptedData.size());
    }

    std::cout << "\033[1;32mFile successfully decrypted and reconstructed as '" << outputFilePath << "'.\033[0m\n";
}

int main()
{
    printBanner();
    int choice;
    std::cout << "\033[1;33m1. Split & Encrypt File\n2. Decrypt & Join Files\nEnter choice: \033[0m";
    std::cin >> choice;
    std::cin.ignore();
    std::string password = getHiddenInput("Enter password: ");

    if (choice == 1)
    {
        std::string filePath;
        size_t chunkSize;
        std::cout << "\033[1;34mEnter file path: \033[0m";
        std::getline(std::cin, filePath);
        std::cout << "\033[1;34mEnter chunk size (bytes): \033[0m";
        std::cin >> chunkSize;
        splitFile(filePath, chunkSize, password);
    }
    else if (choice == 2)
    {
        std::string chunkDir, outputFilePath;
        std::cout << "\033[1;34mEnter chunk directory: \033[0m";
        std::getline(std::cin, chunkDir);
        std::cout << "\033[1;34mEnter output file path: \033[0m";
        std::getline(std::cin, outputFilePath);
        joinFiles(chunkDir, outputFilePath, password);
    }
    return 0;
}
