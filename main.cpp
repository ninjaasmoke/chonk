#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <termios.h>
#include <unistd.h>
#include <openssl/evp.h>

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

void splitFile(const std::string &filePath, size_t chunkSize)
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

        std::ofstream outputFile(outputDir + "/chunk_" + std::to_string(partNumber), std::ios::binary);
        if (!outputFile)
        {
            std::cerr << "\033[1;31mError: Unable to create chunk file!\033[0m\n";
            return;
        }
        outputFile.write(buffer.data(), bytesRead);
        partNumber++;
    }
    std::cout << "\033[1;32mFile split into " << partNumber << " chunks in '" << outputDir << "'.\033[0m\n";
}

void joinFiles(const std::string &chunkDir, const std::string &outputFilePath)
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
        outputFile << inputFile.rdbuf();
    }

    std::cout << "\033[1;32mFile successfully reconstructed as '" << outputFilePath << "'.\033[0m\n";
}

int main()
{
    printBanner();
    int choice;
    std::cout << "\033[1;33m1. Split File\n2. Join Files\nEnter choice: \033[0m";
    std::cin >> choice;
    std::cin.ignore();

    if (choice == 1)
    {
        std::string filePath;
        size_t chunkSize;
        std::cout << "\033[1;34mEnter file path: \033[0m";
        std::getline(std::cin, filePath);
        std::cout << "\033[1;34mEnter chunk size (bytes): \033[0m";
        std::cin >> chunkSize;
        splitFile(filePath, chunkSize);
    }
    else if (choice == 2)
    {
        std::string chunkDir, outputFilePath;
        std::cout << "\033[1;34mEnter chunk directory: \033[0m";
        std::getline(std::cin, chunkDir);
        std::cout << "\033[1;34mEnter output file path: \033[0m";
        std::getline(std::cin, outputFilePath);
        joinFiles(chunkDir, outputFilePath);
    }
    else
    {
        std::cerr << "\033[1;31mInvalid choice!\033[0m\n";
    }
    return 0;
}