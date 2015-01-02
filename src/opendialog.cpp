#include "opendialog.h"
#include "version.h"
#include "log.h"
#include "util.h"

#include <QFileDialog>
#include <QDir>


OpenDialog::OpenDialog(QWidget *parent)
    : QDialog(parent)
{
    
    m_ui.setupUi(this);

    connect(m_ui.pushButton_selectFile, SIGNAL(clicked()), SLOT(onSelectProgram()));
    connect(m_ui.pushButton_selectTcpProgram, SIGNAL(clicked()), SLOT(onSelectTcpProgram()));
    connect(m_ui.radioButton_localProgram, SIGNAL(toggled(bool)), SLOT(onConnectionTypeLocal(bool)));
    connect(m_ui.radioButton_gdbServerTcp, SIGNAL(toggled(bool)), SLOT(onConnectionTypeTcp(bool)));

}

void OpenDialog::setMode(ConnectionMode mode)
{
    m_ui.radioButton_localProgram->setChecked(false);
    m_ui.radioButton_gdbServerTcp->setChecked(false);
    onConnectionTypeLocal(false);
    onConnectionTypeTcp(false);
    
    if(mode == MODE_TCP)
    {
        m_ui.radioButton_gdbServerTcp->setChecked(true);
        onConnectionTypeTcp(true);        
    }
    else // if(mode == MODE_LOCAL)
    {
        m_ui.radioButton_localProgram->setChecked(true);
        onConnectionTypeLocal(true);
    }
    
}

ConnectionMode OpenDialog::getMode()
{
    if(m_ui.radioButton_gdbServerTcp->isChecked())    
        return MODE_TCP;
    else
        return MODE_LOCAL;
}

QString OpenDialog::getProgram()
{
    return m_ui.lineEdit_program->text();
}

QString OpenDialog::getArguments()
{
    return m_ui.lineEdit_arguments->text();
}
    

void OpenDialog::setProgram(QString program)
{
    m_ui.lineEdit_program->setText(program);

}

void OpenDialog::setInitCommands(QStringList commandList)
{
    QString str;
    str = commandList.join("\n");
    m_ui.plainTextEdit_initCommands->setPlainText(str);
}


QStringList OpenDialog::getInitCommands()
{
    return m_ui.plainTextEdit_initCommands->toPlainText().split("\n");
}    

void OpenDialog::setArguments(QString arguments)
{
    m_ui.lineEdit_arguments->setText(arguments);

}

void OpenDialog::onBrowseForProgram(QString *path)
{
    // Get start dir
    QString startPath = *path;
    if(!startPath.isEmpty())
    {
        dividePath(startPath, NULL, &startPath);
    }
    else
        startPath = QDir::currentPath();
        
    // Open dialog
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select Program"), startPath, tr("All Files (*)"));
    if(!fileName.isEmpty())
        *path = fileName;
}

void OpenDialog::onSelectTcpProgram()
{
    QString path = m_ui.lineEdit_tcpProgram->text();

    onBrowseForProgram(&path);
    
    // Fill in the selected path
    m_ui.lineEdit_tcpProgram->setText(path);
}

void OpenDialog::onSelectProgram()
{
    QString path = m_ui.lineEdit_program->text();

    onBrowseForProgram(&path);
    
    // Fill in the selected path
    m_ui.lineEdit_program->setText(path);
}

void OpenDialog::onConnectionTypeLocal(bool checked)
{
    m_ui.lineEdit_program->setEnabled(checked);
    m_ui.pushButton_selectFile->setEnabled(checked);
    m_ui.lineEdit_arguments->setEnabled(checked);
}

void OpenDialog::onConnectionTypeTcp(bool checked)
{
    m_ui.pushButton_selectTcpProgram->setEnabled(checked);
    m_ui.lineEdit_tcpHost->setEnabled(checked);
    m_ui.lineEdit_tcpPort->setEnabled(checked);
    m_ui.lineEdit_tcpProgram->setEnabled(checked);
}

void OpenDialog::setTcpRemoteHost(QString host)
{
    m_ui.lineEdit_tcpHost->setText(host);
}

QString OpenDialog::getTcpRemoteHost()
{
    return m_ui.lineEdit_tcpHost->text();
}


void OpenDialog::setTcpRemoteProgram(QString path)
{
    m_ui.lineEdit_tcpProgram->setText(path);
}

QString OpenDialog::getTcpRemoteProgram()
{
    return m_ui.lineEdit_tcpProgram->text();
}

    
void OpenDialog::setTcpRemotePort(int port)
{
    QString portStr;
    portStr.sprintf("%d", port);
    m_ui.lineEdit_tcpPort->setText(portStr);
}

int OpenDialog::getTcpRemotePort()
{
    return m_ui.lineEdit_tcpPort->text().toInt();
}


QString OpenDialog::getGdbPath()
{
    return m_ui.lineEdit_gdbCommand->text();
}


void OpenDialog::setGdbPath(QString path)
{
    return m_ui.lineEdit_gdbCommand->setText(path);
}

void OpenDialog::saveConfig(Settings *cfg)
{
    OpenDialog &dlg = *this;
    cfg->m_lastProgram = dlg.getProgram();
    cfg->m_argumentList = dlg.getArguments().split(' ');
    cfg->m_connectionMode = dlg.getMode();
    cfg->m_tcpPort = dlg.getTcpRemotePort();
    cfg->m_tcpHost = dlg.getTcpRemoteHost();
    cfg->m_tcpProgram = dlg.getTcpRemoteProgram();
    cfg->m_initCommands = dlg.getInitCommands();
    cfg->m_gdbPath = dlg.getGdbPath();

    if(dlg.m_ui.checkBox_reloadBreakpoints->checkState() == Qt::Checked)
        cfg->m_reloadBreakpoints = true;
    else
        cfg->m_reloadBreakpoints = false;
    
    

}

void OpenDialog::loadConfig(Settings &cfg)
{
    OpenDialog &dlg = *this;
    dlg.setMode(cfg.m_connectionMode);

    dlg.setTcpRemotePort(cfg.m_tcpPort);
    dlg.setTcpRemoteHost(cfg.m_tcpHost);
    dlg.setTcpRemoteProgram(cfg.m_tcpProgram);
    dlg.setInitCommands(cfg.m_initCommands);
    dlg.setGdbPath(cfg.m_gdbPath);

    dlg.m_ui.checkBox_reloadBreakpoints->setChecked(cfg.m_reloadBreakpoints);

    dlg.setProgram(cfg.m_lastProgram);
    QStringList defList;
    dlg.setArguments(cfg.m_argumentList.join(" "));

}



