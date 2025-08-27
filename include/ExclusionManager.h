#ifndef EXCLUSIONMANAGER_H
#define EXCLUSIONMANAGER_H

#include <QStringList>
#include <QSettings>

class ExclusionManager
{
public:
    ExclusionManager();

    void addExcludedFolder(const QString &folderPath);
    void removeExcludedFolder(const QString &folderPath);
    QStringList getExcludedFolders() const;
    bool isFolderExcluded(const QString &folderPath) const;

private:
    QStringList excludedFolders;
    void loadExcludedFolders();
    void saveExcludedFolders();
};

#endif // EXCLUSIONMANAGER_H
