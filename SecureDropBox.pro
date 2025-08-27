# Qt Project File

TEMPLATE = app
QT += core gui widgets sql

# Use C++17 standard
CONFIG += c++17

TARGET = SecureDropBox

# Directories
INCLUDEPATH += include/ src/

# Headers
HEADERS += \
    include/CompressionManager.h \
    include/EncryptionManager.h \
    include/ExclusionManager.h \
    include/FileMonitor.h \
    include/FileProcessor.h \
    include/FileSystemManager.h \
    include/LoadingScreen.h \
    include/Logger.h \
    include/MetadataManager.h \
    include/SettingsDialog.h \
    src/Utils.h

# Sources
SOURCES += \
    src/main.cpp \
    src/CompressionManager.cpp \
    src/EncryptionManager.cpp \
    src/ExclusionManager.cpp \
    src/FileMonitor.cpp \
    src/FileProcessor.cpp \
    src/FileSystemManager.cpp \
    src/LoadingScreen.cpp \
    src/Logger.cpp \
    src/MetadataManager.cpp \
    src/SettingsDialog.cpp \
    src/Utils.cpp

# Libraries
LIBS += -lsodium -lz

# Build directories
MOC_DIR = build/moc
OBJECTS_DIR = build/obj
RCC_DIR = build/rcc
UI_DIR = build/ui

linux-g++ {
    message("Configuring for Linux with GCC")
}
