#ifndef FILE_SETTINGS_H
#define FILE_SETTINGS_H

#include <QString>
#include <QStringList>

#include "core.h"

class Settings
{
    public:
        Settings();

        void load(QString filepath);
        void save(QString filepath);


    public:
        QStringList argumentList;
        ConnectionMode connectionMode;
        int tcpPort;
        QString tcpHost;
        QString tcpProgram;
        QStringList initCommands;
        QString gdbPath;
        QString lastProgram;
        QStringList arguments;



};

#endif // FILE_SETTINGS_H

