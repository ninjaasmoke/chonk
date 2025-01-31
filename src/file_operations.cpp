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

    // Get the total size of the file
    inputFile.seekg(0, std::ios::end);
    size_t totalFileSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    fs::path inputPath(filePath);
    std::string outputDir = inputPath.stem().string() + "_chunks";
    fs::create_directory(outputDir);

    std::vector<char> buffer(chunkSize);
    size_t partNumber = 0;
    size_t totalBytesProcessed = 0;

    while (inputFile)
    {
        inputFile.read(buffer.data(), chunkSize);
        std::streamsize bytesRead = inputFile.gcount();
        if (bytesRead <= 0)
            break;

        auto encryptedData = encryptChunk({buffer.begin(), buffer.begin() + bytesRead}, password);
        // Zero-padded numbers for correct sorting
        std::string chunkPath = outputDir + "/chunk_" + std::string(5 - std::to_string(partNumber).length(), '0') + std::to_string(partNumber);
        std::ofstream outputFile(chunkPath, std::ios::binary);

        if (!outputFile)
        {
            std::cerr << "\033[1;31mError: Unable to create chunk file!\033[0m\n";
            return;
        }

        outputFile.write((char *)encryptedData.data(), encryptedData.size());
        totalBytesProcessed += bytesRead;
        partNumber++;

        // Calculate progress percentage
        double progress = (static_cast<double>(totalBytesProcessed) / totalFileSize) * 100.0;

        // Display progress in a single line
        std::cout << "\r\033[1;32mProgress: " << std::fixed << std::setprecision(2) << progress << "% ("
                  << totalBytesProcessed << " / " << totalFileSize << " bytes)\033[0m" << std::flush;
    }

    std::cout << "\n\033[1;32mFile split and encrypted into " << partNumber << " chunks in '" << outputDir << "'.\033[0m\n";
}

void joinFiles(const std::string &chunkDir, const std::string &outputFilePath, const std::string &password)
{
    std::vector<fs::path> chunkFiles;
    for (const auto &entry : fs::directory_iterator(chunkDir))
    {
        chunkFiles.push_back(entry.path());
    }

    // Sort numerically by extracting numbers from filenames
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

    size_t chunkNumber = 0;
    for (const auto &chunk : chunkFiles)
    {
        std::ifstream inputFile(chunk, std::ios::binary);
        if (!inputFile)
        {
            std::cerr << "\033[1;31mError: Unable to open chunk file " << chunk << "\033[0m\n";
            return;
        }

        // Read entire chunk
        std::vector<char> encryptedData((std::istreambuf_iterator<char>(inputFile)),
                                        std::istreambuf_iterator<char>());

        auto decryptedData = decryptChunk({encryptedData.begin(), encryptedData.end()}, password);
        outputFile.write(decryptedData.data(), decryptedData.size());

        // Calculate progress percentage
        double progress = (static_cast<double>(chunkNumber + 1) / chunkFiles.size()) * 100.0;

        // Display progress in a single line
        std::cout << "\r\033[1;32mProgress: " << std::fixed << std::setprecision(2) << progress << "% ("
                  << chunkNumber + 1 << " / " << chunkFiles.size() << " chunks)\033[0m" << std::flush;

        chunkNumber++;
    }

    std::cout << "\033[1;32mFile successfully decrypted and reconstructed as '" << outputFilePath << "'.\033[0m\n";
}