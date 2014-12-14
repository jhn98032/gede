#include <QtGui/QApplication>
#include "mainwindow.h"
#include "core.h"
#include "log.h"
#include "util.h"
#include <QMessageBox>
#include "tree.h"
#include "opendialog.h"
#include "settings.h"


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
    Settings cfg;
    
    // Load default config
    cfg.load(CONFIG_FILENAME);
    for(int i = 1;i < argc;i++)
    {
        const char *curArg = argv[i];
        if(strcmp(curArg, "--args") == 0)
        {
            cfg.connectionMode = MODE_LOCAL;
            for(int u = i+1;u < argc;u++)
                cfg.argumentList.push_back(argv[u]);
            argc = i;
        }
        else if(strcmp(curArg, "--help") == 0)
        {
            return dumpUsage();
        }
    }

    QApplication app(argc, argv);
    app.setStyle("cleanlooks");

    
    // Got a program to debug?
    if(cfg.argumentList.size() < 1)
    {
        // Ask user for program
        OpenDialog dlg(NULL);
        
        dlg.loadConfig(cfg);

        if(dlg.exec() != QDialog::Accepted)
            return 1;

        dlg.saveConfig(&cfg);
    }

    // Save config
    cfg.save(CONFIG_FILENAME);

    
    Core &core = Core::getInstance();

    
    MainWindow w(NULL);

    if(cfg.connectionMode == MODE_LOCAL)
        core.initLocal(cfg.gdbPath, cfg.argumentList);
    else
        core.initRemote(cfg.gdbPath, cfg.tcpProgram, cfg.tcpHost, cfg.tcpPort);
    
    w.insertSourceFiles();
    
    w.show();

    return app.exec();

}

