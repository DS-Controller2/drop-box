#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace Utils {
    void log(const std::string& message);
    std::vector<unsigned char> readFileContent(const std::string& filePath);
    bool writeFileContent(const std::string& filePath, const std::vector<unsigned char>& content);
}

#endif // UTILS_H
