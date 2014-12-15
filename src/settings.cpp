#include "settings.h"

#include "ini.h"

Settings::Settings()
: m_connectionMode(MODE_LOCAL)
    ,m_tcpPort(0)
{

}

void Settings::load(QString filepath)
{
    Ini tmpIni;
    tmpIni.appendLoad(filepath);
    m_connectionMode = tmpIni.getInt("Mode") == MODE_LOCAL ? MODE_LOCAL : MODE_TCP;
    m_tcpPort = tmpIni.getInt("TcpPort", 2000);
    m_tcpHost = tmpIni.getString("TcpHost", "localhost");
    m_tcpProgram = tmpIni.getString("TcpProgram", "");
    m_initCommands = tmpIni.getStringList("InitCommands", m_initCommands);
    m_gdbPath = tmpIni.getString("GdpPath", "gdb");
    m_lastProgram = tmpIni.getString("LastProgram", "");
    m_argumentList = tmpIni.getStringList("LastProgramArguments", m_argumentList);

    m_fontFamily = tmpIni.getString("Font","Monospace");
    m_fontSize = tmpIni.getInt("FontSize", 8);

}
 
void Settings::save(QString filepath)
{

    Ini tmpIni;
    tmpIni.appendLoad(filepath);
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
    
    tmpIni.setString("Font", m_fontFamily);
    tmpIni.setInt("FontSize", m_fontSize);

    tmpIni.save(filepath);

}
         
        
