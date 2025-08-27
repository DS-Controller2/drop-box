#include "SettingsDialog.h"
#include <QFileDialog>
#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Settings");
    setFixedSize(400, 300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    setupPerformanceTab();
    setupSortingTab();
    setupExclusionsTab(); // Add the new exclusions tab

    // Add buttons at the bottom
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("OK", this);
    QPushButton *cancelButton = new QPushButton("Cancel", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, this, &SettingsDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &SettingsDialog::reject);
}

void SettingsDialog::accept()
{
    QSettings settings("MyDropBox", "DropBoxApp");
    settings.setValue("performance/enableBackgroundProcessing", enableBackgroundProcessing->isChecked());
    settings.setValue("performance/maxThreads", maxThreadsSpinBox->value());
    settings.setValue("sorting/enableAutoSorting", enableAutoSorting->isChecked());
    settings.setValue("sorting/sortByFileType", sortByFileType->isChecked());
    settings.setValue("sorting/sortByDate", sortByDate->isChecked());

    QDialog::accept();
}

void SettingsDialog::setupPerformanceTab()
{
    QSettings settings("MyDropBox", "DropBoxApp");

    performanceTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(performanceTab);

    QGroupBox *groupBox = new QGroupBox("Performance Options", performanceTab);
    QVBoxLayout *groupBoxLayout = new QVBoxLayout(groupBox);

    enableBackgroundProcessing = new QCheckBox("Enable Background Processing", groupBox);
    enableBackgroundProcessing->setChecked(settings.value("performance/enableBackgroundProcessing", false).toBool());
    groupBoxLayout->addWidget(enableBackgroundProcessing);

    QHBoxLayout *threadsLayout = new QHBoxLayout();
    threadsLayout->addWidget(new QLabel("Max Threads:", groupBox));
    maxThreadsSpinBox = new QSpinBox(groupBox);
    maxThreadsSpinBox->setRange(1, 8);
    maxThreadsSpinBox->setValue(settings.value("performance/maxThreads", 4).toInt());
    threadsLayout->addWidget(maxThreadsSpinBox);
    threadsLayout->addStretch();
    groupBoxLayout->addLayout(threadsLayout);

    groupBox->setLayout(groupBoxLayout);
    layout->addWidget(groupBox);
    layout->addStretch();

    tabWidget->addTab(performanceTab, "Performance");
}

void SettingsDialog::setupSortingTab()
{
    QSettings settings("MyDropBox", "DropBoxApp");

    sortingTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(sortingTab);

    QGroupBox *groupBox = new QGroupBox("Sorting Options", sortingTab);
    QVBoxLayout *groupBoxLayout = new QVBoxLayout(groupBox);

    enableAutoSorting = new QCheckBox("Enable Automatic Sorting", groupBox);
    enableAutoSorting->setChecked(settings.value("sorting/enableAutoSorting", false).toBool());
    groupBoxLayout->addWidget(enableAutoSorting);

    sortByFileType = new QCheckBox("Sort by File Type", groupBox);
    sortByFileType->setChecked(settings.value("sorting/sortByFileType", false).toBool());
    groupBoxLayout->addWidget(sortByFileType);

    sortByDate = new QCheckBox("Sort by Date", groupBox);
    sortByDate->setChecked(settings.value("sorting/sortByDate", false).toBool());
    groupBoxLayout->addWidget(sortByDate);

    groupBox->setLayout(groupBoxLayout);
    layout->addWidget(groupBox);
    layout->addStretch();

    tabWidget->addTab(sortingTab, "Sorting");
}

void SettingsDialog::setupExclusionsTab()
{
    exclusionsTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(exclusionsTab);

    QGroupBox *groupBox = new QGroupBox("Excluded Folders", exclusionsTab);
    QVBoxLayout *groupBoxLayout = new QVBoxLayout(groupBox);

    excludedFoldersList = new QListWidget(groupBox);
    groupBoxLayout->addWidget(excludedFoldersList);

    // Populate the list with existing excluded folders
    excludedFoldersList->addItems(exclusionManager.getExcludedFolders());

    QHBoxLayout *addRemoveLayout = new QHBoxLayout();
    newExcludedFolderInput = new QLineEdit(groupBox);
    newExcludedFolderInput->setPlaceholderText("Path to exclude");
    addRemoveLayout->addWidget(newExcludedFolderInput);

    QPushButton *browseExcludedFolderButton = new QPushButton("Browse", groupBox);
    addRemoveLayout->addWidget(browseExcludedFolderButton);

    addExcludedFolderButton = new QPushButton("Add", groupBox);
    addRemoveLayout->addWidget(addExcludedFolderButton);

    removeExcludedFolderButton = new QPushButton("Remove Selected", groupBox);
    addRemoveLayout->addWidget(removeExcludedFolderButton);

    groupBoxLayout->addLayout(addRemoveLayout);

    groupBox->setLayout(groupBoxLayout);
    layout->addWidget(groupBox);
    layout->addStretch();

    tabWidget->addTab(exclusionsTab, "Exclusions");

    // Connect signals
    connect(browseExcludedFolderButton, &QPushButton::clicked, [=]() {
        QString directory = QFileDialog::getExistingDirectory(this, "Select Folder to Exclude");
        if (!directory.isEmpty()) {
            newExcludedFolderInput->setText(directory);
        }
    });

    connect(addExcludedFolderButton, &QPushButton::clicked, [=]() {
        QString folderPath = newExcludedFolderInput->text();
        if (!folderPath.isEmpty()) {
            exclusionManager.addExcludedFolder(folderPath);
            excludedFoldersList->clear();
            excludedFoldersList->addItems(exclusionManager.getExcludedFolders());
            newExcludedFolderInput->clear();
        }
    });

    connect(removeExcludedFolderButton, &QPushButton::clicked, [=]() {
        QListWidgetItem *selectedItem = excludedFoldersList->currentItem();
        if (selectedItem) {
            exclusionManager.removeExcludedFolder(selectedItem->text());
            delete selectedItem; // Remove from QListWidget
        }
    });
}
