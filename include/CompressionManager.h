#ifndef COMPRESSION_MANAGER_H
#define COMPRESSION_MANAGER_H

#include <vector>
#include <zlib.h> // Include zlib header

class CompressionManager {
public:
    std::vector<unsigned char> compress(const std::vector<unsigned char>& data);
    std::vector<unsigned char> decompress(const std::vector<unsigned char>& data);
};

#endif // COMPRESSION_MANAGER_H