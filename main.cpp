#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <termios.h>
#include <unistd.h>

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
    std::ifstream input(filePath, std::ios::binary);
    if (!input)
    {
        std::cerr << "\033[1;31mError: Cannot open input file!\033[0m\n";
        return;
    }

    fs::path path(filePath);
    std::string dir = path.stem().string() + "_chunks";
    fs::create_directory(dir);

    std::vector<char> buffer(chunkSize);
    size_t partNum = 0;
    size_t totalBytes = 0;

    while (input)
    {
        input.read(buffer.data(), chunkSize);
        std::streamsize bytesRead = input.gcount();

        if (bytesRead <= 0)
            break;

        // Use padding to ensure proper numeric ordering
        std::string outPath = dir + "/chunk_" + std::string(5 - std::to_string(partNum).length(), '0') + std::to_string(partNum++);
        std::ofstream chunk(outPath, std::ios::binary);

        if (!chunk)
        {
            std::cerr << "\033[1;31mError: Cannot create chunk file!\033[0m\n";
            return;
        }

        chunk.write(buffer.data(), bytesRead);
        totalBytes += bytesRead;
    }

    std::cout << "\033[1;32mSplit complete: " << partNum << " chunks, " << totalBytes << " total bytes\033[0m\n";
}

void joinFiles(const std::string &chunkDir, const std::string &outputPath)
{
    std::vector<fs::path> chunks;
    for (const auto &entry : fs::directory_iterator(chunkDir))
    {
        chunks.push_back(entry.path());
    }

    // Sort numerically by extracting the number from the chunk filename
    std::sort(chunks.begin(), chunks.end(),
              [](const fs::path &a, const fs::path &b)
              {
                  std::string numA = a.filename().string();
                  std::string numB = b.filename().string();
                  size_t startA = numA.find_last_of('_') + 1;
                  size_t startB = numB.find_last_of('_') + 1;
                  return std::stoi(numA.substr(startA)) < std::stoi(numB.substr(startB));
              });

    std::ofstream output(outputPath, std::ios::binary);
    if (!output)
    {
        std::cerr << "\033[1;31mError: Cannot create output file!\033[0m\n";
        return;
    }

    size_t totalBytes = 0;
    size_t chunkNum = 0;

    for (const auto &chunk : chunks)
    {
        std::ifstream input(chunk, std::ios::binary);
        if (!input)
        {
            std::cerr << "\033[1;31mError: Cannot open chunk " << chunk << "\033[0m\n";
            continue;
        }

        input.seekg(0, std::ios::end);
        size_t size = input.tellg();
        input.seekg(0);

        std::vector<char> buffer(size);
        input.read(buffer.data(), size);
        output.write(buffer.data(), size);

        totalBytes += size;
        chunkNum++;
    }

    std::cout << "\033[1;32mJoin complete: " << chunkNum << " chunks, " << totalBytes << " total bytes\033[0m\n";
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
        std::string chunkDir, outputPath;
        std::cout << "\033[1;34mEnter chunk directory: \033[0m";
        std::getline(std::cin, chunkDir);
        std::cout << "\033[1;34mEnter output file path: \033[0m";
        std::getline(std::cin, outputPath);
        joinFiles(chunkDir, outputPath);
    }
    return 0;
}