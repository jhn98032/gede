//#define ENABLE_DEBUGMSG
/*
 * Copyright (C) 2014-2021 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "settings.h"

#include <QDir>
#include <QStyleFactory>

#include "util.h"
#include "log.h"
#include "ini.h"
#include "config.h"
#include <assert.h>


QString Settings::g_projConfigFilename = PROJECT_CONFIG_FILENAME;


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
    m_viewWindowGedeOutput = true;
    m_viewWindowGdbOutput = true;
    m_viewWindowFileBrowser = true;
    m_enableDebugLog = false;
    m_tabIndentCount = 4;
    m_viewFuncFilter = true;
    m_viewClassFilter = true;
    m_focusOnStop = true;
    
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
#if __APPLE__
    const int fontsize = 11;
#else
    const int fontsize = 8;
#endif
    m_fontFamily = "Monospace";
    m_fontSize = fontsize;
    m_memoryFontFamily = "Monospace";
    m_memoryFontSize = fontsize;
    m_outputFontFamily = "Monospace";
    m_outputFontSize = fontsize;
    m_gdbOutputFontFamily = "Monospace";
    m_gdbOutputFontSize = fontsize;
    m_gedeOutputFontFamily = "Monospace";
    m_gedeOutputFontSize = fontsize;


    m_clrBackground = Qt::black;
    m_clrComment = Qt::green;
    m_clrString = QColor(0,125, 250);
    m_clrIncString = QColor(0,125, 250);
    m_clrKeyword = Qt::yellow;
    m_clrCppKeyword = QColor(240,110,110);
    m_clrCurrentLine = QColor(100,0,0);
    m_clrNumber = Qt::magenta;
    m_clrForeground = Qt::white;
    m_clrSelection = QColor(100,100,100);

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
    m_focusOnStop = true;

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


    loadProjectConfig(getProjectConfigPath());
}
 

/**
* @brief Returns the path of the project config file.
*/
QString Settings::getProjectConfigPath() const
{
    // Get project config path
    QString filepath;
    if(m_globalProjConfig)
        filepath = QDir::homePath() + "/"  GLOBAL_CONFIG_DIR + "/" + PROJECT_GLOBAL_CONFIG_FILENAME;
    else
        filepath = g_projConfigFilename;
    return filepath;
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
    m_gedeOutputFontFamily = tmpIni.getString("Gui/GedeOutputFont", m_outputFontFamily);
    m_gedeOutputFontSize = tmpIni.getInt("Gui/GedeOutputFontSize", m_outputFontSize);

    m_tagSortByName = tmpIni.getBool("Gui/TagsSortByName", false);
    m_tagShowLineNumbers = tmpIni.getBool("Gui/TagsShowLinenumber", true);

    m_tabIndentCount = tmpIni.getInt("Gui/TabIndentCount", m_tabIndentCount);

    m_sourceIgnoreDirs = tmpIni.getStringList("General/ScannerIgnoreDirs", m_sourceIgnoreDirs);

    m_maxTabs = std::max(1, tmpIni.getInt("General/MaxTabs", m_maxTabs));

    m_lastUsedProjectsDir = tmpIni.getStringList("General/LastProjects", QStringList());

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
    m_viewWindowGedeOutput = tmpIni.getBool("GuiState/EnableWindowGedeOutput", m_viewWindowGedeOutput);
    m_viewWindowGdbOutput = tmpIni.getBool("GuiState/EnableWindowGdbOutput", m_viewWindowGdbOutput);
    m_viewWindowFileBrowser = tmpIni.getBool("GuiState/EnableWindowFileBrowser", m_viewWindowFileBrowser);
    m_viewFuncFilter = tmpIni.getBool("GuiState/EnableFuncFilter", m_viewFuncFilter);
    m_viewClassFilter = tmpIni.getBool("GuiState/EnableClassFilter", m_viewClassFilter);
    

    m_clrBackground = tmpIni.getColor("GuiColor/ColorBackground", Qt::black);
    m_clrComment = tmpIni.getColor("GuiColor/ColorComment", Qt::green);
    m_clrString = tmpIni.getColor("GuiColor/ColorString", QColor(0,125, 250));
    m_clrIncString = tmpIni.getColor("GuiColor/ColorIncString", QColor(0,125, 250));
    m_clrKeyword = tmpIni.getColor("GuiColor/ColorKeyword", Qt::yellow);
    m_clrCppKeyword = tmpIni.getColor("GuiColor/ColorCppKeyword", QColor(240,110,110));
    m_clrCurrentLine= tmpIni.getColor("GuiColor/ColorCurrentLine", m_clrCurrentLine);
    m_clrNumber = tmpIni.getColor("GuiColor/ColorNumber", Qt::magenta);
    m_clrForeground = tmpIni.getColor("GuiColor/ColorForeGround", Qt::white);
    m_clrSelection = tmpIni.getColor("GuiColor/ColorSelection", m_clrSelection);

    m_progConScrollback = std::max(1, tmpIni.getInt("ProgramConsole/Scrollback", m_progConScrollback));
    m_progConColorFg = tmpIni.getColor("ProgramConsole/ColorForeground", m_progConColorFg);
    m_progConColorBg = tmpIni.getColor("ProgramConsole/ColorBackground", m_progConColorBg);
    m_progConColorCursor = tmpIni.getColor("ProgramConsole/ColorCursor", m_progConColorCursor);
    for(int clrIdx = 0;clrIdx < 8;clrIdx++)
    {
        QString fieldName;
        fieldName = QString::asprintf("ProgramConsole/ColorNorm%d", clrIdx);
        m_progConColorNorm[clrIdx] = tmpIni.getColor(fieldName, m_progConColorNorm[clrIdx]);
        fieldName = QString::asprintf("ProgramConsole/ColorBright%d", clrIdx);
        m_progConColorBright[clrIdx] = tmpIni.getColor(fieldName, m_progConColorBright[clrIdx]);
    }
    m_progConBackspaceKey = tmpIni.getInt("ProgramConsole/BackspaceKey", m_progConBackspaceKey);
    m_progConDelKey = tmpIni.getInt("ProgramConsole/DelKey", m_progConDelKey);
    m_focusOnStop = tmpIni.getBool("FocusOnStop", m_focusOnStop);
    
}

