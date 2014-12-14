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
    
    void setTcpRemoteHost(QString host);
    QString getTcpRemoteHost();
    
    void setTcpRemotePort(int port);
    int getTcpRemotePort();
    

    void setTcpRemoteProgram(QString path);
    QString getTcpRemoteProgram();
    
    void setMode(ConnectionMode mode);
    ConnectionMode getMode();

    void setInitCommands(QStringList commandList);
    QStringList getInitCommands();

    void setGdbPath(QString path);
    QString getGdbPath();

    void loadConfig(Settings &cfg);
    void saveConfig(Settings *cfg);
    
    
private:
    void onBrowseForProgram(QString *path);
    
private slots:
    void onConnectionTypeLocal(bool checked);
    void onConnectionTypeTcp(bool checked);
    
    void onSelectTcpProgram();
    void onSelectProgram();

private:
    Ui_OpenDialog m_ui;
    
};

#endif // FILE__ABOUTDIALOG_H

