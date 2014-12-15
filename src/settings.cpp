#include "settings.h"

#include "ini.h"

Settings::Settings()
: connectionMode(MODE_LOCAL)
    ,tcpPort(0)
{

}

void Settings::load(QString filepath)
{
    Ini tmpIni;
    tmpIni.appendLoad(filepath);
    connectionMode = tmpIni.getInt("Mode") == MODE_LOCAL ? MODE_LOCAL : MODE_TCP;
    tcpPort = tmpIni.getInt("TcpPort", 2000);
    tcpHost = tmpIni.getString("TcpHost", "localhost");
    tcpProgram = tmpIni.getString("TcpProgram", "");
    initCommands = tmpIni.getStringList("InitCommands", initCommands);
    gdbPath = tmpIni.getString("GdpPath", "gdb");
    lastProgram = tmpIni.getString("LastProgram", "");
    QStringList defList;
    argumentList = tmpIni.getStringList("LastProgramArguments", defList);

    m_fontFamily = tmpIni.getString("Font","Monospace");
    m_fontSize = tmpIni.getInt("FontSize", 8);

}
 
void Settings::save(QString filepath)
{
     Settings &cfg = *this;

    Ini tmpIni;
    tmpIni.appendLoad(filepath);
    tmpIni.setInt("TcpPort", cfg.tcpPort);
    tmpIni.setString("TcpHost", cfg.tcpHost);
    tmpIni.setInt("Mode", (int)cfg.connectionMode);
    tmpIni.setString("LastProgram", lastProgram);
    tmpIni.setString("TcpProgram", cfg.tcpProgram);
    tmpIni.setStringList("InitCommands", cfg.initCommands);
    tmpIni.setString("GdpPath", cfg.gdbPath);
    QStringList tmpArgs;
    tmpArgs = cfg.argumentList;
    tmpIni.setStringList("LastProgramArguments", tmpArgs);
    
    tmpIni.setString("Font", m_fontFamily);
    tmpIni.setInt("FontSize", m_fontSize);

    tmpIni.save(filepath);

}
         
        
