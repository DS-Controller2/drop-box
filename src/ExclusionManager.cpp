#include "ExclusionManager.h"

ExclusionManager::ExclusionManager()
{
    loadExcludedFolders();
}

void ExclusionManager::addExcludedFolder(const QString &folderPath)
{
    if (!excludedFolders.contains(folderPath)) {
        excludedFolders.append(folderPath);
        saveExcludedFolders();
    }
}

void ExclusionManager::removeExcludedFolder(const QString &folderPath)
{
    if (excludedFolders.contains(folderPath)) {
        excludedFolders.removeOne(folderPath);
        saveExcludedFolders();
    }
}

QStringList ExclusionManager::getExcludedFolders() const
{
    return excludedFolders;
}

bool ExclusionManager::isFolderExcluded(const QString &folderPath) const
{
    // Check if the folderPath itself is in the excluded list
    if (excludedFolders.contains(folderPath)) {
        return true;
    }

    // Check if any parent of folderPath is in the excluded list
    for (const QString &excluded : excludedFolders) {
        if (folderPath.startsWith(excluded + "/")) {
            return true;
        }
    }
    return false;
}

void ExclusionManager::loadExcludedFolders()
{
    QSettings settings("MyDropBox", "DropBoxApp");
    excludedFolders = settings.value("excludedFolders").toStringList();
}

void ExclusionManager::saveExcludedFolders()
{
    QSettings settings("MyDropBox", "DropBoxApp");
    settings.setValue("excludedFolders", excludedFolders);
}
