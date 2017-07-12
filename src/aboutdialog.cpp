/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "aboutdialog.h"

#include "version.h"
#include <QProcess>
#include "util.h"
#include "log.h"



AboutDialog::AboutDialog(QWidget *parent, Settings *cfg)
    : QDialog(parent)
{
    
    m_ui.setupUi(this);

    //
    QString verStr;
    verStr.sprintf("Version: v%d.%d.%d", GD_MAJOR, GD_MINOR, GD_PATCH);
    m_ui.label_version->setText(verStr);

    //
    QString buildStr;
    buildStr = __DATE__;
    buildStr += " ";
    buildStr += __TIME__;
    m_ui.label_buildDate->setText("Built: " + buildStr);


    QString qtVersionStr;
    qtVersionStr.sprintf("Qt: %s (compiled) / %s (running)", QT_VERSION_STR, qVersion());
    m_ui.label_qtVersion->setText(qtVersionStr);

    
    QString gdbPath = "Gdb: " + cfg->m_gdbPath + " ('" + getGdbVersion(cfg->m_gdbPath) + "')";
    m_ui.label_gdbPath->setText(gdbPath);

    QString distroName;
    detectDistro(NULL, &distroName);
    QString osText = "Running on " + distroName;
    m_ui.label_os->setText(osText);

}


QString AboutDialog::getGdbVersion(QString gdbPath)
{
    QString versionStr;
    QProcess process;
    process.start(gdbPath,
        QStringList("--version"),
        QIODevice::ReadWrite | QIODevice::Text);
    if(!process.waitForFinished(2000))
    {
        errorMsg("Failed to launch gdb to get version: %s", stringToCStr(process.exitCode()));
    }
    else
    {
        QStringList versionStrList = QString(process.readAllStandardOutput()).split('\n');
        if(versionStrList.size() >= 1)
            versionStr = versionStrList[0];
    }
    return versionStr;
}


    
