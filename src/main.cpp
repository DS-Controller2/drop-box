#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QMessageBox>
#include <QFileDialog> // For file dialogs
#include <QTimer> // For simulating loading time
#include <QThread> // For QThread::msleep

#include <filesystem>
#include <iostream>
#include <fstream> // For file operations

#include "FileSystemManager.h"
#include "EncryptionManager.h"
#include "CompressionManager.h"
#include "LoadingScreen.h" // Include the new loading screen header
#include "SettingsDialog.h" // Include the new settings dialog header
#include "FileMonitor.h" // Include the new file monitor header
#include "MetadataManager.h" // Include the new metadata manager header

#define CHUNK_SIZE 4096 // Define a chunk size for file I/O (e.g., 4KB)

namespace fs = std::filesystem;

// Helper function to read file content into a vector<unsigned char>
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

// Helper function to write vector<unsigned char> content to a file
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

class DropBoxMainWindow : public QMainWindow {
    Q_OBJECT

public:
    DropBoxMainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Secure Drop-Box");
        setGeometry(100, 100, 800, 600);

        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

        // --- FileSystemManager Section ---
        QLabel *fsLabel = new QLabel("File System Management", this);
        mainLayout->addWidget(fsLabel);

        QLineEdit *folderSpaceNameInput = new QLineEdit(this);
        folderSpaceNameInput->setPlaceholderText("Enter folder space name");
        mainLayout->addWidget(folderSpaceNameInput);

        QPushButton *createFolderSpaceButton = new QPushButton("Create Folder Space", this);
        mainLayout->addWidget(createFolderSpaceButton);

        QLineEdit *filePathInput = new QLineEdit(this);
        filePathInput->setPlaceholderText("Enter file path to add");
        mainLayout->addWidget(filePathInput);

        QPushButton *addFileButton = new QPushButton("Add File", this);
        mainLayout->addWidget(addFileButton);

        // --- Drop-box Location Section ---
        QLabel *dropboxLocationLabel = new QLabel("\nDrop-box Location", this);
        mainLayout->addWidget(dropboxLocationLabel);

        QLineEdit *dropboxLocationInput = new QLineEdit(this);
        dropboxLocationInput->setPlaceholderText("Select Drop-box location");
        dropboxLocationInput->setReadOnly(true); // Make it read-only
        mainLayout->addWidget(dropboxLocationInput);

        QPushButton *browseDropboxLocationButton = new QPushButton("Browse Drop-box Location", this);
        mainLayout->addWidget(browseDropboxLocationButton);

        QPushButton *setDropboxLocationButton = new QPushButton("Set Drop-box Location", this);
        mainLayout->addWidget(setDropboxLocationButton);

        // Connect signal for browsing Drop-box location
        connect(browseDropboxLocationButton, &QPushButton::clicked, [=]() {
            QString directory = QFileDialog::getExistingDirectory(this, "Select Drop-box Location");
            if (!directory.isEmpty()) {
                dropboxLocationInput->setText(directory);
            }
        });

        // Connect signal for setting Drop-box location
        connect(setDropboxLocationButton, &QPushButton::clicked, [=]() {
            QString selectedPath = dropboxLocationInput->text();
            if (selectedPath.isEmpty()) {
                QMessageBox::warning(this, "Selection Required", "Please select a valid location for your Drop-box folder.");
                return;
            }
            fs::path dropBoxPath = (fs::path(selectedPath.toStdString()) / "my_drop_box_space");
            fsManager.createFolderSpace(dropBoxPath.string());
            feedbackTextEdit->append(QString("Drop-box folder created at: %1").arg(QString::fromStdString(dropBoxPath.string())));

            // Start monitoring the new Drop-box location
            fileMonitor->stopMonitoring(); // Stop previous monitoring if any
            fileMonitor->startMonitoring(QString::fromStdString(dropBoxPath.string()));
        });

        // Connect FileMonitor signals
        connect(fileMonitor, &FileMonitor::fileAdded, this, &DropBoxMainWindow::processMonitoredFile);
        connect(fileMonitor, &FileMonitor::fileChanged, this, &DropBoxMainWindow::processMonitoredFile);

