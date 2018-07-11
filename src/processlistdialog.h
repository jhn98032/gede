/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__PROCESSLISTDIALOG_H
#define FILE__PROCESSLISTDIALOG_H

#include <QDialog>
#include <QList>

#include "settings.h"
#include "ui_processlistdialog.h"


class ProcessInfo
{
public:
    QString cmdline;
    int pid;
    int uid;

    QString getCmdline() { return cmdline; };
    int getPid() { return pid;};
    int getUid() { return uid; };
};

QList<ProcessInfo> getProcessListByUser(int ownerUid = -1);
QList<ProcessInfo> getProcessListAllUsers();
QList<ProcessInfo> getProcessListThisUser();



class ProcessListDialog : public QDialog
{
    Q_OBJECT

public:

    ProcessListDialog(QWidget *parent = NULL);

    void selectPid(int pid);
    int getSelectedProcess();

private slots:

    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    void fillInList();

private:

    Ui_ProcessListDialog m_ui;
    
};

#endif // FILE__PROCESSLISTDIALOG_H

