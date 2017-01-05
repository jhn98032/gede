/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "settings.h"
#include "util.h"
#include "log.h"
#include "ini.h"
#include "config.h"

#include <QDir>



Settings::Settings()
: m_connectionMode(MODE_LOCAL)
    ,m_tcpPort(0)
{
    m_viewWindowStack = true;
    m_viewWindowThreads = true;
    m_viewWindowBreakpoints = true;
    m_viewWindowWatch = true;
    m_viewWindowAutoVariables = true;
    m_viewWindowTargetOutput = true;
    m_viewWindowGdbOutput = true;
    m_viewWindowFileBrowser = true;
    m_enableDebugLog = false;
}

void Settings::loadDefaultsGui()
{
    m_fontFamily = "Monospace";
    m_fontSize = 8;
    m_memoryFontFamily = "Monospace";
    m_memoryFontSize = 8;
    m_outputFontFamily = "Monospace";
    m_outputFontSize = 8;
    m_gdbOutputFontFamily = "Monospace";
    m_gdbOutputFontSize = 8;


    m_clrBackground = Qt::black;
    m_clrComment = Qt::green;
    m_clrString = QColor(0,125, 250);
    m_clrIncString = QColor(0,125, 250);
    m_clrKeyword = Qt::yellow;
    m_clrCppKeyword = QColor(240,110,110);
    m_clrNumber = Qt::magenta;
    m_clrForeground = Qt::white;

    m_tagSortByName = false;
    m_tagShowLineNumbers = true;
}

void Settings::loadDefaultsAdvanced()
{
    m_sourceIgnoreDirs.clear();
    m_sourceIgnoreDirs.append("/build");
    m_sourceIgnoreDirs.append("/usr");
    
}


void Settings::load()
{
    loadProjectConfig();
    loadGlobalConfig();
}
 


void Settings::loadGlobalConfig()
{
    // Load from file
    QString globalConfigFilename = QDir::homePath() + "/"  GLOBAL_CONFIG_DIR + "/" + GLOBAL_CONFIG_FILENAME;
    Ini tmpIni;
    if(tmpIni.appendLoad(globalConfigFilename))
        infoMsg("Failed to load global ini '%s'. File will be created.", stringToCStr(globalConfigFilename));

    loadDefaultsGui();
    loadDefaultsAdvanced();

    m_enableDebugLog = tmpIni.getBool("General/EnableDebugLog", false);
    
    m_fontFamily = tmpIni.getString("Gui/CodeFont", m_fontFamily);
    m_fontSize = tmpIni.getInt("Gui/CodeFontSize", m_fontSize);
    m_memoryFontFamily = tmpIni.getString("Gui/MemoryFont", m_memoryFontFamily);
    m_memoryFontSize = tmpIni.getInt("Gui/MemoryFontSize", m_memoryFontSize);
    m_outputFontFamily = tmpIni.getString("Gui/OutputFont", m_outputFontFamily);
    m_outputFontSize = tmpIni.getInt("Gui/OutputFontSize", m_outputFontSize);
    m_gdbOutputFontFamily = tmpIni.getString("Gui/GdbOutputFont", m_outputFontFamily);
    m_gdbOutputFontSize = tmpIni.getInt("Gui/GdbOutputFontSize", m_outputFontSize);

    m_tagSortByName = tmpIni.getBool("Gui/TagsSortByName", false);
    m_tagShowLineNumbers = tmpIni.getBool("Gui/TagsShowLinenumber", true);
    
    m_sourceIgnoreDirs = tmpIni.getStringList("General/ScannerIgnoreDirs", m_sourceIgnoreDirs);

    tmpIni.getByteArray("GuiState/MainWindowState", &m_gui_mainwindowState);
    tmpIni.getByteArray("GuiState/MainWindowGeometry", &m_gui_mainwindowGeometry);
    tmpIni.getByteArray("GuiState/Splitter1State", &m_gui_splitter1State);
    tmpIni.getByteArray("GuiState/Splitter2State", &m_gui_splitter2State);
    tmpIni.getByteArray("GuiState/Splitter3State", &m_gui_splitter3State);
    tmpIni.getByteArray("GuiState/Splitter4State", &m_gui_splitter4State);

    m_viewWindowStack = tmpIni.getBool("GuiState/EnableWindowStack", m_viewWindowStack);
    m_viewWindowThreads = tmpIni.getBool("GuiState/EnableWindowThreads", m_viewWindowThreads);
    m_viewWindowBreakpoints = tmpIni.getBool("GuiState/EnableWindowBreakpoints", m_viewWindowBreakpoints);
    m_viewWindowWatch = tmpIni.getBool("GuiState/EnableWindowWatch", m_viewWindowWatch);
    m_viewWindowAutoVariables = tmpIni.getBool("GuiState/EnableWindowAuto", m_viewWindowAutoVariables);
    m_viewWindowTargetOutput = tmpIni.getBool("GuiState/EnableWindowTargetOutput", m_viewWindowTargetOutput);
    m_viewWindowGdbOutput = tmpIni.getBool("GuiState/EnableWindowGdbOutput", m_viewWindowGdbOutput);
    m_viewWindowFileBrowser = tmpIni.getBool("GuiState/EnableWindowFileBrowser", m_viewWindowFileBrowser);


}

