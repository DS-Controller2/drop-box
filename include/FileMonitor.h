#ifndef FILEMONITOR_H
#define FILEMONITOR_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QString>

class FileMonitor : public QObject
{
    Q_OBJECT

public:
    explicit FileMonitor(QObject *parent = nullptr);

    void startMonitoring(const QString &path);
    void stopMonitoring();

signals:
    void fileAdded(const QString &filePath);
    void fileChanged(const QString &filePath);

private slots:
    void directoryChanged(const QString &path);
    void onFileChanged(const QString &path);

private:
    QFileSystemWatcher *watcher;
    QString monitoredPath;
};

#endif // FILEMONITOR_H
