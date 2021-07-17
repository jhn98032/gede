/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "util.h"

#include <assert.h>
#include <QString>
#include <stdio.h>
#include <QFile>
#include <QProcess>
#include <QMap>
#include <QDir>
#include <QStringList>



/**
 * @brief Divides a path into a filename and a path.
 *
 * Example: dividePath("/dir/filename.ext") => "/dir", "filename.ext".
 */
void dividePath(QString fullPath, QString *filename, QString *folderPath)
{
    int divPos = fullPath.lastIndexOf('/');
    if(divPos> 0)
    {
        if(filename)
            *filename = fullPath.mid(divPos+1);
        if(folderPath)
            *folderPath = fullPath.left(divPos);
    }
    else
    {
        if(filename)
            *filename = fullPath;
    }
}

/**
 * @brief Returns the filename of a path.
 *
 * Example: getFilenamePart("/dir/filename.ext") => "filename.ext".
 */
QString getFilenamePart(QString fullPath)
{
    QString filename;
    dividePath(fullPath, &filename, NULL);
    return filename;
}


/**
 * @brief Returns the extension of a file.
 * @return The extension including the dot (Eg: ".txt").
 */
QString getExtensionPart(QString filename)
{
    int idx = filename.lastIndexOf('.');
    if(idx == -1)
        return QString("");
    return filename.mid(idx);
}



long long stringToLongLong(const char* str)
{
    unsigned long long num = 0;
    QString str2 = str;
    str2.replace('_', "");
    num = str2.toLongLong(0,0);

    return num;
}


QString longLongToHexString(long long num)
{
    QString newStr;
    QString str;
    str.sprintf("%llx", num);
    if(num != 0)
    {
        while(str.length()%4 != 0)
            str = "0" + str;

        
        for(int i = str.length()-1;i >= 0;i--)
        {
            newStr += str[str.length()-i-1];
            if(i%4 == 0 && i != 0)
                newStr += "_";
        }
        str = newStr;
    }
    return "0x" + str;
}


static QString priv_simplifySubPath(QString path)
{
    QString out;

    if(path.startsWith('/'))
        return simplifyPath(path.mid(1));
    if(path.startsWith("./"))
        return simplifyPath(path.mid(2));

    QString first;
    QString rest;

    int piv = path.indexOf('/');
    if(piv == -1)
        return path;
    else
    {
        first = path.left(piv);
        rest = path.mid(piv+1);
        rest = priv_simplifySubPath(rest);
        if(rest.isEmpty())
            path = first;
        else
            path = first + "/" + rest;
    }
    return path;
}


/**
 * @brief Simplifies a path by removing unnecessary seperators.
 *
 * Eg: simplifyPath("./a///path/") => "./a/path".
 */
QString simplifyPath(QString path)
{
    QString out;
    if(path.startsWith("./"))
        out = "./" + priv_simplifySubPath(path.mid(2));
    else if(path.startsWith('/'))
        out = '/' + priv_simplifySubPath(path.mid(1));
    else
        out = priv_simplifySubPath(path);
    return out;
}

/**
*  @brief Converts a hex two byte string to a unsigned char.
*/
unsigned char hexStringToU8(const char *str)
{
    unsigned char d = 0;
    char c1 = str[0];
    char c2 = str[1];

    // Upper byte
    if('0' <= c1 && c1 <= '9')
        d =  c1-'0';
    else if('a' <= c1 && c1 <= 'f')
        d =  0xa + (c1-'a');
    else if('A' <= c1 && c1 <= 'F')
        d =  0xa + (c1-'A');
    else // invalid character
    {
        assert(0);
        return 0;
    }
    d = d<<4;

    // Lower byte
    if('0' <= c2 && c2 <= '9')
        d +=  c2-'0';
    else if('a' <= c2 && c2 <= 'f')
        d +=  0xa + (c2-'a');
    else if('A' <= c2 && c2 <= 'F')
        d +=  0xa + (c2-'A');
    else // invalid character?
    {
        assert(0);
        d = d>>4;
    }

    return d;
}

long long stringToLongLong(QString str)
{
    return stringToLongLong(stringToCStr(str));
}


/**
  * @brief Tries to detect the distro the system is running on.
  */
