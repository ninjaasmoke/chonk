#include <iostream>
#include "banner.h"
#include "input_utils.h"
#include "file_operations.h"

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
        std::cout << "\033[1;34mEnter chunk size (Mega bytes): \033[0m";
        std::cin >> chunkSize;
        splitFile(filePath, chunkSize * 1048576, password);
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