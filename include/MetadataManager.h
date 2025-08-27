#ifndef METADATAMANAGER_H
#define METADATAMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QDateTime>
struct FileMetadata {
    QString originalFileName;
    QString encryptedFileName;
    QString fileType;
    qint64 fileSize;
    QString encryptionKeyId; // Placeholder for key management
    double compressionRatio;
    QDateTime processingDate;
    QString originalPath;
    QString currentStoredPath;
};

class MetadataManager : public QObject
{
    Q_OBJECT

public:
    explicit MetadataManager(QObject *parent = nullptr);
    ~MetadataManager();

    bool initializeDatabase();
    bool addMetadata(const FileMetadata &metadata);
    QList<FileMetadata> getAllMetadata();
    QList<FileMetadata> searchMetadata(const QString &query);
    bool updateMetadata(const FileMetadata &metadata);
    bool deleteMetadata(const QString &encryptedFileName);

private:
    QSqlDatabase db;
    bool createTable();
};

#endif // METADATAMANAGER_H