void Settings::loadProjectConfig(QString filepath)
{
    assert(!filepath.isEmpty());

    // Load from file
    Ini tmpIni;
    if(tmpIni.appendLoad(filepath))
        infoMsg("Failed to load project ini '%s'. File will be created.", stringToCStr(filepath));
    else
    {
          debugMsg("Loaded config from %s", qPrintable(filepath));
    }

    m_download = tmpIni.getBool("Download", true);
    switch(tmpIni.getInt("Mode", MODE_LOCAL))
    {
        default: assert(0);break;
        case MODE_LOCAL: m_connectionMode = MODE_LOCAL;break;
        case MODE_TCP: m_connectionMode = MODE_TCP;break;
        case MODE_SERIAL: m_connectionMode = MODE_SERIAL;break;
        case MODE_COREDUMP: m_connectionMode = MODE_COREDUMP;break;
        case MODE_PID: m_connectionMode = MODE_PID;break;
    }

    m_serialBaudRate = tmpIni.getInt("SerialBaudRate", 115200);
    m_serialPort = tmpIni.getString("SerialPort", "");
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

    m_gotoRuiList.clear();
    m_gotoRuiList = tmpIni.getStringList("GoToRecentlyUsed", m_gotoRuiList);

    
    //
    QStringList breakpointStringList;
    breakpointStringList = tmpIni.getStringList("Breakpoints", breakpointStringList);
    for(int i = 0;i < breakpointStringList.size();i++)
    {
        QString str = breakpointStringList[i];
        if(str.indexOf(':') != -1)
        {
            SettingsBreakpoint bkptCfg;
            bkptCfg.m_filename = str.left(str.indexOf(':'));
            bkptCfg.m_lineNo = str.mid(str.indexOf(':')+1).toInt();
            
            m_breakpoints.push_back(bkptCfg);
        }
    }


}
 

