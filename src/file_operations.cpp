#include "file_operations.h"
#include "encryption.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>

void splitFile(const std::string &filePath, size_t chunkSize, const std::string &password)
{
    std::ifstream inputFile(filePath, std::ios::binary);
    if (!inputFile)
    {
        std::cerr << "\033[1;31mError: Unable to open file!\033[0m\n";
        return;
    }

    inputFile.seekg(0, std::ios::end);
    size_t totalFileSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    fs::path inputPath(filePath);
    std::string outputDir = inputPath.stem().string() + "_chunks";
    fs::create_directory(outputDir);

    std::vector<char> buffer(chunkSize);
    size_t partNumber = 0;
    size_t totalBytesProcessed = 0;
    auto startTime = std::chrono::steady_clock::now();

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    unsigned char key[32], iv[16];
    if (!deriveKeyAndIV(password, key, iv))
    {
        std::cerr << "\033[1;31mError: Failed to derive encryption key!\033[0m\n";
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    while (inputFile)
    {
        inputFile.read(buffer.data(), chunkSize);
        std::streamsize bytesRead = inputFile.gcount();
        if (bytesRead <= 0)
            break;

        std::vector<unsigned char> encryptedData(bytesRead + EVP_MAX_BLOCK_LENGTH);
        int outLen1 = 0, outLen2 = 0;
        EVP_EncryptUpdate(ctx, encryptedData.data(), &outLen1,
                          (unsigned char *)buffer.data(), bytesRead);
        EVP_EncryptFinal_ex(ctx, encryptedData.data() + outLen1, &outLen2);
        encryptedData.resize(outLen1 + outLen2);

        // Write the encrypted chunk to disk
        std::string chunkPath = outputDir + "/chunk_" + std::string(5 - std::to_string(partNumber).length(), '0') + std::to_string(partNumber);
        std::ofstream outputFile(chunkPath, std::ios::binary);
        if (!outputFile)
        {
            std::cerr << "\033[1;31mError: Unable to create chunk file!\033[0m\n";
            EVP_CIPHER_CTX_free(ctx);
            return;
        }
        outputFile.write((char *)encryptedData.data(), encryptedData.size());

        totalBytesProcessed += bytesRead;
        partNumber++;
        double progress = (static_cast<double>(totalBytesProcessed) / totalFileSize) * 100.0;
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsedSeconds = currentTime - startTime;
        double projectedTimeRemaining = (elapsedSeconds.count() / progress) * (100.0 - progress);

        std::cout << "\r\033[1;32mProgress: " << std::fixed << std::setprecision(2) << progress << "% ("
              << totalBytesProcessed << " / " << totalFileSize << " bytes), "
              << "Elapsed: " << std::fixed << std::setprecision(2) << elapsedSeconds.count() << "s, "
              << "Remaining: " << std::fixed << std::setprecision(2) << projectedTimeRemaining << "s\033[0m" << std::flush;
    }

    EVP_CIPHER_CTX_free(ctx);

    std::cout << "\n\033[1;32mFile split and encrypted into " << partNumber << " chunks in '" << outputDir << "'.\033[0m\n";
}

void joinFiles(const std::string &chunkDir, const std::string &outputFilePath, const std::string &password)
{
    std::vector<fs::path> chunkFiles;
    for (const auto &entry : fs::directory_iterator(chunkDir))
    {
        chunkFiles.push_back(entry.path());
    }

    std::sort(chunkFiles.begin(), chunkFiles.end(),
              [](const fs::path &a, const fs::path &b)
              {
                  std::string numA = a.filename().string();
                  std::string numB = b.filename().string();
                  size_t startA = numA.find_last_of('_') + 1;
                  size_t startB = numB.find_last_of('_') + 1;
                  return std::stoi(numA.substr(startA)) < std::stoi(numB.substr(startB));
              });

    std::ofstream outputFile(outputFilePath, std::ios::binary);
    if (!outputFile)
    {
        std::cerr << "\033[1;31mError: Unable to create output file!\033[0m\n";
        return;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    unsigned char key[32], iv[16];
    if (!deriveKeyAndIV(password, key, iv))
    {
        std::cerr << "\033[1;31mError: Failed to derive decryption key!\033[0m\n";
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    size_t chunkNumber = 0;
    const size_t progressUpdateInterval = 5;
    size_t totalBytesProcessed = 0;
    auto startTime = std::chrono::steady_clock::now();

    for (const auto &chunk : chunkFiles)
    {
        std::ifstream inputFile(chunk, std::ios::binary);
        if (!inputFile)
        {
            std::cerr << "\033[1;31mError: Unable to open chunk file " << chunk << "\033[0m\n";
            EVP_CIPHER_CTX_free(ctx);
            return;
        }

        std::vector<char> encryptedData((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());

        std::vector<char> decryptedData(encryptedData.size() + EVP_MAX_BLOCK_LENGTH);
        int outLen1 = 0, outLen2 = 0;
        EVP_DecryptUpdate(ctx, (unsigned char *)decryptedData.data(), &outLen1,
                          (unsigned char *)encryptedData.data(), encryptedData.size());
        EVP_DecryptFinal_ex(ctx, (unsigned char *)decryptedData.data() + outLen1, &outLen2);
        decryptedData.resize(outLen1 + outLen2);

        outputFile.write(decryptedData.data(), decryptedData.size());
        totalBytesProcessed += decryptedData.size();

        chunkNumber++;
        if (chunkNumber % progressUpdateInterval == 0 || chunkNumber == chunkFiles.size())
        {
            auto currentTime = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsedSeconds = currentTime - startTime;
            double progress = (static_cast<double>(chunkNumber) / chunkFiles.size()) * 100.0;
            double projectedTimeRemaining = (elapsedSeconds.count() / progress) * (100.0 - progress);

            std::cout << "\r\033[1;32mProgress: " << std::fixed << std::setprecision(2) << progress << "% ("
                      << chunkNumber << " / " << chunkFiles.size() << " chunks), "
                      << "Elapsed: " << std::fixed << std::setprecision(2) << elapsedSeconds.count() << "s, "
                      << "Remaining: " << std::fixed << std::setprecision(2) << projectedTimeRemaining << "s\033[0m" << std::flush;
        }
    }

    EVP_CIPHER_CTX_free(ctx);

    std::cout << "\n\033[1;32mFile successfully decrypted and reconstructed as '" << outputFilePath << "'.\033[0m\n";
}