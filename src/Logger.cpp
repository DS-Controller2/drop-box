#include "Logger.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

QFile *Logger::logFile = nullptr;
QTextStream *Logger::logStream = nullptr;
QMutex Logger::mutex;

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Logger::log(type, context, msg);
}

void Logger::initialize()
{
    QMutexLocker locker(&mutex);
    if (logFile) {
        return; // Already initialized
    }

    QString logDirectory = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (logDirectory.isEmpty()) {
        logDirectory = QCoreApplication::applicationDirPath();
    }
    QDir().mkpath(logDirectory);

    QString logFilePath = logDirectory + QDir::separator() + "dropbox.log";
    logFile = new QFile(logFilePath);
    if (!logFile->open(QFile::WriteOnly | QFile::Append)) {
        // Fallback to stderr if log file cannot be opened
        fprintf(stderr, "Warning: Could not open log file %s: %s\n", qPrintable(logFilePath), qPrintable(logFile->errorString()));
        delete logFile;
        logFile = nullptr;
        return;
    }
    logStream = new QTextStream(logFile);

    qInstallMessageHandler(customMessageHandler);
    qDebug() << "Logger initialized. Log file:" << logFilePath;
}

void Logger::log(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutexLocker locker(&mutex);
    if (!logStream) {
        // If logger not initialized or failed, print to stderr
        fprintf(stderr, "%s\n", qPrintable(msg));
        return;
    }

    QString logMessage;
    QTextStream(&logMessage) << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");

    switch (type) {
        case QtDebugMsg:
            logMessage += "DEBUG: ";
            break;
        case QtInfoMsg:
            logMessage += "INFO: ";
            break;
        case QtWarningMsg:
            logMessage += "WARNING: ";
            break;
        case QtCriticalMsg:
            logMessage += "CRITICAL: ";
            break;
        case QtFatalMsg:
            logMessage += "FATAL: ";
            break;
    }

    logMessage += msg;

    if (context.file && context.line) {
        logMessage += QString (" (%1:%2)").arg(context.file).arg(context.line);
    }

    *logStream << logMessage << Qt::endl;
    logStream->flush();

    // Also print to console for immediate feedback during development
    fprintf(stderr, "%s\n", qPrintable(logMessage));

    if (type == QtFatalMsg) {
        abort();
    }
}