        // --- EncryptionManager Section ---
        QLabel *encLabel = new QLabel("\nEncryption/Decryption", this);
        mainLayout->addWidget(encLabel);

        QLineEdit *encInputFile = new QLineEdit(this);
        encInputFile->setPlaceholderText("Input File for Encryption/Decryption");
        mainLayout->addWidget(encInputFile);

        QPushButton *browseEncInput = new QPushButton("Browse Input", this);
        mainLayout->addWidget(browseEncInput);

        QLineEdit *encOutputFile = new QLineEdit(this);
        encOutputFile->setPlaceholderText("Output File for Encryption/Decryption");
        mainLayout->addWidget(encOutputFile);

        QPushButton *browseEncOutput = new QPushButton("Browse Output", this);
        mainLayout->addWidget(browseEncOutput);

        QLineEdit *encKey = new QLineEdit(this);
        encKey->setPlaceholderText("Encryption/Decryption Key (32 chars)");
        mainLayout->addWidget(encKey);

        QPushButton *encryptButton = new QPushButton("Encrypt File", this);
        mainLayout->addWidget(encryptButton);

        QPushButton *decryptButton = new QPushButton("Decrypt File", this);
        mainLayout->addWidget(decryptButton);

        // --- CompressionManager Section ---
        QLabel *compLabel = new QLabel("\nCompression/Decompression", this);
        mainLayout->addWidget(compLabel);

        QLineEdit *compInputFile = new QLineEdit(this);
        compInputFile->setPlaceholderText("Input File for Compression/Decompression");
        mainLayout->addWidget(compInputFile);

        QPushButton *browseCompInput = new QPushButton("Browse Input", this);
        mainLayout->addWidget(browseCompInput);

        QLineEdit *compOutputFile = new QLineEdit(this);
        compOutputFile->setPlaceholderText("Output File for Compression/Decompression");
        mainLayout->addWidget(compOutputFile);

        QPushButton *browseCompOutput = new QPushButton("Browse Output", this);
        mainLayout->addWidget(browseCompOutput);

        QPushButton *compressButton = new QPushButton("Compress File", this);
        mainLayout->addWidget(compressButton);

        QPushButton *decompressButton = new QPushButton("Decompress File", this);
        mainLayout->addWidget(decompressButton);

        // --- File Search Section ---
        QLabel *searchLabel = new QLabel("File Search", this);
        mainLayout->addWidget(searchLabel);

        QLineEdit *searchQueryInput = new QLineEdit(this);
        searchQueryInput->setPlaceholderText("Enter search query");
        mainLayout->addWidget(searchQueryInput);

        QPushButton *searchButton = new QPushButton("Search Files", this);
        mainLayout->addWidget(searchButton);

        QListWidget *searchResultsList = new QListWidget(this);
        mainLayout->addWidget(searchResultsList);

        // Connect search button
        connect(searchButton, &QPushButton::clicked, [=]() {
            QString query = searchQueryInput->text();
            if (query.isEmpty()) {
                QMessageBox::warning(this, "Input Required", "Please enter a search query.");
                return;
            }
            searchResultsList->clear();
            QList<FileMetadata> results = metadataManager.searchMetadata(query);
            for (const FileMetadata &metadata : results) {
                searchResultsList->addItem(QString("Original: %1, Encrypted: %2, Type: %3").arg(metadata.originalFileName).arg(metadata.encryptedFileName).arg(metadata.fileType));
            }
            if (results.isEmpty()) {
                searchResultsList->addItem("No results found.");
            }
        });

        // Feedback display
        feedbackTextEdit = new QTextEdit(this);
        feedbackTextEdit->setReadOnly(true);
        mainLayout->addWidget(feedbackTextEdit);

        fileMonitor = new FileMonitor(this);

        // Settings Button
        QPushButton *settingsButton = new QPushButton("Settings", this);
        mainLayout->addWidget(settingsButton);

