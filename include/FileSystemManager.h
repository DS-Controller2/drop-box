#ifndef FILESYSTEM_MANAGER_H
#define FILESYSTEM_MANAGER_H

#include <string>
#include <filesystem>

class FileSystemManager {
public:
    void createFolderSpace(const std::string& path);
    void addFile(const std::string& folderSpacePath, const std::string& filePath);
};

#endif // FILESYSTEM_MANAGER_H
