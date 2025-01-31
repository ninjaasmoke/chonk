#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

void splitFile(const std::string &filePath, size_t chunkSize, const std::string &password);
void joinFiles(const std::string &chunkDir, const std::string &outputFilePath, const std::string &password);

#endif // FILE_OPERATIONS_H