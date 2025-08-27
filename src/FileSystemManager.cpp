#include "FileSystemManager.h"
#include <iostream>
#include <filesystem> // Include filesystem

namespace fs = std::filesystem; // Use namespace alias for brevity

void FileSystemManager::createFolderSpace(const std::string& path) {
    std::cout << "Attempting to create folder space at: " << path << std::endl;
    try {
        if (fs::create_directory(path)) {
            std::cout << "Successfully created folder space at: " << path << std::endl;
        } else {
            std::cout << "Folder space already exists or could not be created at: " << path << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating folder space: " << e.what() << std::endl;
    }
}

void FileSystemManager::addFile(const std::string& folderSpacePath, const std::string& filePath) {
    std::cout << "Attempting to add file " << filePath << " to folder space: " << folderSpacePath << std::endl;
    try {
        fs::path sourcePath = filePath;
        fs::path destinationPath = fs::path(folderSpacePath) / sourcePath.filename();

        fs::copy(sourcePath, destinationPath, fs::copy_options::overwrite_existing);
        std::cout << "Successfully added file " << filePath << " to " << destinationPath << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error adding file: " << e.what() << std::endl;
    }
}