#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

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

int main() {
    std::string filePath;
    size_t chunkSize;

    std::cout << "Enter file path: ";
    std::cin >> filePath;
    std::cout << "Enter chunk size (bytes): ";
    std::cin >> chunkSize;

    splitFile(filePath, chunkSize);
    return 0;
}
