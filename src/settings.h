/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE_SETTINGS_H
#define FILE_SETTINGS_H

#include <QString>
#include <QStringList>
#include "ini.h"


enum ConnectionMode
{
    MODE_LOCAL = 0,  //!< Local program
    MODE_COREDUMP,   //!< Core dump file
    MODE_TCP         //!< TCP/IP connection to a gdbserver
    
};

class SettingsBreakpoint
{
public:
    
    QString filename;
    int lineNo;
};


class Settings
{
    public:
        Settings();

        void load();
        void save();
        void loadDefaultsGui();
        void loadDefaultsAdvanced();
        
        static QStringList getDefaultCxxKeywordList();
        static QStringList getDefaultCppKeywordList();
        static QStringList getDefaultBasicKeywordList();

        QString getProgramPath();

        int getTabIndentCount() const { return m_tabIndentCount; };
        
    private:
        void loadProjectConfig();
        void loadGlobalConfig();

        void saveProjectConfig();
        void saveGlobalConfig();
        
    public:
        QStringList m_argumentList;
        ConnectionMode m_connectionMode;
        int m_tcpPort;
        QString m_tcpHost;
        QString m_tcpProgram;
        QStringList m_initCommands;
        QString m_gdbPath;
        QString m_lastProgram;
        QString m_coreDumpFile;
        QString m_coreDumpProgram;
        bool m_download;

        QString m_fontFamily;
        int m_fontSize;
        QString m_memoryFontFamily;
        int m_memoryFontSize;
        QString m_outputFontFamily;
        int m_outputFontSize;
        QString m_gdbOutputFontFamily;
        int m_gdbOutputFontSize;

        QStringList m_sourceIgnoreDirs;

        bool m_reloadBreakpoints;
        QString m_initialBreakpoint;
        
        QList<SettingsBreakpoint> m_breakpoints;

        QColor m_clrBackground;
        QColor m_clrComment;
        QColor m_clrString;
        QColor m_clrIncString;
        QColor m_clrKeyword;
        QColor m_clrCppKeyword;
        QColor m_clrCurrentLine;
        QColor m_clrNumber;
        QColor m_clrForeground;
       
        QByteArray m_gui_mainwindowState;
        QByteArray m_gui_mainwindowGeometry;
        QByteArray m_gui_splitter1State;
        QByteArray m_gui_splitter2State;
        QByteArray m_gui_splitter3State;
        QByteArray m_gui_splitter4State;

        bool m_viewWindowStack;
        bool m_viewWindowThreads;
        bool m_viewWindowBreakpoints;
        bool m_viewWindowWatch;
        bool m_viewWindowAutoVariables;
        bool m_viewWindowTargetOutput;
        bool m_viewWindowGdbOutput;
        bool m_viewWindowFileBrowser;

        bool m_tagSortByName;
        bool m_tagShowLineNumbers;

        bool m_enableDebugLog;
        QString m_guiStyleName; // The GUI style to use (Eg: "cleanlooks").

        int m_tabIndentCount;
};


 
#endif // FILE_SETTINGS_H

