/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

//#define ENABLE_DEBUGMSG

#include "processlistdialog.h"

#include <QList>
#include <QDir>
#include <QFileInfoList>
#include <QProcess>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "version.h"
#include "util.h"
#include "log.h"



enum
{
    COLUMN_PID = 0
    ,COLUMN_UID
    ,COLUMN_TIME
    ,COLUMN_CMDLINE
};


ProcessListWidgetItem::ProcessListWidgetItem()
{

}
ProcessListWidgetItem::ProcessListWidgetItem( const ProcessInfo &prc)
 : m_prc(prc)
{
    setText(COLUMN_PID, QString::number(prc.pid));
    setText(COLUMN_UID, QString::number(prc.uid));
    int secs = prc.mtime.secsTo(QDateTime::currentDateTime());
    QString dtStr;
    dtStr.sprintf("%02d:%02d", secs/3600, (secs/60)%60);
    setText(COLUMN_TIME, dtStr);
    setText(COLUMN_CMDLINE, prc.getCmdline());
        
}
ProcessListWidgetItem::~ProcessListWidgetItem()
{
}

bool ProcessListWidgetItem::operator<(const QTreeWidgetItem &other) const
{
    QTreeWidget * tree = treeWidget ();
    const ProcessListWidgetItem* item1 = this;
    const ProcessListWidgetItem* item2 = (ProcessListWidgetItem*)&other;
    int column = tree ? tree->sortColumn() : 0;
    if(column == COLUMN_UID)
        return item1->m_prc.mtime < item2->m_prc.mtime;
    else
        return text(column) < other.text(column);
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
                         prc.mtime.setTime_t(buf.st_mtim.tv_sec);
                         prc.m_exePath = procDirPath + "/exe";
                         QFile f(prc.m_exePath);
                         prc.m_exePath = f.symLinkTarget();
                         
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


ProcessListDialog::ProcessListDialog(QWidget *parent)
    : QDialog(parent)
{
    
    m_ui.setupUi(this);

    m_ui.treeWidget->setColumnCount(4);
    m_ui.treeWidget->setColumnWidth(COLUMN_PID, 80);
    m_ui.treeWidget->setColumnWidth(COLUMN_UID, 80);
    m_ui.treeWidget->setColumnWidth(COLUMN_TIME, 80);
  
    QStringList names;
    names += "PID";
    names += "UID";
    names += "Time";
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
* @brief Looks for info about a process
*/
ProcessInfo *ProcessListDialog::findProcessByPid(int pid)
{
    // Display the list
    for(int pIdx = 0;pIdx < m_processList.size();pIdx++)
    {
        ProcessInfo &prc = m_processList[pIdx];
        if(prc.getPid() == pid)
            return &prc;
    }
    return NULL;
}



/**
 * @brief Returns the selected process PID
*/
int ProcessListDialog::getSelectedPid()
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
* @brief Returns the selected process.
*/
ProcessInfo ProcessListDialog::getSelectedProcess()
{
    ProcessInfo *info = findProcessByPid(getSelectedPid());
    if(info)
        return *info;
    return ProcessInfo();
}

/**
 * @brief Fill in the list of processes.
 */
void ProcessListDialog::fillInList()
{
    m_ui.treeWidget->clear();

    // Get a list of processes
    m_processList = getProcessListThisUser();

    // Display the list
    for(int pIdx = 0;pIdx < m_processList.size();pIdx++)
    {
        ProcessInfo &prc = m_processList[pIdx];
        ProcessListWidgetItem *item;
        item = new ProcessListWidgetItem(prc);
        item->setData(0, Qt::UserRole, prc.getPid());
        m_ui.treeWidget->addTopLevelItem(item);

    }

    // Set correct column width
    m_ui.treeWidget->resizeColumnToContents(COLUMN_PID);
    m_ui.treeWidget->resizeColumnToContents(COLUMN_UID);
    m_ui.treeWidget->resizeColumnToContents(COLUMN_TIME);
    m_ui.treeWidget->resizeColumnToContents(COLUMN_CMDLINE);

    // Sort list
    m_ui.treeWidget->sortItems(COLUMN_TIME, Qt::AscendingOrder);
}