void Settings::save()
{
    saveProjectConfig();
    saveGlobalConfig();
}

void Settings::setProjectConfig(QString filename)
{
    debugMsg("%s(filename:%s)", __func__, qPrintable(filename));
    g_projConfigFilename = filename;
}


void Settings::saveProjectConfig()
{

    QString filepath = getProjectConfigPath();

    debugMsg("Saving config to %s", qPrintable(filepath));

    Ini tmpIni;

    tmpIni.appendLoad(filepath);

    //
    tmpIni.setBool("Download", m_download);
    tmpIni.setInt("TcpPort", m_tcpPort);
    tmpIni.setString("TcpHost", m_tcpHost);
    tmpIni.setInt("SerialBaudRate", m_serialBaudRate);
    tmpIni.setString("SerialPort", m_serialPort);
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

    tmpIni.setStringList("GoToRecentlyUsed", m_gotoRuiList);

    
    //
    QStringList breakpointStringList;
    for(int i = 0;i < m_breakpoints.size();i++)
    {
        SettingsBreakpoint bkptCfg = m_breakpoints[i];
        QString field;
        field = bkptCfg.m_filename;
        field += ":";
        QString lineNoStr;
        lineNoStr = QString::asprintf("%d", bkptCfg.m_lineNo);
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
    tmpIni.setString("Gui/GedeOutputFont", m_gedeOutputFontFamily);
    tmpIni.setInt("Gui/GedeOutputFontSize", m_gedeOutputFontSize);

    tmpIni.setBool("Gui/TagsSortByName", m_tagSortByName);
    tmpIni.setBool("Gui/TagsShowLinenumber", m_tagShowLineNumbers);

    tmpIni.setInt("Gui/TabIndentCount", m_tabIndentCount);

    tmpIni.setInt("General/MaxTabs", m_maxTabs);

    tmpIni.setStringList("General/ScannerIgnoreDirs", m_sourceIgnoreDirs);

    tmpIni.setStringList("General/LastProjects", m_lastUsedProjectsDir);

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
    tmpIni.setBool("GuiState/EnableWindowGedeOutput", m_viewWindowGedeOutput);
    tmpIni.setBool("GuiState/EnableWindowGdbOutput", m_viewWindowGdbOutput);
    tmpIni.setBool("GuiState/EnableWindowFileBrowser", m_viewWindowFileBrowser);
    tmpIni.setBool("GuiState/EnableFuncFilter", m_viewFuncFilter);
    tmpIni.setBool("GuiState/EnableClassFilter", m_viewClassFilter);
    

    tmpIni.setColor("GuiColor/ColorBackground", m_clrBackground);
    tmpIni.setColor("GuiColor/ColorComment", m_clrComment);
    tmpIni.setColor("GuiColor/ColorString", m_clrString);
    tmpIni.setColor("GuiColor/ColorIncString", m_clrIncString);
    tmpIni.setColor("GuiColor/ColorKeyword", m_clrKeyword);
    tmpIni.setColor("GuiColor/ColorCppKeyword", m_clrCppKeyword);
    tmpIni.setColor("GuiColor/ColorCurrentLine", m_clrCurrentLine);
    tmpIni.setColor("GuiColor/ColorNumber", m_clrNumber);
    tmpIni.setColor("GuiColor/ColorForeGround", m_clrForeground);
    tmpIni.setColor("GuiColor/ColorSelection", m_clrSelection);

    tmpIni.setInt("ProgramConsole/Scrollback", m_progConScrollback);

    tmpIni.setColor("ProgramConsole/ColorForeground", m_progConColorFg);
    tmpIni.setColor("ProgramConsole/ColorBackground", m_progConColorBg);
    tmpIni.setColor("ProgramConsole/ColorCursor", m_progConColorCursor);
    for(int clrIdx = 0;clrIdx < 8;clrIdx++)
    {
        QString fieldName;
        fieldName = QString::asprintf("ProgramConsole/ColorNorm%d", clrIdx);
        tmpIni.setColor(fieldName, m_progConColorNorm[clrIdx]);
        fieldName = QString::asprintf("ProgramConsole/ColorBright%d", clrIdx);
        tmpIni.setColor(fieldName, m_progConColorBright[clrIdx]);
    }
    tmpIni.setInt("ProgramConsole/BackspaceKey", m_progConBackspaceKey);
    tmpIni.setInt("ProgramConsole/DelKey", m_progConDelKey);
    tmpIni.setBool("FocusOnStop",m_focusOnStop);

    if(tmpIni.save(globalConfigFilename))
        infoMsg("Failed to save '%s'", stringToCStr(globalConfigFilename));

}



         
QStringList Settings::getDefaultCppKeywordList()
{
    QStringList keywordList;
    keywordList += "#";
    keywordList += "if";
    keywordList += "else";
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

QStringList Settings::getDefaultGoKeywordList()
{
    QStringList keywordList;

    keywordList += "break";
    keywordList += "case";
    keywordList += "chan";
    keywordList += "const";
    keywordList += "continue";
    keywordList += "default";
    keywordList += "defer";
    keywordList += "else";
    keywordList += "fallthrough";
    keywordList += "for";
    keywordList += "func";
    keywordList += "go";
    keywordList += "goto";
    keywordList += "if";
    keywordList += "import";
    keywordList += "interface";
    keywordList += "map";
    keywordList += "package";
    keywordList += "range";
    keywordList += "return";
    keywordList += "select";
    keywordList += "struct";
    keywordList += "switch";
    keywordList += "type";
    keywordList += "var";


    
    return keywordList;
}


QStringList Settings::getDefaultAdaKeywordList()
{
    QStringList keywordList;

    keywordList += "if";
    keywordList += "loop";
    keywordList += "for";
    keywordList += "begin";
    keywordList += "end";
    keywordList += "then";
    keywordList += "return";

    keywordList += "with";
    keywordList += "use";
    keywordList += "in";
    keywordList += "is";
    keywordList += "of";

    keywordList += "procedure";
    keywordList += "function";

    keywordList += "pragma";
    keywordList += "renames";
    keywordList += "import";


    keywordList += "integer";
    keywordList += "array";
    keywordList += "string";
    keywordList += "character";
    keywordList += "natural";

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
    keywordList += "input";
    keywordList += "sleep";
    keywordList += "return";

    keywordList += "do";
    keywordList += "loop";
    keywordList += "until";
    keywordList += "declare";
    keywordList += "select";
    keywordList += "case";

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


QStringList Settings::getDefaultFortranKeywordList()
{
    QStringList keywordList;

    keywordList += "print";

    keywordList += "subroutine";
    keywordList += "program";

    keywordList += "end";
    keywordList += "call";

    keywordList += "real";

    return keywordList;
}

/**
 * @brief Returns the path of the program to debug
 */
QString Settings::getProgramPath() const
{
    return m_lastProgram;
}

/**
 * @brief Returns the path of the program to debug
 */
void Settings::setProgramPath(QString path)
{
    m_lastProgram = path;
}



QStringList Settings::getGoToList()
{
    return m_gotoRuiList;
}
    
void Settings::setGoToList(QStringList list)
{
    while(list.size() > MAX_GOTO_RUI_COUNT)
        list.takeLast();
    m_gotoRuiList = list;
}

QStringList Settings::getLastUsedProjectsDir()
{
    return m_lastUsedProjectsDir;
}

void Settings::setLastUsedProjectDir(QString dirpath)
{
    // Save the config filename to the last used config files
    QString fullConfigPath = QFileInfo(dirpath).absoluteFilePath();
    m_lastUsedProjectsDir.removeAll(fullConfigPath);
    m_lastUsedProjectsDir.prepend(fullConfigPath);
}



