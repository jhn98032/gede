#include <QtGui/QApplication>
#include "mainwindow.h"
#include "core.h"
#include "log.h"
#include "util.h"
#include <QMessageBox>
#include "tree.h"
#include "opendialog.h"


static int dumpUsage()
{
    /*
    QMessageBox::information ( NULL, "Unable to start",
                    "Usage: gd --args PROGRAM_NAME",
                    QMessageBox::Ok, QMessageBox::Ok);
      */
    printf("Usage: gd --args PROGRAM_NAME [PROGRAM_ARGUMENTS...]\n"
           "\n"
           );
    
    return -1;  
}

/**
 * @brief Main program entry.
 */
int main(int argc, char *argv[])
{
    QStringList argumentList;
    ConnectionMode connectionMode = MODE_LOCAL;
    int tcpPort = 0;
    QString tcpHost;
    QString tcpProgram;
        
    // Load default config
    Ini tmpIni;
    tmpIni.appendLoad(CONFIG_FILENAME);
    connectionMode = tmpIni.getInt("Mode") == MODE_LOCAL ? MODE_LOCAL : MODE_TCP;
    tcpPort = tmpIni.getInt("TcpPort", 2000);
    tcpHost = tmpIni.getString("TcpHost", "localhost");
    tcpProgram = tmpIni.getString("TcpProgram", "");
    
    for(int i = 1;i < argc;i++)
    {
        const char *curArg = argv[i];
        if(strcmp(curArg, "--args") == 0)
        {
            connectionMode = MODE_LOCAL;
            for(int u = i+1;u < argc;u++)
                argumentList.push_back(argv[u]);
            argc = i;
        }
        else if(strcmp(curArg, "--help") == 0)
        {
            return dumpUsage();
        }
    }

    QApplication a(argc, argv);

    
    // Got a program to debug?
    if(argumentList.size() < 1)
    {
        // Ask user for program
        OpenDialog dlg(NULL);
        

        dlg.setMode(connectionMode);

        dlg.setTcpRemotePort(tcpPort);
        dlg.setTcpRemoteHost(tcpHost);
        dlg.setTcpRemoteProgram(tcpProgram);
        
        dlg.setProgram(tmpIni.getString("LastProgram", ""));
        QStringList defList;
        dlg.setArguments(tmpIni.getStringList("LastProgramArguments", defList).join(" "));
    
        if(dlg.exec() != QDialog::Accepted)
            return 1;
        argumentList.clear();
        argumentList += dlg.getProgram();
        argumentList += dlg.getArguments().split(' ');
        connectionMode = dlg.getMode();
        tcpPort = dlg.getTcpRemotePort();
        tcpHost = dlg.getTcpRemoteHost();
        tcpProgram = dlg.getTcpRemoteProgram();
    }

    // Save config
    tmpIni.setInt("TcpPort", tcpPort);
    tmpIni.setString("TcpHost", tcpHost);
    tmpIni.setInt("Mode", (int)connectionMode);
    tmpIni.setString("LastProgram", argumentList[0]);
    tmpIni.setString("TcpProgram", tcpProgram);
    QStringList tmpArgs;
    tmpArgs = argumentList;
    tmpArgs.pop_front();
    tmpIni.setStringList("LastProgramArguments", tmpArgs);
    tmpIni.save(CONFIG_FILENAME);

    
    Core &core = Core::getInstance();

    
    MainWindow w(NULL);

    if(connectionMode == MODE_LOCAL)
        core.initLocal(argumentList);
    else
        core.initRemote(tcpProgram, tcpHost, tcpPort);
    
    w.insertSourceFiles();
    
    w.show();

    return a.exec();

}

