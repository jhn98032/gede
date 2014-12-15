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
        QStringList m_argumentList;
        ConnectionMode m_connectionMode;
        int m_tcpPort;
        QString m_tcpHost;
        QString m_tcpProgram;
        QStringList m_initCommands;
        QString m_gdbPath;
        QString m_lastProgram;
        QString m_fontFamily;
        int m_fontSize;
    

};

#endif // FILE_SETTINGS_H

