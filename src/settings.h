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
    MODE_TCP,        //!< TCP/IP connection to a gdbserver
    MODE_PID         //!< Connect to a running process
    
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
        static QStringList getDefaultRustKeywordList();

        QString getProgramPath();

        int getTabIndentCount() const { return m_tabIndentCount; };
        
    private:
        void loadProjectConfig();
        void loadGlobalConfig();

        void saveProjectConfig();
        void saveGlobalConfig();
        
    public:
        bool m_globalProjConfig;

        QStringList m_argumentList;
        ConnectionMode m_connectionMode;
        int m_tcpPort;
        QString m_tcpHost;
        QStringList m_initCommands;
        QString m_gdbPath;
        QString m_lastProgram;
        QString m_coreDumpFile;
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

        bool m_showLineNo;

        bool m_enableDebugLog;
        QString m_guiStyleName; // The GUI style to use (Eg: "cleanlooks").

        int m_progConScrollback; // Number of lines of console output to save

        QColor m_progConColorFg;
        QColor m_progConColorBg;
        QColor m_progConColorCursor;
        QColor m_progConColorNorm[8];
        QColor m_progConColorBright[8];

        int m_progConBackspaceKey;
        int m_progConDelKey;
        
        int m_runningPid;
        
        int m_tabIndentCount;
        typedef enum { HOLLOW_RECT = 0, FILLED_RECT } CurrentLineStyle;
        CurrentLineStyle m_currentLineStyle;

        int m_maxTabs; //!< Max number of opened tabs at the same time

        int m_variablePopupDelay; //!< Number of milliseconds before the variables value should be displayed in a popup.
};


 
#endif // FILE_SETTINGS_H

