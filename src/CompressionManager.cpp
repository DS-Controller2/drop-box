#include "CompressionManager.h"
#include <iostream>
#include <stdexcept> // For std::runtime_error

// Function to compress data
std::vector<unsigned char> CompressionManager::compress(const std::vector<unsigned char>& data) {
    if (data.empty()) {
        return {};
    }

    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    zs.avail_in = data.size();
    zs.next_in = reinterpret_cast<Bytef*>(const_cast<unsigned char*>(data.data()));

    if (deflateInit(&zs, Z_DEFAULT_COMPRESSION) != Z_OK) {
        throw std::runtime_error("deflateInit failed");
    }

    // Estimate output buffer size
    size_t compressed_buffer_size = deflateBound(&zs, data.size());
    std::vector<unsigned char> compressed_data(compressed_buffer_size);
    zs.avail_out = compressed_buffer_size;
    zs.next_out = reinterpret_cast<Bytef*>(compressed_data.data());

    int ret = deflate(&zs, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&zs);
        if (ret == Z_OK) {
            throw std::runtime_error("deflate did not finish or buffer too small");
        }
        throw std::runtime_error("deflate failed with error: " + std::to_string(ret));
    }

    deflateEnd(&zs);
    compressed_data.resize(compressed_buffer_size - zs.avail_out);
    return compressed_data;
}

// Function to decompress data
std::vector<unsigned char> CompressionManager::decompress(const std::vector<unsigned char>& compressed_data) {
    if (compressed_data.empty()) {
        return {};
    }

    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    zs.avail_in = compressed_data.size();
    zs.next_in = reinterpret_cast<Bytef*>(const_cast<unsigned char*>(compressed_data.data()));

    if (inflateInit(&zs) != Z_OK) {
        throw std::runtime_error("inflateInit failed");
    }

    // Initial guess for decompressed size, will resize if needed
    size_t decompressed_buffer_size = compressed_data.size() * 2;
    std::vector<unsigned char> decompressed_data(decompressed_buffer_size);
    zs.avail_out = decompressed_buffer_size;
    zs.next_out = reinterpret_cast<Bytef*>(decompressed_data.data());

    int ret;
    do {
        ret = inflate(&zs, Z_NO_FLUSH);
        switch (ret) {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                inflateEnd(&zs);
                throw std::runtime_error("zlib inflate error: " + std::to_string(ret));
        }
        if (zs.avail_out == 0) { // Output buffer full, resize and continue
            size_t current_size = decompressed_data.size();
            decompressed_data.resize(current_size * 2);
            zs.next_out = reinterpret_cast<Bytef*>(decompressed_data.data() + current_size);
            zs.avail_out = decompressed_data.size() - current_size;
        }
    } while (ret != Z_STREAM_END);

    inflateEnd(&zs);
    decompressed_data.resize(decompressed_buffer_size - zs.avail_out);
    return decompressed_data;
}