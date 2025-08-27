#include "MetadataManager.h"
#include <QDebug>
#include <QSqlError>
#include <QCoreApplication>
#include <QDir>

MetadataManager::MetadataManager(QObject *parent) : QObject(parent)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QCoreApplication::applicationDirPath() + QDir::separator() + "dropbox_metadata.db";
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qWarning() << "Error: connection with database failed" << db.lastError().text();
    } else {
        qDebug() << "Database connection established.";
        createTable();
    }
}

MetadataManager::~MetadataManager()
{
    if (db.isOpen()) {
        db.close();
        qDebug() << "Database connection closed.";
    }
}

bool MetadataManager::initializeDatabase()
{
    return db.isOpen();
}

bool MetadataManager::createTable()
{
    QSqlQuery query(db);
    QString createTableSql = "CREATE TABLE IF NOT EXISTS files_metadata ("
                             "originalFileName TEXT NOT NULL," 
                             "encryptedFileName TEXT PRIMARY KEY NOT NULL," 
                             "fileType TEXT," 
                             "fileSize INTEGER," 
                             "encryptionKeyId TEXT," 
                             "compressionRatio REAL," 
                             "processingDate TEXT," 
                             "originalPath TEXT," 
                             "currentStoredPath TEXT"
                             ");";

    if (!query.exec(createTableSql)) {
        qWarning() << "Error creating table:" << query.lastError().text();
        return false;
    }
    qDebug() << "Table 'files_metadata' created or already exists.";
    return true;
}

bool MetadataManager::addMetadata(const FileMetadata &metadata)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO files_metadata (originalFileName, encryptedFileName, fileType, fileSize, encryptionKeyId, compressionRatio, processingDate, originalPath, currentStoredPath) "
                  "VALUES (:originalFileName, :encryptedFileName, :fileType, :fileSize, :encryptionKeyId, :compressionRatio, :processingDate, :originalPath, :currentStoredPath)");
    query.bindValue(":originalFileName", metadata.originalFileName);
    query.bindValue(":encryptedFileName", metadata.encryptedFileName);
    query.bindValue(":fileType", metadata.fileType);
    query.bindValue(":fileSize", metadata.fileSize);
    query.bindValue(":encryptionKeyId", metadata.encryptionKeyId);
    query.bindValue(":compressionRatio", metadata.compressionRatio);
    query.bindValue(":processingDate", metadata.processingDate.toString(Qt::ISODate));
    query.bindValue(":originalPath", metadata.originalPath);
    query.bindValue(":currentStoredPath", metadata.currentStoredPath);

    if (!query.exec()) {
        qWarning() << "Error adding metadata:" << query.lastError().text();
        return false;
    }
    qDebug() << "Metadata added for:" << metadata.originalFileName;
    return true;
}

QList<FileMetadata> MetadataManager::getAllMetadata()
{
    QList<FileMetadata> metadataList;
    QSqlQuery query("SELECT * FROM files_metadata", db);

    if (!query.exec()) {
        qWarning() << "Error getting all metadata:" << query.lastError().text();
        return metadataList;
    }

    while (query.next()) {
        FileMetadata metadata;
        metadata.originalFileName = query.value("originalFileName").toString();
        metadata.encryptedFileName = query.value("encryptedFileName").toString();
        metadata.fileType = query.value("fileType").toString();
        metadata.fileSize = query.value("fileSize").toLongLong();
        metadata.encryptionKeyId = query.value("encryptionKeyId").toString();
        metadata.compressionRatio = query.value("compressionRatio").toDouble();
        metadata.processingDate = QDateTime::fromString(query.value("processingDate").toString(), Qt::ISODate);
        metadata.originalPath = query.value("originalPath").toString();
        metadata.currentStoredPath = query.value("currentStoredPath").toString();
        metadataList.append(metadata);
    }
    return metadataList;
}

QList<FileMetadata> MetadataManager::searchMetadata(const QString &queryText)
{
    QList<FileMetadata> metadataList;
    QSqlQuery query(db);
    query.prepare("SELECT * FROM files_metadata WHERE originalFileName LIKE :query OR encryptedFileName LIKE :query OR fileType LIKE :query");
    query.bindValue(":query", "%" + queryText + "%");

    if (!query.exec()) {
        qWarning() << "Error searching metadata:" << query.lastError().text();
        return metadataList;
    }

    while (query.next()) {
        FileMetadata metadata;
        metadata.originalFileName = query.value("originalFileName").toString();
        metadata.encryptedFileName = query.value("encryptedFileName").toString();
        metadata.fileType = query.value("fileType").toString();
        metadata.fileSize = query.value("fileSize").toLongLong();
        metadata.encryptionKeyId = query.value("encryptionKeyId").toString();
        metadata.compressionRatio = query.value("compressionRatio").toDouble();
        metadata.processingDate = QDateTime::fromString(query.value("processingDate").toString(), Qt::ISODate);
        metadata.originalPath = query.value("originalPath").toString();
        metadata.currentStoredPath = query.value("currentStoredPath").toString();
        metadataList.append(metadata);
    }
    return metadataList;
}

bool MetadataManager::updateMetadata(const FileMetadata &metadata)
{
    QSqlQuery query(db);
    query.prepare("UPDATE files_metadata SET originalFileName = :originalFileName, fileType = :fileType, fileSize = :fileSize, encryptionKeyId = :encryptionKeyId, compressionRatio = :compressionRatio, processingDate = :processingDate, originalPath = :originalPath, currentStoredPath = :currentStoredPath WHERE encryptedFileName = :encryptedFileName");
    query.bindValue(":originalFileName", metadata.originalFileName);
    query.bindValue(":fileType", metadata.fileType);
    query.bindValue(":fileSize", metadata.fileSize);
    query.bindValue(":encryptionKeyId", metadata.encryptionKeyId);
    query.bindValue(":compressionRatio", metadata.compressionRatio);
    query.bindValue(":processingDate", metadata.processingDate.toString(Qt::ISODate));
    query.bindValue(":originalPath", metadata.originalPath);
    query.bindValue(":currentStoredPath", metadata.currentStoredPath);
    query.bindValue(":encryptedFileName", metadata.encryptedFileName); // WHERE clause

    if (!query.exec()) {
        qWarning() << "Error updating metadata:" << query.lastError().text();
        return false;
    }
    qDebug() << "Metadata updated for:" << metadata.originalFileName;
    return true;
}

bool MetadataManager::deleteMetadata(const QString &encryptedFileName)
{
    QSqlQuery query(db);
    query.prepare("DELETE FROM files_metadata WHERE encryptedFileName = :encryptedFileName");
    query.bindValue(":encryptedFileName", encryptedFileName);

    if (!query.exec()) {
        qWarning() << "Error deleting metadata:" << query.lastError().text();
        return false;
    }
    qDebug() << "Metadata deleted for:" << encryptedFileName;
    return true;
}