void Settings::loadProjectConfig()
{
    // Load from file
    QString filepath = PROJECT_CONFIG_FILENAME;
    Ini tmpIni;
    if(tmpIni.appendLoad(filepath))
        infoMsg("Failed to load project ini '%s'. File will be created.", stringToCStr(filepath));


    m_download = tmpIni.getBool("Download", true);
    m_connectionMode = tmpIni.getInt("Mode", MODE_LOCAL) == MODE_LOCAL ? MODE_LOCAL : MODE_TCP;
    m_tcpPort = tmpIni.getInt("TcpPort", 2000);
    m_tcpHost = tmpIni.getString("TcpHost", "localhost");
    m_tcpProgram = tmpIni.getString("TcpProgram", "");
    m_initCommands = tmpIni.getStringList("InitCommands", m_initCommands);
    m_gdbPath = tmpIni.getString("GdpPath", "gdb");
    m_lastProgram = tmpIni.getString("LastProgram", "");
    m_argumentList = tmpIni.getStringList("LastProgramArguments", m_argumentList);

    m_reloadBreakpoints = tmpIni.getBool("ReuseBreakpoints", false);

    m_initialBreakpoint = tmpIni.getString("InitialBreakpoint","main");
    

    //
    QStringList breakpointStringList;
    breakpointStringList = tmpIni.getStringList("Breakpoints", breakpointStringList);
    for(int i = 0;i < breakpointStringList.size();i++)
    {
        QString str = breakpointStringList[i];
        if(str.indexOf(':') != -1)
        {
            SettingsBreakpoint bkptCfg;
            bkptCfg.filename = str.left(str.indexOf(':'));
            bkptCfg.lineNo = str.mid(str.indexOf(':')+1).toInt();
            
            m_breakpoints.push_back(bkptCfg);
        }
    }


}
 

void Settings::save()
{
    saveProjectConfig();
    saveGlobalConfig();
}



void Settings::saveProjectConfig()
{

    QString filepath = PROJECT_CONFIG_FILENAME;
    
    Ini tmpIni;

    tmpIni.appendLoad(filepath);

    //
    tmpIni.setBool("Download", m_download);
    tmpIni.setInt("TcpPort", m_tcpPort);
    tmpIni.setString("TcpHost", m_tcpHost);
    tmpIni.setInt("Mode", (int)m_connectionMode);
    tmpIni.setString("LastProgram", m_lastProgram);
    tmpIni.setString("TcpProgram", m_tcpProgram);
    tmpIni.setStringList("InitCommands", m_initCommands);
    tmpIni.setString("GdpPath", m_gdbPath);
    QStringList tmpArgs;
    tmpArgs = m_argumentList;
    tmpIni.setStringList("LastProgramArguments", tmpArgs);
    
    tmpIni.setBool("ReuseBreakpoints", m_reloadBreakpoints);

    tmpIni.setString("InitialBreakpoint",m_initialBreakpoint);


    
    //
    QStringList breakpointStringList;
    for(int i = 0;i < m_breakpoints.size();i++)
    {
        SettingsBreakpoint bkptCfg = m_breakpoints[i];
        QString field;
        field = bkptCfg.filename;
        field += ":";
        QString lineNoStr;
        lineNoStr.sprintf("%d", bkptCfg.lineNo);
        field += lineNoStr;
        breakpointStringList.push_back(field);
    }
    tmpIni.setStringList("Breakpoints", breakpointStringList);


    if(tmpIni.save(filepath))
        infoMsg("Failed to save '%s'", stringToCStr(filepath));

}


