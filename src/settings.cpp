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
#include <QStyleFactory>



Settings::Settings()
: m_globalProjConfig(false),
    m_connectionMode(MODE_LOCAL)
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
    m_tabIndentCount = 4;
    
    // Set cleanlooks as default on Debian
    DistroType distroType = DISTRO_UNKNOWN;
    detectDistro(&distroType, NULL);
    if(distroType == DISTRO_DEBIAN)
        m_guiStyleName = "Cleanlooks";
    else
        m_guiStyleName = "";
    

    m_currentLineStyle = FILLED_RECT;
    
}

void Settings::loadDefaultsGui()
{
    m_maxTabs = 15;
    
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
    m_clrCurrentLine = QColor(100,0,0);
    m_clrNumber = Qt::magenta;
    m_clrForeground = Qt::white;

    m_tagSortByName = false;
    m_tagShowLineNumbers = true;

    m_tabIndentCount = 4;

    m_currentLineStyle = FILLED_RECT;
    m_showLineNo = true;


    m_progConScrollback = 1000;

    m_progConColorFg = Qt::black;
    m_progConColorBg = Qt::white;
    m_progConColorCursor = Qt::black;
    m_progConColorNorm[0] =   QColor(0,    0,   0);
    m_progConColorNorm[1] =   QColor(170,  0,   0);
    m_progConColorNorm[2] =   QColor(0,  170,   0);
    m_progConColorNorm[3] =   QColor(170, 85,   0);
    m_progConColorNorm[4] =   QColor(0,    0, 170);
    m_progConColorNorm[5] =   QColor(170,  0, 170);
    m_progConColorNorm[6] =   QColor(0,  170, 170);
    m_progConColorNorm[7] =   QColor(170,170, 170);
    m_progConColorBright[0] = QColor(85,  85,  85);
    m_progConColorBright[1] = QColor(255, 85,  85);
    m_progConColorBright[2] = QColor(85, 255,  85);
    m_progConColorBright[3] = QColor(255,255,  85);
    m_progConColorBright[4] = QColor(85,  85, 255);
    m_progConColorBright[5] = QColor(255, 85, 255);
    m_progConColorBright[6] = QColor(85, 255, 255);
    m_progConColorBright[7] = QColor(255,255, 255);

    m_progConBackspaceKey = 0;
    m_progConDelKey = 2;

    m_variablePopupDelay = 300;
}

void Settings::loadDefaultsAdvanced()
{
    m_sourceIgnoreDirs.clear();
    m_sourceIgnoreDirs.append("/build");
    m_sourceIgnoreDirs.append("/usr");
    
}


void Settings::load()
{
    loadGlobalConfig();
    loadProjectConfig();
}
 


void Settings::loadGlobalConfig()
{
    // Load from file
    QString globalConfigFilename = QDir::homePath() + "/"  GLOBAL_CONFIG_DIR + "/" + GLOBAL_CONFIG_FILENAME;
    Ini tmpIni;
    if(tmpIni.appendLoad(globalConfigFilename))
        infoMsg("Failed to load global ini '%s'. File will be created.", stringToCStr(globalConfigFilename));

    m_globalProjConfig = tmpIni.getBool("General/GlobalProjConfig", false);
    
    loadDefaultsGui();
    loadDefaultsAdvanced();

    m_enableDebugLog = tmpIni.getBool("General/EnableDebugLog", false);

    m_variablePopupDelay = tmpIni.getInt("Gui/VariablePopupDelay", m_variablePopupDelay);

    switch(tmpIni.getInt("Gui/CurrentLineStyle", m_currentLineStyle))
    {
        case HOLLOW_RECT: m_currentLineStyle = HOLLOW_RECT;break;
        default:
        case FILLED_RECT: m_currentLineStyle = FILLED_RECT;break;
    };

    m_showLineNo = tmpIni.getBool("Gui/ShowLineNo", m_showLineNo);
    m_guiStyleName = tmpIni.getString("Gui/Style", m_guiStyleName);
    // Verify that the style name is valid
    QStyleFactory sf;
    QStringList styleList = sf.keys();
    if(!styleList.contains(m_guiStyleName) && !m_guiStyleName.isEmpty())
    {
        warnMsg("Invalid style name '%s'", stringToCStr(m_guiStyleName));
        m_guiStyleName = "";
    }
       
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

    m_tabIndentCount = tmpIni.getInt("Gui/TabIndentCount", m_tabIndentCount);

    m_sourceIgnoreDirs = tmpIni.getStringList("General/ScannerIgnoreDirs", m_sourceIgnoreDirs);

    m_maxTabs = std::max(1, tmpIni.getInt("General/MaxTabs", m_maxTabs));

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


    m_clrBackground = tmpIni.getColor("GuiColor/ColorBackground", Qt::black);
    m_clrComment = tmpIni.getColor("GuiColor/ColorComment", Qt::green);
    m_clrString = tmpIni.getColor("GuiColor/ColorString", QColor(0,125, 250));
    m_clrIncString = tmpIni.getColor("GuiColor/ColorIncString", QColor(0,125, 250));
    m_clrKeyword = tmpIni.getColor("GuiColor/ColorKeyword", Qt::yellow);
    m_clrCppKeyword = tmpIni.getColor("GuiColor/ColorCppKeyword", QColor(240,110,110));
    m_clrCurrentLine= tmpIni.getColor("GuiColor/ColorCurrentLine", m_clrCurrentLine);
    m_clrNumber = tmpIni.getColor("GuiColor/ColorNumber", Qt::magenta);
    m_clrForeground = tmpIni.getColor("GuiColor/ColorForeGround", Qt::white);


    m_progConScrollback = std::max(1, tmpIni.getInt("ProgramConsole/Scrollback", m_progConScrollback));
    m_progConColorFg = tmpIni.getColor("ProgramConsole/ColorForeground", m_progConColorFg);
    m_progConColorBg = tmpIni.getColor("ProgramConsole/ColorBackground", m_progConColorBg);
    m_progConColorCursor = tmpIni.getColor("ProgramConsole/ColorCursor", m_progConColorCursor);
    for(int clrIdx = 0;clrIdx < 8;clrIdx++)
    {
        QString fieldName;
        fieldName.sprintf("ProgramConsole/ColorNorm%d", clrIdx);
        m_progConColorNorm[clrIdx] = tmpIni.getColor(fieldName, m_progConColorNorm[clrIdx]);
        fieldName.sprintf("ProgramConsole/ColorBright%d", clrIdx);
        m_progConColorBright[clrIdx] = tmpIni.getColor(fieldName, m_progConColorBright[clrIdx]);
    }
    m_progConBackspaceKey = tmpIni.getInt("ProgramConsole/BackspaceKey", m_progConBackspaceKey);
    m_progConDelKey = tmpIni.getInt("ProgramConsole/DelKey", m_progConDelKey);
    
}

