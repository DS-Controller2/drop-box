#include "FileProcessor.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <fstream>

#define CHUNK_SIZE 4096 // Define a chunk size for file I/O (e.g., 4KB)

// Helper function to read file content into a vector<unsigned char>
std::vector<unsigned char> readFileContent(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        qWarning() << "Error: Could not open file for reading:" << QString::fromStdString(filePath);
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

// Helper function to write vector<unsigned char> content to a file
bool writeFileContent(const std::string& filePath, const std::vector<unsigned char>& content) {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        qWarning() << "Error: Could not open file for writing:" << QString::fromStdString(filePath);
        return false;
    }

    size_t totalWritten = 0;
    while (totalWritten < content.size()) {
        size_t bytesToWrite = std::min((size_t)CHUNK_SIZE, content.size() - totalWritten);
        if (!file.write(reinterpret_cast<const char*>(content.data() + totalWritten), bytesToWrite)) {
            qWarning() << "Error: Could not write file content:" << QString::fromStdString(filePath);
            return false;
        }
        totalWritten += bytesToWrite;
    }
    return true;
}

FileProcessor::FileProcessor(FileSystemManager *fsManager, EncryptionManager *encManager, CompressionManager *compManager, ExclusionManager *exclusionManager, MetadataManager *metadataManager, QObject *parent)
    : QObject(parent),
      fsManager(fsManager),
      encManager(encManager),
      compManager(compManager),
      exclusionManager(exclusionManager),
      metadataManager(metadataManager)
{
}

QString FileProcessor::getFileTypeSubfolder(const QString &fileName)
{
    QString suffix = QFileInfo(fileName).suffix().toLower();
    if (suffix == "txt" || suffix == "doc" || suffix == "docx" || suffix == "pdf") {
        return "documents";
    } else if (suffix == "jpg" || suffix == "jpeg" || suffix == "png" || suffix == "gif") {
        return "images";
    } else if (suffix == "zip" || suffix == "rar" || suffix == "7z") {
        return "archives";
    } else {
        return "others";
    }
}

void FileProcessor::processFile(const QString &filePath)
{
    emit fileProcessed(QString("Processing file: %1").arg(filePath));

    // 1. Exclusion Check
    if (exclusionManager->isFolderExcluded(QFileInfo(filePath).absolutePath())) {
        emit fileProcessed(QString("File %1 is in an excluded folder. Skipping processing.").arg(filePath));
        return;
    }

    // 2. Read file content
    std::vector<unsigned char> inputContent = readFileContent(filePath.toStdString());
    if (inputContent.empty()) {
        emit errorOccurred(QString("Error reading input file for processing: %1").arg(filePath));
        return;
    }

    // 3. Encrypt the file
    // For simplicity, using a fixed key. In a real app, this would be managed securely.
    std::string fixedKey = "thisisalongandsecurekeyforthisapp"; 
    std::vector<unsigned char> key(crypto_secretbox_KEYBYTES);
    std::copy(fixedKey.begin(), fixedKey.begin() + crypto_secretbox_KEYBYTES, key.begin());

    std::vector<unsigned char> encryptedContent = encManager->encrypt(inputContent, key);
    if (encryptedContent.empty()) {
        emit errorOccurred("Encryption failed for " + filePath);
        return;
    }

    // 4. Compress the encrypted file
    std::vector<unsigned char> compressedContent;
    try {
        compressedContent = compManager->compress(encryptedContent);
        if (compressedContent.empty()) {
            emit errorOccurred("Compression failed for " + filePath);
            return;
        }
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Compression Error for %1: %2").arg(filePath).arg(e.what()));
        return;
    }

    // 5. Determine file type and sort
    QString subfolder = getFileTypeSubfolder(filePath);
    QString dropBoxRootPath = QFileInfo(filePath).absolutePath(); // Assuming the monitored path is the dropbox root
    QString destinationFolder = dropBoxRootPath + QDir::separator() + subfolder;

    // Ensure destination folder exists
    fsManager->createFolderSpace(destinationFolder.toStdString());

    // Construct new file name (e.g., original_name.enc.zlib)
    QString newFileName = QFileInfo(filePath).fileName() + ".enc.zlib";
    QString destinationPath = destinationFolder + QDir::separator() + newFileName;

    // Write the processed content to the new file
    if (writeFileContent(destinationPath.toStdString(), compressedContent)) {
        emit fileProcessed(QString("Successfully processed and moved %1 to %2").arg(filePath).arg(destinationPath));
        // Remove original file after successful processing
        QFile::remove(filePath);

        // Add metadata to database
        FileMetadata metadata;
        metadata.originalFileName = QFileInfo(filePath).fileName();
        metadata.encryptedFileName = newFileName;
        metadata.fileType = subfolder;
        metadata.fileSize = compressedContent.size();
        metadata.encryptionKeyId = "fixed_key"; // Placeholder
        metadata.compressionRatio = (double)compressedContent.size() / inputContent.size();
        metadata.processingDate = QDateTime::currentDateTime();
        metadata.originalPath = filePath;
        metadata.currentStoredPath = destinationPath;
        metadataManager->addMetadata(metadata);

    } else {
        emit errorOccurred(QString("Error writing processed file to %1").arg(destinationPath));
    }
}
