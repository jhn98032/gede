/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif

#include <QMessageBox>
#include <QDir>

#include "mainwindow.h"
#include "gdbcore.h"
#include "lldbcore.h"
#include "log.h"
#include "util.h"
#include "tree.h"
#include "opendialog.h"
#include "settings.h"
#include "version.h"



static int dumpUsage()
{
    /*
    QMessageBox::information ( NULL, "Unable to start",
                    "Usage: gd --args PROGRAM_NAME",
                    QMessageBox::Ok, QMessageBox::Ok);
      */
    printf("Usage: gede [OPTIONS] [--args PROGRAM_NAME [PROGRAM_ARGUMENTS...]]\n");
    printf("\n");
    printf("Where OPTIONS are:\n");
    printf("  --no-show-config / --show-config   Shows the configuration window at startup.\n");
    printf("  --help                             Displays this text.\n");
    printf("  --version                          Displays the version of gede.\n");
    printf("  --projconfig FILENAME              Specify config filename to use.\n");
    printf("                                     Default is '%s' \n", PROJECT_CONFIG_FILENAME);
    printf("\n");
    printf("Examples:\n");
    printf("\n");
    printf("  To start to debug the application \"my_application\":\n");
    printf("  $ gede --args my_application\n");
    printf("\n");
    
    return -1;  
}

static int dumpVersion()
{
    printf("gede %d.%d.%d\n", GD_MAJOR, GD_MINOR, GD_PATCH);
    
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
        debugMsg("Setting breakpoint at %s:L%d", qPrintable(bkptCfg.m_filename), bkptCfg.m_lineNo);
        core.gdbSetBreakpoint(bkptCfg.m_filename, bkptCfg.m_lineNo);
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

    // Ensure that the config dir exist
    QDir d;
    d.mkdir(QDir::homePath() + "/" + GLOBAL_CONFIG_DIR);

    //
    for(int i = 1;i < argc;i++)
    {
        const char *curArg = argv[i];
        if(strcmp(curArg, "--version") == 0)
        {
            return dumpVersion();
        }
        else if(strcmp(curArg, "--help") == 0)
        {
            return dumpUsage();
        }
        else if((strcmp(curArg, "--projconfig") == 0 || strcmp(curArg, "--proj-config") == 0)
            && i+1 < argc)
        {
            i++;
            cfg.setProjectConfig(argv[i]);
        }
        
    }
    
    // Load default config
    cfg.load();
    for(int i = 1;i < argc;i++)
    {
        const char *curArg = argv[i];
        if((strcmp(curArg, "--projconfig") == 0 || strcmp(curArg, "--proj-config") == 0)
            && i+1 < argc)
        {
            i++;
        }
        else if(strcmp(curArg, "--args") == 0)
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
        else if(strcmp(curArg, "--version") == 0)
        {
            return dumpVersion();
        }
        else // if(strcmp(curArg, "--help") == 0)
        {
            return dumpUsage();
        }
    }

    QApplication app(argc, argv);

    if(!cfg.m_guiStyleName.isEmpty())
    {
        QApplication::setStyle(cfg.m_guiStyleName);
    }

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
    cfg.save();

    //
    if(cfg.getProgramPath().isEmpty())
    {
        errorMsg("No program to debug");
        return 1;
    }

    LldbCore core;
//    GdbCore core;
    Core::setInstance(&core);
    
    
    
    MainWindow w(NULL);

    if(cfg.m_connectionMode == MODE_LOCAL)
        rc = core.initLocal(&cfg, cfg.m_gdbPath, cfg.m_lastProgram, cfg.m_argumentList);
    else if(cfg.m_connectionMode == MODE_COREDUMP)
        rc = core.initCoreDump(&cfg, cfg.m_gdbPath, cfg.m_lastProgram, cfg.m_coreDumpFile);
    else if(cfg.m_connectionMode == MODE_PID)
        rc = core.initPid(&cfg, cfg.m_gdbPath, cfg.m_lastProgram, cfg.m_runningPid);
    else
        rc = core.initRemote(&cfg, cfg.m_gdbPath, cfg.m_lastProgram, cfg.m_tcpHost, cfg.m_tcpPort);

    if(rc)
        return rc;

    // Set the status line
    w.setStatusLine(cfg);

    w.insertSourceFiles();

    if(cfg.m_reloadBreakpoints)
        loadBreakpoints(cfg, core);

    if(rc == 0 && (cfg.m_connectionMode == MODE_LOCAL || cfg.m_connectionMode == MODE_TCP))
        core.gdbRun();

    w.show();

    return app.exec();
}

