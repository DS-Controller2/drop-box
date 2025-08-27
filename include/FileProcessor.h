#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

#include <vector>
#include <string>

#include "FileSystemManager.h"
#include "EncryptionManager.h"
#include "CompressionManager.h"
#include "ExclusionManager.h"
#include "MetadataManager.h"
#include "Utils.h"

class FileProcessor : public QObject
{
    Q_OBJECT

public:
    explicit FileProcessor(FileSystemManager *fsManager, EncryptionManager *encManager, CompressionManager *compManager, ExclusionManager *exclusionManager, MetadataManager *metadataManager, QObject *parent = nullptr);

signals:
    void fileProcessed(const QString &message);
    void errorOccurred(const QString &message);

public slots:
    void processFile(const QString &filePath);

private:
    FileSystemManager *fsManager;
    EncryptionManager *encManager;
    CompressionManager *compManager;
    ExclusionManager *exclusionManager;
    MetadataManager *metadataManager;

    QString getFileTypeSubfolder(const QString &fileName);
};

#endif // FILEPROCESSOR_H