void detectDistro(DistroType *type, QString *distroDesc)
{
    QString machine = "";
    QString distroName = "";
    
    distroName = "Unknown OS";
    if(type)
        *type = DISTRO_UNKNOWN;

    // Check for Debian
    QFile file1("/etc/debian_version");
    if(file1.open(QIODevice::ReadOnly))
    {
        if(type)
            *type = DISTRO_DEBIAN;

        QString version = file1.readLine().trimmed();
        distroName = "Debian " + version;
    }

    // Parse lsb-release file
    QFile file2("/etc/lsb-release");
    if(file2.open(QIODevice::ReadOnly))
    {
        QMap<QString,QString> fields;
        // Parse ini-like structure
        while (!file2.atEnd())
        {
            QString line = file2.readLine().trimmed();
            QStringList tokens = line.split("=");
            if(tokens.size() == 2)
            {
                QString name = tokens[0].trimmed();
                QString data = tokens[1].trimmed();
                if(data.startsWith('"'))
                    data = data.mid(1, data.length()-2);
                fields[name] = data;


            }
            
        }
        
        if(fields.contains("DISTRIB_ID"))
        {
            QString distribId = fields["DISTRIB_ID"];
            if(type)
            {
                if(distribId == "Ubuntu")
                    *type = DISTRO_UBUNTU;
            }
            distroName = distribId;
            if(fields.contains("DISTRIB_RELEASE"))
                distroName += " " + fields["DISTRIB_RELEASE"];
        }
        if(fields.contains("DISTRIB_DESCRIPTION"))
            distroName = fields["DISTRIB_DESCRIPTION"];
        
    }


    // Detect x64/x86 
    QString versionStr;
    QProcess process;
    process.start("uname", 
        QStringList("-m"),
        QIODevice::ReadOnly | QIODevice::Text);
    if(!process.waitForFinished(2000))
    {
    }
    else
    {
        machine = process.readAllStandardOutput().trimmed();
    }


    if(distroDesc)
    {
        *distroDesc = distroName;
        if(!machine.isEmpty())
            *distroDesc += " " + machine;
    }
}


/**
 * @brief Returns a string describing an address (eg: "0x3000_1234").
 */
QString addrToString(quint64 addr)
{
    QString valueText;
    QString text;
    text.sprintf("%llx", (unsigned long long) addr);

    // Prefix the string with suitable number of zeroes
    while(text.length()%4 != 0 && text.length() > 4)
        text = "0" + text;
    if(text.length()%2 != 0)
        text = "0" + text;

    for(int i = 0;i < text.length();i++)
    {
        valueText = valueText + text[i];
        if(i%4 == 3 && i+1 != text.length())
            valueText += "_";
    }
    valueText = "0x" + valueText;
    return valueText;
}

/**
 * @brief Checks if an executable exist in the PATH (or in current directory)
 * @param name   Name of executable (Eg: "cp").
 * @return true if the exe exists
 */
bool exeExists(QString name, bool checkCurrentDir)
{
    QStringList pathList;

    //
    const char *pathStr = getenv("PATH");
    if(pathStr)
        pathList = QString(pathStr).split(":");
    
    if(checkCurrentDir)
    {
        pathList.append("./");
    }
    
    for(int i = 0;i < pathList.size();i++)
    {
        QString exePath = pathList[i];
        
        QDir dir(exePath);
        dir.setFilter(QDir::Files | QDir::Executable);
        if(dir.exists(name))
            return true;
    }
    return false;
}



QByteArray fileToContent(QString filename)
{
    QByteArray cnt;
    QFile f(filename);
    if(!f.open(QIODevice::ReadOnly))
    {
    }
    else
    {
        cnt = f.readAll();
    }
    return cnt;
}

#ifdef NEVER
void testFuncs()
{
    printf("%12x -> '%s'\n", 0, stringToCStr(longLongToHexString(0)));
    printf("%12x -> '%s'\n", 0x1, stringToCStr(longLongToHexString(0x1)));
    printf("%12x -> '%s'\n", 0x123, stringToCStr(longLongToHexString(0x123)));
    printf("%12x -> '%s'\n", 0x1234, stringToCStr(longLongToHexString(0x1234)));
    printf("%12x -> '%s'\n", 0x1234567, stringToCStr(longLongToHexString(0x1234567)));
    printf("%12llx -> '%s'\n", 0x12345678ULL, stringToCStr(longLongToHexString(0x12345678ULL)));
    printf("%12llx -> '%s'\n", 0x123456789abcULL, stringToCStr(longLongToHexString(0x123456789abcULL)));
}
#endif

