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


static QMutex g_mutex;

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

    printf(RED_CODE "%2d.%03d| ERROR | %s" NO_CODE "\n",
        curTime.second()%100, curTime.msec(),
        buffer);
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

    printf(YELLOW_CODE "%2d.%03d| WARN  | %s" NO_CODE "\n",
        curTime.second()%100, curTime.msec(),
        buffer);
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

    printf("%2d.%03d| INFO  | %s\n",
            curTime.second()%100, curTime.msec(),
            buffer);
}