        connect(settingsButton, &QPushButton::clicked, [=]() {
            SettingsDialog settingsDialog(this);
            settingsDialog.exec();
        });

        // Connect signals and slots for FileSystemManager
        connect(createFolderSpaceButton, &QPushButton::clicked, [=]() {
            std::string folderSpaceName = folderSpaceNameInput->text().toStdString();
            if (folderSpaceName.empty()) {
                QMessageBox::warning(this, "Input Required", "Please enter a name for the folder space.");
                return;
            }
            fs::path applicationDirPath = QCoreApplication::applicationDirPath().toStdString();
            fs::path projectRootPath = applicationDirPath.parent_path(); // Assuming project root is one level up from executable
            std::string folderSpacePath = (projectRootPath / folderSpaceName).string();
            
            fsManager.createFolderSpace(folderSpacePath);
            feedbackTextEdit->append(QString("Created folder space: %1").arg(QString::fromStdString(folderSpacePath)));
        });

        connect(addFileButton, &QPushButton::clicked, [=]() {
            std::string folderSpaceName = folderSpaceNameInput->text().toStdString();
            std::string filePath = filePathInput->text().toStdString();
            if (folderSpaceName.empty() || filePath.empty()) {
                QMessageBox::warning(this, "Input Required", "Please enter both a folder space name and a file path to add.");
                return;
            }
            fs::path applicationDirPath = QCoreApplication::applicationDirPath().toStdString();
            fs::path projectRootPath = applicationDirPath.parent_path(); // Assuming project root is one level up from executable
            std::string folderSpacePath = (projectRootPath / folderSpaceName).string();
            
            fsManager.addFile(folderSpacePath, fs::absolute(filePath).string());
            feedbackTextEdit->append(QString("Added file %1 to %2").arg(QString::fromStdString(filePath)).arg(QString::fromStdString(folderSpacePath)));
        });

        // Connect signals and slots for EncryptionManager
        connect(browseEncInput, &QPushButton::clicked, [=]() {
            QString fileName = QFileDialog::getOpenFileName(this, "Select Input File");
            if (!fileName.isEmpty()) {
                encInputFile->setText(fileName);
            }
        });

        connect(browseEncOutput, &QPushButton::clicked, [=]() {
            QString fileName = QFileDialog::getSaveFileName(this, "Select Output File");
            if (!fileName.isEmpty()) {
                encOutputFile->setText(fileName);
            }
        });

        connect(encryptButton, &QPushButton::clicked, [=]() {
            std::string inputFile = encInputFile->text().toStdString();
            std::string outputFile = encOutputFile->text().toStdString();
            std::string keyString = encKey->text().toStdString();

            if (inputFile.empty() || outputFile.empty() || keyString.empty()) {
                QMessageBox::warning(this, "Input Required", "Please provide an input file, an output file, and an encryption key for encryption.");
                return;
            }
            if (keyString.length() < crypto_secretbox_KEYBYTES) {
                QMessageBox::warning(this, "Invalid Key Length", QString("The encryption key must be at least %1 characters long for encryption.").arg(crypto_secretbox_KEYBYTES));
                return;
            }

            std::vector<unsigned char> inputContent = readFileContent(inputFile);
            if (inputContent.empty()) {
                feedbackTextEdit->append(QString("Error reading input file: %1").arg(QString::fromStdString(inputFile)));
                return;
            }

            std::vector<unsigned char> key(crypto_secretbox_KEYBYTES);
            std::copy(keyString.begin(), keyString.begin() + crypto_secretbox_KEYBYTES, key.begin());

            // For simplicity, using a fixed HMAC key. In a real app, this would be managed securely.
            std::string fixedHmacKey = "thisisalongandsecurehmackeyforthisapp";
            std::vector<unsigned char> hmac_key(EncryptionManager::HMAC_KEY_SIZE);
            std::copy(fixedHmacKey.begin(), fixedHmacKey.begin() + EncryptionManager::HMAC_KEY_SIZE, hmac_key.begin());

            std::vector<unsigned char> decryptedContent = encManager.decrypt(inputContent, key, hmac_key);
            if (decryptedContent.empty()) {
                feedbackTextEdit->append("Decryption failed.");
                return;
            }

            if (writeFileContent(outputFile, decryptedContent)) {
                feedbackTextEdit->append(QString("Successfully decrypted %1 to %2").arg(QString::fromStdString(inputFile)).arg(QString::fromStdString(outputFile)));
            } else {
                feedbackTextEdit->append(QString("Error writing output file: %1").arg(QString::fromStdString(outputFile)));
            }
        });