void Settings::loadProjectConfig()
{
    // Get project config path
    QString filepath;
    if(m_globalProjConfig)
        filepath = QDir::homePath() + "/"  GLOBAL_CONFIG_DIR + "/" + PROJECT_GLOBAL_CONFIG_FILENAME;
    else
        filepath = PROJECT_CONFIG_FILENAME;

    // Load from file
    Ini tmpIni;
    if(tmpIni.appendLoad(filepath))
        infoMsg("Failed to load project ini '%s'. File will be created.", stringToCStr(filepath));


    m_download = tmpIni.getBool("Download", true);
    switch(tmpIni.getInt("Mode", MODE_LOCAL))
    {
        default:
        case MODE_LOCAL: m_connectionMode = MODE_LOCAL;break;
        case MODE_TCP: m_connectionMode = MODE_TCP;break;
        case MODE_COREDUMP: m_connectionMode = MODE_COREDUMP;break;
        case MODE_PID: m_connectionMode = MODE_PID;break;
    }
    m_tcpPort = tmpIni.getInt("TcpPort", 2000);
    m_tcpHost = tmpIni.getString("TcpHost", "localhost");
    m_initCommands = tmpIni.getStringList("InitCommands", m_initCommands);
    m_gdbPath = tmpIni.getString("GdpPath", "gdb");
    m_lastProgram = tmpIni.getString("LastProgram", "");
    m_argumentList = tmpIni.getStringList("LastProgramArguments", m_argumentList);

    m_coreDumpFile = tmpIni.getString("CoreDumpFile", "./core");
    
    m_runningPid = tmpIni.getInt("RunningPid", 0);
        
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

    QString filepath;

    if(m_globalProjConfig)
        filepath = QDir::homePath() + "/"  GLOBAL_CONFIG_DIR + "/" + PROJECT_GLOBAL_CONFIG_FILENAME;
    else
        filepath = PROJECT_CONFIG_FILENAME;
    
    Ini tmpIni;

    tmpIni.appendLoad(filepath);

    //
    tmpIni.setBool("Download", m_download);
    tmpIni.setInt("TcpPort", m_tcpPort);
    tmpIni.setString("TcpHost", m_tcpHost);
    tmpIni.setInt("Mode", (int)m_connectionMode);
    tmpIni.setString("LastProgram", m_lastProgram);
    tmpIni.setStringList("InitCommands", m_initCommands);
    tmpIni.setString("GdpPath", m_gdbPath);
    tmpIni.setString("CoreDumpFile", m_coreDumpFile);
    
    tmpIni.setInt("RunningPid", m_runningPid);

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

    tmpIni.setBool("General/GlobalProjConfig", m_globalProjConfig);
    
    tmpIni.setBool("General/EnableDebugLog", m_enableDebugLog);

    tmpIni.setInt("Gui/CurrentLineStyle", m_currentLineStyle);

    tmpIni.setInt("Gui/VariablePopupDelay", m_variablePopupDelay);

    tmpIni.setBool("Gui/ShowLineNo", m_showLineNo);
    
    tmpIni.setString("Gui/Style", m_guiStyleName);
 
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

    tmpIni.setInt("Gui/TabIndentCount", m_tabIndentCount);

    tmpIni.setInt("General/MaxTabs", m_maxTabs);

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


    tmpIni.setColor("GuiColor/ColorBackground", m_clrBackground);
    tmpIni.setColor("GuiColor/ColorComment", m_clrComment);
    tmpIni.setColor("GuiColor/ColorString", m_clrString);
    tmpIni.setColor("GuiColor/ColorIncString", m_clrIncString);
    tmpIni.setColor("GuiColor/ColorKeyword", m_clrKeyword);
    tmpIni.setColor("GuiColor/ColorCppKeyword", m_clrCppKeyword);
    tmpIni.setColor("GuiColor/ColorCurrentLine", m_clrCurrentLine);
    tmpIni.setColor("GuiColor/ColorNumber", m_clrNumber);
    tmpIni.setColor("GuiColor/ColorForeGround", m_clrForeground);

    tmpIni.setInt("ProgramConsole/Scrollback", m_progConScrollback);

    tmpIni.setColor("ProgramConsole/ColorForeground", m_progConColorFg);
    tmpIni.setColor("ProgramConsole/ColorBackground", m_progConColorBg);
    tmpIni.setColor("ProgramConsole/ColorCursor", m_progConColorCursor);
    for(int clrIdx = 0;clrIdx < 8;clrIdx++)
    {
        QString fieldName;
        fieldName.sprintf("ProgramConsole/ColorNorm%d", clrIdx);
        tmpIni.setColor(fieldName, m_progConColorNorm[clrIdx]);
        fieldName.sprintf("ProgramConsole/ColorBright%d", clrIdx);
        tmpIni.setColor(fieldName, m_progConColorBright[clrIdx]);
    }
    tmpIni.setInt("ProgramConsole/BackspaceKey", m_progConBackspaceKey);
    tmpIni.setInt("ProgramConsole/DelKey", m_progConDelKey);
 
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
    keywordList += "error";
    keywordList += "elif";
    keywordList += "warning";

    return keywordList;
}

