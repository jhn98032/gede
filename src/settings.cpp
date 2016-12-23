/*
 * Copyright (C) 2014-2015 Johan Henriksson.
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
        infoMsg("Failed to global ini '%s'. File will be created.", stringToCStr(globalConfigFilename));

    loadDefaultsGui();
    loadDefaultsAdvanced();
    
    m_fontFamily = tmpIni.getString("Font", m_fontFamily);
    m_fontSize = tmpIni.getInt("FontSize", m_fontSize);
    m_memoryFontFamily = tmpIni.getString("MemoryFont", m_memoryFontFamily);
    m_memoryFontSize = tmpIni.getInt("MemoryFontSize", m_memoryFontSize);
    m_outputFontFamily = tmpIni.getString("OutputFont", m_outputFontFamily);
    m_outputFontSize = tmpIni.getInt("OutputFontSize", m_outputFontSize);
    m_gdbOutputFontFamily = tmpIni.getString("GdbOutputFont", m_outputFontFamily);
    m_gdbOutputFontSize = tmpIni.getInt("GdbOutputFontSize", m_outputFontSize);

    m_sourceIgnoreDirs = tmpIni.getStringList("ScannerIgnoreDirs", m_sourceIgnoreDirs);

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

    tmpIni.setString("Font", m_fontFamily);
    tmpIni.setInt("FontSize", m_fontSize);

    tmpIni.setString("MemoryFont", m_memoryFontFamily);
    tmpIni.setInt("MemoryFontSize", m_memoryFontSize);
    tmpIni.setString("OutputFont", m_outputFontFamily);
    tmpIni.setInt("OutputFontSize", m_outputFontSize);
    tmpIni.setString("GdbOutputFont", m_gdbOutputFontFamily);
    tmpIni.setInt("GdbOutputFontSize", m_gdbOutputFontSize);

    tmpIni.setStringList("ScannerIgnoreDirs", m_sourceIgnoreDirs);

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