        // Connect signals and slots for CompressionManager
        connect(browseCompInput, &QPushButton::clicked, [=]() {
            QString fileName = QFileDialog::getOpenFileName(this, "Select Input File");
            if (!fileName.isEmpty()) {
                compInputFile->setText(fileName);
            }
        });

        connect(browseCompOutput, &QPushButton::clicked, [=]() {
            QString fileName = QFileDialog::getSaveFileName(this, "Select Output File");
            if (!fileName.isEmpty()) {
                compOutputFile->setText(fileName);
            }
        });

        connect(compressButton, &QPushButton::clicked, [=]() {
            std::string inputFile = compInputFile->text().toStdString();
            std::string outputFile = compOutputFile->text().toStdString();

            if (inputFile.empty() || outputFile.empty()) {
                QMessageBox::warning(this, "Input Required", "Please provide both an input file and an output file for compression.");
                return;
            }

            std::vector<unsigned char> inputContent = readFileContent(inputFile);
            if (inputContent.empty()) {
                feedbackTextEdit->append(QString("Error reading input file: %1").arg(QString::fromStdString(inputFile)));
                return;
            }

            try {
                std::vector<unsigned char> compressedContent = compManager.compress(inputContent);
                if (compressedContent.empty()) {
                    feedbackTextEdit->append("Compression failed.");
                    return;
                }

                if (writeFileContent(outputFile, compressedContent)) {
                    feedbackTextEdit->append(QString("Successfully compressed %1 to %2").arg(QString::fromStdString(inputFile)).arg(QString::fromStdString(outputFile)));
                } else {
                    feedbackTextEdit->append(QString("Error writing output file: %1").arg(QString::fromStdString(outputFile)));
                }
            } catch (const std::exception& e) {
                feedbackTextEdit->append(QString("Compression Error: %1").arg(e.what()));
            }
        });

        connect(decompressButton, &QPushButton::clicked, [=]() {
            std::string inputFile = compInputFile->text().toStdString();
            std::string outputFile = compOutputFile->text().toStdString();

            if (inputFile.empty() || outputFile.empty()) {
                QMessageBox::warning(this, "Input Required", "Please provide both an input file and an output file for decompression.");
                return;
            }

            std::vector<unsigned char> inputContent = readFileContent(inputFile);
            if (inputContent.empty()) {
                feedbackTextEdit->append(QString("Error reading input file: %1").arg(QString::fromStdString(inputFile)));
                return;
            }

            try {
                std::vector<unsigned char> decompressedContent = compManager.decompress(inputContent);
                if (decompressedContent.empty()) {
                    feedbackTextEdit->append("Decompression failed.");
                    return;
                }

                if (writeFileContent(outputFile, decompressedContent)) {
                    feedbackTextEdit->append(QString("Successfully decompressed %1 to %2").arg(QString::fromStdString(inputFile)).arg(QString::fromStdString(outputFile)));
                } else {
                    feedbackTextEdit->append(QString("Error writing output file: %1").arg(QString::fromStdString(outputFile)));
                }
            } catch (const std::exception& e) {
                feedbackTextEdit->append(QString("Decompression Error: %1").arg(e.what()));
            }
        });
    }

private:
    FileSystemManager fsManager;
    EncryptionManager encManager;
    CompressionManager compManager;
    QTextEdit *feedbackTextEdit;
    FileMonitor *fileMonitor;
    ExclusionManager exclusionManager;
    MetadataManager metadataManager;

