/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__UTIL_H
#define FILE__UTIL_H

#include <QString>
#include <QByteArray>

#define MIN(a,b) ((a)<(b))
#define MAX(a,b) ((a)>(b))

//#define stringToCStr(str) str.toAscii().constData()
#define stringToCStr(str) qPrintable(str)


QString getFilenamePart(QString fullPath);
void dividePath(QString fullPath, QString *filename, QString *folderPath);
QString getExtensionPart(QString filename);

quint8 hexStringToU8(const char *str);
long long stringToLongLong(const char* str);
long long stringToLongLong(QString str);
QString longLongToHexString(long long num);

QString simplifyPath(QString path);

typedef enum{ DISTRO_DEBIAN, DISTRO_UBUNTU, DISTRO_UNKNOWN} DistroType;
void detectDistro(DistroType *type, QString *distroDesc);

QString addrToString(quint64 addr);

bool exeExists(QString name, bool checkCurrentDir = false);

QByteArray fileToContent(QString filename);

QStringList splitString(QString str, char separator = ' ');
QString joingStringList(QStringList arguments, char separator = ' ');



#endif // FILE__UTIL_H

