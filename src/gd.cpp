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
    printf("Usage: gd [OPTIONS] [--args PROGRAM_NAME [PROGRAM_ARGUMENTS...]]\n"
                "--no-show-config / --show-config   Shows the configuration window at startup."
           "\n"
           );
    
    return -1;  
}


/**
 * @brief Loads the breakpoints from the settings file and set the breakpoints.
 */
void loadBreakpoints(Settings &cfg, Core &core)
{
    for(int i = 0;i < cfg.m_breakpoints.size();i++)
    {
        SettingsBreakpoint bkptCfg = cfg.m_breakpoints[i];
        core.gdbSetBreakpoint(bkptCfg.filename, bkptCfg.lineNo);
    }
}

    

/**
 * @brief Main program entry.
 */
int main(int argc, char *argv[])
{
    int rc = 0;
    Settings cfg;
    bool showConfigDialog = true;
    
    // Load default config
    cfg.load(CONFIG_FILENAME);
    for(int i = 1;i < argc;i++)
    {
        const char *curArg = argv[i];
        if(strcmp(curArg, "--args") == 0)
        {
            cfg.m_connectionMode = MODE_LOCAL;
            cfg.m_argumentList.clear();
            for(int u = i+1;u < argc;u++)
            {
                if(u == i+1)
                    cfg.m_lastProgram = argv[u];
                else
                    cfg.m_argumentList.push_back(argv[u]);
            }
            argc = i;
        }
        else if(strcmp(curArg, "--show-config") == 0)
            showConfigDialog = true;
        else if(strcmp(curArg, "--no-show-config") == 0)
            showConfigDialog = false;
        else if(strcmp(curArg, "--help") == 0)
        {
            return dumpUsage();
        }
    }

    QApplication app(argc, argv);
    app.setStyle("cleanlooks");

    if(cfg.m_lastProgram.isEmpty())
        showConfigDialog = true;
        
    // Got a program to debug?
    if(showConfigDialog)
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

    //
    if(cfg.m_lastProgram.isEmpty())
    {
        errorMsg("No program to debug");
        return 1;
    }
    
    Core &core = Core::getInstance();

    
    MainWindow w(NULL);

    if(cfg.m_connectionMode == MODE_LOCAL)
        rc = core.initLocal(&cfg, cfg.m_gdbPath, cfg.m_lastProgram, cfg.m_argumentList);
    else
        rc = core.initRemote(&cfg, cfg.m_gdbPath, cfg.m_tcpProgram, cfg.m_tcpHost, cfg.m_tcpPort);

    if(rc)
        return rc;

    w.insertSourceFiles();

    if(cfg.m_reloadBreakpoints)
        loadBreakpoints(cfg, core);

    w.show();

    return app.exec();
}