private slots:
    void processMonitoredFile(const QString &filePath);

private:
    QString getFileTypeSubfolder(const QString &fileName);
};

QString DropBoxMainWindow::getFileTypeSubfolder(const QString &fileName)
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

void DropBoxMainWindow::processMonitoredFile(const QString &filePath)
{
    feedbackTextEdit->append(QString("Processing file: %1").arg(filePath));

    // 1. Exclusion Check
    if (exclusionManager.isFolderExcluded(QFileInfo(filePath).absolutePath())) {
        feedbackTextEdit->append(QString("File %1 is in an excluded folder. Skipping processing.").arg(filePath));
        return;
    }

    // 2. Read file content
    std::vector<unsigned char> inputContent = readFileContent(filePath.toStdString());
    if (inputContent.empty()) {
        feedbackTextEdit->append(QString("Error reading input file for processing: %1").arg(filePath));
        return;
    }

    // 3. Encrypt the file
    // For simplicity, using a fixed key. In a real app, this would be managed securely.
    std::string fixedKey = "thisisalongandsecurekeyforthisapp"; 
    std::vector<unsigned char> key(crypto_secretbox_KEYBYTES);
    std::copy(fixedKey.begin(), fixedKey.begin() + crypto_secretbox_KEYBYTES, key.begin());

    // For simplicity, using a fixed HMAC key. In a real app, this would be managed securely.
    std::string fixedHmacKey = "thisisalongandsecurehmackeyforthisapp";
    std::vector<unsigned char> hmac_key(EncryptionManager::HMAC_KEY_SIZE);
    std::copy(fixedHmacKey.begin(), fixedHmacKey.begin() + EncryptionManager::HMAC_KEY_SIZE, hmac_key.begin());

    std::vector<unsigned char> encryptedContent = encManager.encrypt(inputContent, key, hmac_key);
    if (encryptedContent.empty()) {
        feedbackTextEdit->append("Encryption failed for " + filePath);
        return;
    }

    // 4. Compress the encrypted file
    std::vector<unsigned char> compressedContent;
    try {
        compressedContent = compManager.compress(encryptedContent);
        if (compressedContent.empty()) {
            feedbackTextEdit->append("Compression failed for " + filePath);
            return;
        }
    } catch (const std::exception& e) {
        feedbackTextEdit->append(QString("Compression Error for %1: %2").arg(filePath).arg(e.what()));
        return;
    }

    // 5. Determine file type and sort
    QString subfolder = getFileTypeSubfolder(filePath);
    QString dropBoxRootPath = QFileInfo(filePath).absolutePath(); // Assuming the monitored path is the dropbox root
    QString destinationFolder = dropBoxRootPath + QDir::separator() + subfolder;

    // Ensure destination folder exists
    fsManager.createFolderSpace(destinationFolder.toStdString());

    // Construct new file name (e.g., original_name.enc.zlib)
    QString newFileName = QFileInfo(filePath).fileName() + ".enc.zlib";
    QString destinationPath = destinationFolder + QDir::separator() + newFileName;

    // Write the processed content to the new file
    if (writeFileContent(destinationPath.toStdString(), compressedContent)) {
        feedbackTextEdit->append(QString("Successfully processed and moved %1 to %2").arg(filePath).arg(destinationPath));
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
        metadataManager.addMetadata(metadata);

    } else {
        feedbackTextEdit->append(QString("Error writing processed file to %1").arg(destinationPath));
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    LoadingScreen loadingScreen;
    loadingScreen.show();
    app.processEvents(); // Process events to ensure the loading screen is shown immediately

    // Simulate some loading time
    for (int i = 0; i <= 100; i += 10) {
        loadingScreen.setMessage(QString("Loading application... %1%").arg(i));
        loadingScreen.setProgress(i);
        app.processEvents(); // Update UI
        QThread::msleep(100); // Simulate work
    }

    DropBoxMainWindow window;
    window.show();
    loadingScreen.close(); // Close the loading screen

    return app.exec();
}

#include "main.moc"