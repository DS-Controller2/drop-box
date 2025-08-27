#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>

#include "ExclusionManager.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    void accept() override;

private:
    QTabWidget *tabWidget;

    // Performance Tab
    QWidget *performanceTab;
    QCheckBox *enableBackgroundProcessing;
    QSpinBox *maxThreadsSpinBox;

    // Sorting Tab
    QWidget *sortingTab;
    QCheckBox *enableAutoSorting;
    QCheckBox *sortByFileType;
    QCheckBox *sortByDate;

    // Exclusions Tab
    QWidget *exclusionsTab;
    QListWidget *excludedFoldersList;
    QLineEdit *newExcludedFolderInput;
    QPushButton *addExcludedFolderButton;
    QPushButton *removeExcludedFolderButton;

    ExclusionManager exclusionManager;

    void setupPerformanceTab();
    void setupSortingTab();
    void setupExclusionsTab();
};

#endif // SETTINGSDIALOG_H