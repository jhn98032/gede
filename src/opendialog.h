/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__ABOUTDIALOG_H
#define FILE__ABOUTDIALOG_H

#include <QDialog>

#include "settings.h"
#include "core.h"
#include "ui_opendialog.h"


class OpenDialog : public QDialog
{
    Q_OBJECT

public:

    OpenDialog(QWidget *parent);

    void setProgram(QString program);
    void setArguments(QString program);
    QString getProgram();
    QString getArguments();

    QString getCoreDumpFile();
    
    void setCoreDumpFile(QString coreDumpFile);

    void setInitialBreakpoint(QString list);
    QString getInitialBreakpoint();
    
    
    void setTcpRemoteHost(QString host);
    QString getTcpRemoteHost();
    
    void setTcpRemotePort(int port);
    int getTcpRemotePort();
    
    bool getDownload();
    void setDownload(bool enable);

    
    void setMode(ConnectionMode mode);
    ConnectionMode getMode();

    void setInitCommands(QStringList commandList);
    QStringList getInitCommands();

    void setGdbPath(QString path);
    QString getGdbPath();

    int getRunningPid();
    void setRunningPid(int pid);

    
    void loadConfig(Settings &cfg);
    void saveConfig(Settings *cfg);

    
private:
    void onBrowseForProgram(QString *path);
    
private slots:
    void onConnectionTypeLocal(bool checked);
    void onConnectionTypeTcp(bool checked);
    void onConnectionTypeCoreDump(bool checked);
    void onConnectionTypePid(bool checked);
    
    void onSelectProgram();
    void onSelectCoreFile();
    
    void onSelectRunningPid();

private:
    Ui_OpenDialog m_ui;
    
};

#endif // FILE__ABOUTDIALOG_H

