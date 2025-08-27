#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

class Logger
{
public:
    static void initialize();
    static void log(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    static QFile *logFile;
    static QTextStream *logStream;
    static QMutex mutex;
};

#endif // LOGGER_H
