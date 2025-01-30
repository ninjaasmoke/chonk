#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

void splitFile(const std::string& filePath, size_t chunkSize) {
    std::ifstream inputFile(filePath, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Error: Unable to open file!\n";
        return;
    }

    fs::path inputPath(filePath);
    std::string outputDir = inputPath.stem().string() + "_chunks";
    fs::create_directory(outputDir);

    std::vector<char> buffer(chunkSize);
    size_t partNumber = 0;
    while (inputFile) {
        inputFile.read(buffer.data(), chunkSize);
        std::streamsize bytesRead = inputFile.gcount();
        if (bytesRead <= 0) break;

        std::ofstream outputFile(outputDir + "/chunk_" + std::to_string(partNumber), std::ios::binary);
        if (!outputFile) {
            std::cerr << "Error: Unable to create chunk file!\n";
            return;
        }
        outputFile.write(buffer.data(), bytesRead);
        partNumber++;
    }
    std::cout << "File split into " << partNumber << " chunks in '" << outputDir << "'.\n";
}

void joinFiles(const std::string& chunkDir, const std::string& outputFilePath) {
    std::vector<std::string> chunkFiles;
    for (const auto& entry : fs::directory_iterator(chunkDir)) {
        chunkFiles.push_back(entry.path().string());
    }
    
    std::sort(chunkFiles.begin(), chunkFiles.end());
    
    std::ofstream outputFile(outputFilePath, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: Unable to create output file!\n";
        return;
    }
    
    for (const auto& chunk : chunkFiles) {
        std::ifstream inputFile(chunk, std::ios::binary);
        if (!inputFile) {
            std::cerr << "Error: Unable to open chunk file " << chunk << "\n";
            return;
        }
        outputFile << inputFile.rdbuf();
    }
    
    std::cout << "File successfully reconstructed as '" << outputFilePath << "'.\n";
}

int main() {
    int choice;
    std::cout << "1. Split File\n2. Join Files\nEnter choice: ";
    std::cin >> choice;
    
    if (choice == 1) {
        std::string filePath;
        size_t chunkSize;
        std::cout << "Enter file path: ";
        std::cin >> filePath;
        std::cout << "Enter chunk size (bytes): ";
        std::cin >> chunkSize;
        splitFile(filePath, chunkSize);
    } else if (choice == 2) {
        std::string chunkDir, outputFilePath;
        std::cout << "Enter chunk directory: ";
        std::cin >> chunkDir;
        std::cout << "Enter output file path: ";
        std::cin >> outputFilePath;
        joinFiles(chunkDir, outputFilePath);
    } else {
        std::cerr << "Invalid choice!\n";
    }
    return 0;
}
