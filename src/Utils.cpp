#include "Utils.h"
#include <iostream>
#include <fstream>
#include <vector>

#define CHUNK_SIZE 4096 // Define a chunk size for file I/O (e.g., 4KB)

namespace Utils {
    void log(const std::string& message) {
        std::cout << "[LOG] " << message << std::endl;
    }

    std::vector<unsigned char> readFileContent(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file for reading: " << filePath << std::endl;
            return {};
        }

        std::vector<unsigned char> buffer;
        char chunk[CHUNK_SIZE];
        while (file.read(chunk, sizeof(chunk))) {
            buffer.insert(buffer.end(), chunk, chunk + sizeof(chunk));
        }
        buffer.insert(buffer.end(), chunk, chunk + file.gcount());

        return buffer;
    }

    bool writeFileContent(const std::string& filePath, const std::vector<unsigned char>& content) {
        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file for writing: " << filePath << std::endl;
            return false;
        }

        size_t totalWritten = 0;
        while (totalWritten < content.size()) {
            size_t bytesToWrite = std::min((size_t)CHUNK_SIZE, content.size() - totalWritten);
            if (!file.write(reinterpret_cast<const char*>(content.data() + totalWritten), bytesToWrite)) {
                std::cerr << "Error: Could not write file content: " << filePath << std::endl;
                return false;
            }
            totalWritten += bytesToWrite;
        }
        return true;
    }
}
