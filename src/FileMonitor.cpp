#include "FileMonitor.h"
#include <QDebug>
#include <QDir>

FileMonitor::FileMonitor(QObject *parent) : QObject(parent)
{
    watcher = new QFileSystemWatcher(this);
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, &FileMonitor::directoryChanged);
    connect(watcher, &QFileSystemWatcher::fileChanged, this, &FileMonitor::onFileChanged);
}

void FileMonitor::startMonitoring(const QString &path)
{
    if (watcher->directories().contains(path)) {
        qDebug() << "Already monitoring:" << path;
        return;
    }

    if (!QDir(path).exists()) {
        qWarning() << "Directory does not exist:" << path;
        return;
    }

    watcher->addPath(path);
    monitoredPath = path;
    qDebug() << "Started monitoring:" << path;
}

void FileMonitor::stopMonitoring()
{
    if (!monitoredPath.isEmpty()) {
        watcher->removePath(monitoredPath);
        qDebug() << "Stopped monitoring:" << monitoredPath;
        monitoredPath.clear();
    }
}

void FileMonitor::directoryChanged(const QString &path)
{
    qDebug() << "Directory changed:" << path;
    // Re-add the path to monitor new files within the directory
    // This is a common workaround for QFileSystemWatcher not always detecting new files directly
    watcher->removePath(path);
    watcher->addPath(path);

    // For simplicity, we'll just emit fileAdded for any change in directory for now.
    // A more robust solution would compare directory contents.
    emit fileAdded(path); // This is a simplification, ideally we'd detect *which* file was added
}

void FileMonitor::onFileChanged(const QString &path)
{
    qDebug() << "File changed:" << path;
    emit fileChanged(path);
}
