/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#define ENABLE_DEBUGMSG

#include "processlistdialog.h"

#include "version.h"
#include <QProcess>
#include "util.h"
#include "log.h"

#include <QList>
#include <QDir>
#include <QFileInfoList>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


QByteArray fileToContent(QString filename)
{
    QByteArray cnt;
    QFile f(filename);
    if(!f.open(QIODevice::ReadOnly))
    {
    }
    else
    {
        cnt = f.readAll();
    }
    return cnt;
}

QList<ProcessInfo> getProcessListByUser(int ownerUid)
{
    QList<ProcessInfo> lst;

    QDir dir("/proc");
    dir.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        QString fileName = fileInfo.fileName();
        if(fileName != ".." && fileName != ".")
        {
            QString procDirPath = "/proc/" + fileInfo.fileName();
            QFileInfo s(procDirPath + "/status");
            if(s.exists())
            {
                const int pid = fileInfo.fileName().toInt();
                
                struct stat buf;
                 if(stat(qPrintable(procDirPath), &buf) == 0)
                 {
                     if(buf.st_uid == (unsigned int)ownerUid || ownerUid == -1)
                     {
                         ProcessInfo prc;
                         prc.uid = buf.st_uid;
                         prc.pid = pid;

                        
                        prc.cmdline = QString(fileToContent(procDirPath + "/cmdline")).trimmed();

                        lst.append(prc);
                     }
                 }
      
            }
        }
    }
/*
     for(int u = 0;u < lst.size();u++)
     {
        ProcessInfo &prc = lst[u];
        debugMsg("[%d/%d] PID:%d UID:%d CMDLINE='%s'", u+1, lst.size(), prc.pid, prc.uid, qPrintable(prc.cmdline));
     }
*/
    return lst;
}



QList<ProcessInfo> getProcessListAllUsers()
{
    return getProcessListByUser(-1); 
}

QList<ProcessInfo> getProcessListThisUser()
{
    return getProcessListByUser(getuid());
}

//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------

enum
{
    COLUMN_PID = 0
    ,COLUMN_UID
    ,COLUMN_CMDLINE
};

ProcessListDialog::ProcessListDialog(QWidget *parent)
    : QDialog(parent)
{
    
    m_ui.setupUi(this);

    m_ui.treeWidget->setColumnCount(3);
    m_ui.treeWidget->setColumnWidth(COLUMN_PID, 80);
    m_ui.treeWidget->setColumnWidth(COLUMN_UID, 80);
  
    QStringList names;
    names += "PID";
    names += "UID";
    names += "Cmdline";
    m_ui.treeWidget->setHeaderLabels(names);

    fillInList();

    connect(m_ui.treeWidget,SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int )),this,SLOT(onItemDoubleClicked(QTreeWidgetItem *, int )));

}


void ProcessListDialog::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(item);
    Q_UNUSED(column);
    return accept();
}

/**
 * @brief Selects a specific PID in the PID list.
 */
void ProcessListDialog::selectPid(int pid)
{
    QTreeWidget *treeWidget = m_ui.treeWidget;
    QString pidText = QString::number(pid);
    QList<QTreeWidgetItem *> foundItem = treeWidget->findItems ( pidText , Qt::MatchExactly, COLUMN_PID );
    if(!foundItem.empty())
    {
        treeWidget->setCurrentItem( foundItem[0]);
    }
}


/**
 * @brief Returns the selected process PID
*/
int ProcessListDialog::getSelectedProcess()
{
    QTreeWidget *treeWidget = m_ui.treeWidget;
    int pid = -1;

    // No items?
    if(treeWidget->topLevelItemCount() == -1)
        return pid;
    
    // Get the selected ones
    QList<QTreeWidgetItem *> selectedItems = treeWidget->selectedItems();
    if(selectedItems.size() == 0)
    {
        selectedItems.append(treeWidget->topLevelItem(0));
    }
    
    if(!selectedItems.empty())
        pid = selectedItems[0]->data(0, Qt::UserRole).toInt();
                    
    return pid;
}

/**
 * @brief Fill in the list of processes.
 */
void ProcessListDialog::fillInList()
{
    m_ui.treeWidget->clear();
    QList<ProcessInfo> list = getProcessListThisUser();
    for(int pIdx = 0;pIdx < list.size();pIdx++)
    {
        ProcessInfo &prc = list[pIdx];
        QTreeWidgetItem *item;
        QStringList names;
        names += QString::number(prc.pid);
        names += QString::number(prc.uid);
        names += prc.getCmdline();
        item = new QTreeWidgetItem(names);
        item->setData(0, Qt::UserRole, prc.getPid());
        m_ui.treeWidget->addTopLevelItem(item);

    }

    m_ui.treeWidget->resizeColumnToContents(COLUMN_PID);
    m_ui.treeWidget->resizeColumnToContents(COLUMN_UID);
    m_ui.treeWidget->resizeColumnToContents(COLUMN_CMDLINE);
  
}