void Settings::saveGlobalConfig()
{
    QString globalConfigFilename = QDir::homePath() + "/"  GLOBAL_CONFIG_DIR + "/" + GLOBAL_CONFIG_FILENAME;

    Ini tmpIni;

    tmpIni.appendLoad(globalConfigFilename);

    tmpIni.setBool("General/EnableDebugLog", m_enableDebugLog);

    tmpIni.setString("Gui/CodeFont", m_fontFamily);
    tmpIni.setInt("Gui/CodeFontSize", m_fontSize);

    tmpIni.setString("Gui/MemoryFont", m_memoryFontFamily);
    tmpIni.setInt("Gui/MemoryFontSize", m_memoryFontSize);
    tmpIni.setString("Gui/OutputFont", m_outputFontFamily);
    tmpIni.setInt("Gui/OutputFontSize", m_outputFontSize);
    tmpIni.setString("Gui/GdbOutputFont", m_gdbOutputFontFamily);
    tmpIni.setInt("Gui/GdbOutputFontSize", m_gdbOutputFontSize);

    tmpIni.setBool("Gui/TagsSortByName", m_tagSortByName);
    tmpIni.setBool("Gui/TagsShowLinenumber", m_tagShowLineNumbers);
    
    tmpIni.setStringList("General/ScannerIgnoreDirs", m_sourceIgnoreDirs);

    tmpIni.setByteArray("GuiState/MainWindowState", m_gui_mainwindowState);
    tmpIni.setByteArray("GuiState/MainWindowGeometry", m_gui_mainwindowGeometry);
    tmpIni.setByteArray("GuiState/Splitter1State", m_gui_splitter1State);
    tmpIni.setByteArray("GuiState/Splitter2State", m_gui_splitter2State);
    tmpIni.setByteArray("GuiState/Splitter3State", m_gui_splitter3State);
    tmpIni.setByteArray("GuiState/Splitter4State", m_gui_splitter4State);

    tmpIni.setBool("GuiState/EnableWindowStack", m_viewWindowStack);
    tmpIni.setBool("GuiState/EnableWindowThreads", m_viewWindowThreads);
    tmpIni.setBool("GuiState/EnableWindowBreakpoints", m_viewWindowBreakpoints);
    tmpIni.setBool("GuiState/EnableWindowWatch", m_viewWindowWatch);
    tmpIni.setBool("GuiState/EnableWindowAuto", m_viewWindowAutoVariables);
    tmpIni.setBool("GuiState/EnableWindowTargetOutput", m_viewWindowTargetOutput);
    tmpIni.setBool("GuiState/EnableWindowGdbOutput", m_viewWindowGdbOutput);
    tmpIni.setBool("GuiState/EnableWindowFileBrowser", m_viewWindowFileBrowser);


    if(tmpIni.save(globalConfigFilename))
        infoMsg("Failed to save '%s'", stringToCStr(globalConfigFilename));

}



         
QStringList Settings::getDefaultCppKeywordList()
{
    QStringList keywordList;
    keywordList += "#";
    keywordList += "if";
    keywordList += "def";
    keywordList += "defined";
    keywordList += "define";
    keywordList += "ifdef";
    keywordList += "endif";
    keywordList += "ifndef";
    keywordList += "include";
    return keywordList;
}

QStringList Settings::getDefaultKeywordList()
{
    QStringList keywordList;
    keywordList += "if";
    keywordList += "for";
    keywordList += "while";
    keywordList += "switch";
    keywordList += "case";
    keywordList += "else";
    keywordList += "do";
    keywordList += "false";
    keywordList += "true";
    
    keywordList += "unsigned";
    keywordList += "bool";
    keywordList += "int";
    keywordList += "short";
    keywordList += "long";
    keywordList += "float";
    keywordList += "double";
    keywordList += "void";
    keywordList += "char";
    keywordList += "struct";

    keywordList += "class";
    keywordList += "static";
    keywordList += "volatile";
    keywordList += "return";
    keywordList += "new";
    keywordList += "const";
    

    keywordList += "uint32_t";
    keywordList += "uint16_t";
    keywordList += "uint8_t";
    keywordList += "int32_t";
    keywordList += "int16_t";
    keywordList += "int8_t";
    
    return keywordList;
}


/**
 * @brief Returns the path of the program to debug
 */
QString Settings::getProgramPath()
{
    if(m_connectionMode == MODE_LOCAL)
    {
        return m_lastProgram;
    }
    else
    {
        return m_tcpProgram;
    }
    return "";
}