QStringList Settings::getDefaultCxxKeywordList()
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
    keywordList += "default";

    keywordList += "sizeof";
    keywordList += "typedef";
    
    keywordList += "enum";
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
    keywordList += "new";
    keywordList += "const";

    keywordList += "return";
    keywordList += "break";
    keywordList += "continue";
    

    keywordList += "uint32_t";
    keywordList += "uint16_t";
    keywordList += "uint8_t";
    keywordList += "int32_t";
    keywordList += "int16_t";
    keywordList += "int8_t";
    
    keywordList += "quint32";
    keywordList += "quint16";
    keywordList += "quint8";
    keywordList += "qint32";
    keywordList += "qint16";
    keywordList += "qint8";
    
   return keywordList;
}


QStringList Settings::getDefaultRustKeywordList()
{
    QStringList keywordList;

    //
    keywordList += "as";
    keywordList += "break";
    keywordList += "const";
    keywordList += "continue";
    keywordList += "crate";
    keywordList += "else";
    keywordList += "enum";
    keywordList += "extern";
    keywordList += "false";
    keywordList += "fn";
    keywordList += "for";
    keywordList += "if";
    keywordList += "impl";
    keywordList += "in";
    keywordList += "let";
    keywordList += "loop";
    keywordList += "match";
    keywordList += "mod";
    keywordList += "move";
    keywordList += "mut";
    keywordList += "pub";
    keywordList += "ref";
    keywordList += "return";
    keywordList += "self";
    keywordList += "static";
    keywordList += "struct";
    keywordList += "super";
    keywordList += "trait";
    keywordList += "true";
    keywordList += "type";
    keywordList += "unsafe";
    keywordList += "use";
    keywordList += "where";
    keywordList += "while";
    keywordList += "yield";


    keywordList += "println";
    keywordList += "println!";
    
    keywordList += "i64";
    keywordList += "i32";
    keywordList += "i8";
    keywordList += "i16";
    keywordList += "u64";
    keywordList += "u32";
    keywordList += "u8";
    keywordList += "u16";
    keywordList += "f32";
    keywordList += "f64";

    keywordList += "bool";

    return keywordList;
}

QStringList Settings::getDefaultBasicKeywordList()
{
    QStringList keywordList;

    keywordList += "print";
    keywordList += "sleep";
    keywordList += "return";

    keywordList += "do";
    keywordList += "loop";
    keywordList += "until";
    keywordList += "declare";

    keywordList += "cls";
    keywordList += "function";
    keywordList += "sub";
    keywordList += "as";
    keywordList += "end";
    keywordList += "dim";
    
    keywordList += "byte";
    keywordList += "const";
    keywordList += "double";
    keywordList += "enum";
    keywordList += "integer";
    keywordList += "long";
    keywordList += "longint";
    keywordList += "short";
    keywordList += "string";
    keywordList += "ubyte";
    keywordList += "uinteger";
    keywordList += "ulongint";
    keywordList += "union";
    keywordList += "unsigned";
    keywordList += "ushort";
    keywordList += "wstring";
    keywordList += "zstring";


    return keywordList;
}


/**
 * @brief Returns the path of the program to debug
 */
QString Settings::getProgramPath()
{
    return m_lastProgram;
}


