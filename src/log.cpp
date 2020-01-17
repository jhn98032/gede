/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "log.h"

#include <QDebug>
#include <QTime>
#include <QMutex>
#include <QMutexLocker>
#include <QMessageBox>

namespace gedelog
{
struct LogEntry
{
public:
    typedef enum { TYPE_WARN, TYPE_INFO, TYPE_ERROR} Type;

    LogEntry(Type t, QString msg) : m_type(t), m_text(msg) {};
    virtual ~LogEntry() {};
    

    Type m_type;
    QString m_text;
    
};
}

using namespace gedelog;


static QMutex g_mutex;
static ILogger *g_logger = NULL;
static QList<gedelog::LogEntry> m_pendingEntries;


#ifdef WIN32
#define YELLOW_CODE ""
#define GREEN_CODE  ""
#define RED_CODE    ""
#define NO_CODE     ""
#else
#define YELLOW_CODE "\033[1;33m"
#define GREEN_CODE  "\033[1;32m"
#define RED_CODE    "\033[1;31m"
#define NO_CODE     "\033[1;0m"
#endif



void loggerRegister(ILogger *logger)
{
    QMutexLocker locker(&g_mutex);
    g_logger = logger;

    while(!m_pendingEntries.isEmpty())
    {
        LogEntry entry = m_pendingEntries.takeFirst();
        if(entry.m_type == LogEntry::TYPE_INFO)
            g_logger->ILogger_onInfoMsg(entry.m_text);
        else if(entry.m_type == LogEntry::TYPE_WARN)
            g_logger->ILogger_onWarnMsg(entry.m_text);
        else
            g_logger->ILogger_onErrorMsg(entry.m_text);
    }
    
}

void loggerUnregister(ILogger *logger)
{
    QMutexLocker locker(&g_mutex);
    Q_UNUSED(logger);
    
    g_logger = NULL;
}


void debugMsg_(const char *filename, int lineNo, const char *fmt, ...)
{
    va_list ap;
    char buffer[1024];
    QTime curTime = QTime::currentTime();

    QMutexLocker locker(&g_mutex);

    va_start(ap, fmt);
    
    vsnprintf(buffer, sizeof(buffer), fmt, ap);


    va_end(ap);

    printf("%2d.%03d| DEBUG | %s:%d| %s\n",
        curTime.second()%100, curTime.msec(),
        filename, lineNo, buffer);
}


void errorMsg(const char *fmt, ...)
{
    va_list ap;
    char buffer[1024];
    QTime curTime = QTime::currentTime();

    QMutexLocker locker(&g_mutex);

    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);


    va_end(ap);


    if(g_logger)
        g_logger->ILogger_onErrorMsg(buffer);
    else
    {
        m_pendingEntries.append(LogEntry(LogEntry::TYPE_ERROR, buffer));

        printf(RED_CODE "%2d.%03d| ERROR | %s" NO_CODE "\n",
            curTime.second()%100, curTime.msec(),
            buffer);

    }
    QMessageBox::critical(NULL, QString("Gede - Error"), QString(buffer));

}


void warnMsg(const char *fmt, ...)
{
    va_list ap;
    char buffer[1024];
    QTime curTime = QTime::currentTime();

    QMutexLocker locker(&g_mutex);

    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);


    va_end(ap);


    if(g_logger)
        g_logger->ILogger_onWarnMsg(buffer);
    else
    {
        m_pendingEntries.append(LogEntry(LogEntry::TYPE_WARN, buffer));

        printf(YELLOW_CODE "%2d.%03d| WARN  | %s" NO_CODE "\n",
            curTime.second()%100, curTime.msec(),
            buffer);
    }
}



void infoMsg(const char *fmt, ...)
{
    va_list ap;
    char buffer[1024];
    QTime curTime = QTime::currentTime();

    QMutexLocker locker(&g_mutex);

    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);


    va_end(ap);

    if(g_logger)
        g_logger->ILogger_onInfoMsg(buffer);
    else
    {
        m_pendingEntries.append(LogEntry(LogEntry::TYPE_INFO, buffer));

        printf("%2d.%03d| INFO  | %s\n",
            curTime.second()%100, curTime.msec(),
            buffer);
    }
}


