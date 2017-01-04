/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "autovarctl.h"

#include "mainwindow.h"
#include "log.h"
#include "util.h"
#include "memorydialog.h"

enum
{
    COLUMN_NAME = 0,
    COLUMN_VALUE = 1,
};


AutoVarCtl::AutoVarCtl()
    : m_autoWidget(0)
{

}

QString AutoVarCtl::getTreeWidgetItemPath(QTreeWidgetItem *item)
{
    QTreeWidgetItem *parent = item->parent();
    if(parent)
        return getTreeWidgetItemPath(parent) + "/" + item->text(0);
    else
        return item->text(0);
}


void AutoVarCtl::setWidget(QTreeWidget *autoWidget)
{
    m_autoWidget = autoWidget;

    autoWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_autoWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onContextMenu(const QPoint&)));

    //
    m_autoWidget->setColumnCount(2);
    m_autoWidget->setColumnWidth(0, 120);
    QStringList names;
    names.clear();
    names += "Name";
    names += "Value";
    m_autoWidget->setHeaderLabels(names);
    connect(m_autoWidget, SIGNAL(itemDoubleClicked ( QTreeWidgetItem * , int  )), this,
                            SLOT(onAutoWidgetItemDoubleClicked(QTreeWidgetItem *, int )));
    connect(m_autoWidget, SIGNAL(itemExpanded ( QTreeWidgetItem * )), this,
                            SLOT(onAutoWidgetItemExpanded(QTreeWidgetItem * )));
    connect(m_autoWidget, SIGNAL(itemCollapsed ( QTreeWidgetItem *  )), this,
                            SLOT(onAutoWidgetItemCollapsed(QTreeWidgetItem * )));


}

void AutoVarCtl::onContextMenu ( const QPoint &pos)
{
    QAction *action = NULL;

    m_popupMenu.clear();
    
    // Add 'open'
    action = m_popupMenu.addAction("Display as dec");
    action->setData(0);
    connect(action, SIGNAL(triggered()), this, SLOT(onDisplayAsDec()));
    action = m_popupMenu.addAction("Display as hex");
    action->setData(0);
    connect(action, SIGNAL(triggered()), this, SLOT(onDisplayAsHex()));
    action = m_popupMenu.addAction("Display as bin");
    action->setData(0);
    connect(action, SIGNAL(triggered()), this, SLOT(onDisplayAsBin()));
    action = m_popupMenu.addAction("Display as char");
    action->setData(0);
    connect(action, SIGNAL(triggered()), this, SLOT(onDisplayAsChar()));
    m_popupMenu.addSeparator();
    action = m_popupMenu.addAction("Show memory");
    action->setData(0);
    connect(action, SIGNAL(triggered()), this, SLOT(onShowMemory()));

        
    m_popupMenu.popup(m_autoWidget->mapToGlobal(pos));
}

void AutoVarCtl::onShowMemory()
{
    QList<QTreeWidgetItem *> selectedItems = m_autoWidget->selectedItems();
    if(!selectedItems.empty())
    {
        QTreeWidgetItem *item = selectedItems[0];

        long long addr = item->data(1, Qt::UserRole).toLongLong(0);
        debugMsg("%s addr:%llx\n", stringToCStr(item->text(0)), addr);
        if(addr != 0)
        {
            
            MemoryDialog dlg;
            dlg.setConfig(&m_cfg);
            dlg.setStartAddress(addr);
            dlg.exec();
        }
    }
        
}

void AutoVarCtl::onAutoWidgetItemCollapsed(QTreeWidgetItem *item)
{
    QString varPath = getTreeWidgetItemPath(item);
    if(m_autoVarDispInfo.contains(varPath))
    {
        VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varPath];
        dispInfo.isExpanded = false;
    }

}

void AutoVarCtl::onAutoWidgetItemExpanded(QTreeWidgetItem *item)
{
    QString varPath = getTreeWidgetItemPath(item);
    if(m_autoVarDispInfo.contains(varPath))
    {
        VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varPath];
        dispInfo.isExpanded = true;

    }
}




void AutoVarCtl::onAutoWidgetItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if(column == 0)
    {
    }
    else if(column == 1)
    {
        QString varName = getTreeWidgetItemPath(item);
        if(m_autoVarDispInfo.contains(varName))
        {
            VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varName];
            if(dispInfo.orgFormat == VarCtl::DISP_DEC)
            {
                QString valStr = dispInfo.orgValue;

                if(dispInfo.dispFormat == VarCtl::DISP_DEC)
                {
                    dispInfo.dispFormat = VarCtl::DISP_HEX;
                }
                else if(dispInfo.dispFormat == VarCtl::DISP_HEX)
                {
                    dispInfo.dispFormat = VarCtl::DISP_BIN;
                }
                else if(dispInfo.dispFormat == VarCtl::DISP_BIN)
                {
                    dispInfo.dispFormat = VarCtl::DISP_CHAR;
                }
                else if(dispInfo.dispFormat == VarCtl::DISP_CHAR)
                {
                    dispInfo.dispFormat = VarCtl::DISP_DEC;
                }

                QString valueText = VarCtl::valueDisplay(valStr, dispInfo.dispFormat);

                item->setText(1, valueText);
            }
        }
    }
}

void AutoVarCtl::ICore_onLocalVarReset()
{
    QTreeWidget *autoWidget = m_autoWidget;

    autoWidget->clear();
}


void AutoVarCtl::ICore_onLocalVarChanged(CoreVarValue *varValue)
{
    QString name = varValue->getName();

    assert(varValue != NULL);


    createTreeWidgetItem(NULL, &m_autoVarDispInfo, name, varValue);
   
}






/**
 * @brief Create a item and adds it to the tree widget.
 */        
void AutoVarCtl::createTreeWidgetItem(
                    QTreeWidgetItem *parentItem,
                    VarCtl::DispInfoMap *map,
                    QString fullPath,
                    CoreVarValue *varValue)
{
    QString name = varValue->getName();
    QTreeWidget *autoWidget = m_autoWidget;

    // Get the text to display
    QString value = varValue->getData();

    VarCtl::DispFormat orgFormat = VarCtl::findVarType(value);
    QString displayValue = value;

    //
    if(map->contains(fullPath))
    {
        VarCtl::DispInfo &dispInfo = (*map)[fullPath];
        dispInfo.orgValue = value;

        // Update the variable value
        if(orgFormat == VarCtl::DISP_DEC && dispInfo.dispFormat != VarCtl::DISP_NATIVE)
        {
            displayValue = VarCtl::valueDisplay(value, dispInfo.dispFormat);
        }
    }
    else
    {
        VarCtl::DispInfo dispInfo;
        dispInfo.orgValue = value;
        dispInfo.orgFormat = orgFormat;
        dispInfo.dispFormat = dispInfo.orgFormat;
        dispInfo.isExpanded = false;
        (*map)[fullPath] = dispInfo;
    }

    //
    QStringList names;
    names.clear();
    names += name;
    names += displayValue;
    QTreeWidgetItem *item;
    item = new QTreeWidgetItem(names);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    item->setData(1, Qt::UserRole, QVariant((qlonglong)varValue->getAddress()));



    if(parentItem)
        parentItem->addChild(item);
    else
        autoWidget->insertTopLevelItem(0, item);
    

    for(int i = 0;i < varValue->getChildCount();i++)
    {
        CoreVarValue *child = varValue->getChild(i);

        QString varPath = fullPath + "/" + child->getName();

        createTreeWidgetItem(
                item,
                map,
                varPath,
                child);
    }

    // Expand it?
    if(m_autoVarDispInfo.contains(fullPath))
    {
        VarCtl::DispInfo &dispInfo = (*map)[fullPath];

        if(dispInfo.isExpanded)
        {
            autoWidget->expandItem(item);
        }
    }
    else
    {
        // Add it to the dispinfomap
        VarCtl::DispInfo dispInfo;
        dispInfo.isExpanded = false;
        (*map)[fullPath] = dispInfo;
    }

}


void AutoVarCtl::setConfig(Settings *cfg)
{
    m_cfg = *cfg;
}


void AutoVarCtl::onDisplayAsDec()
{
    selectedChangeDisplayFormat(VarCtl::DISP_DEC);
}

void AutoVarCtl::onDisplayAsHex()
{
    selectedChangeDisplayFormat(VarCtl::DISP_HEX);
}

void AutoVarCtl::onDisplayAsBin()
{
    selectedChangeDisplayFormat(VarCtl::DISP_BIN);
}

void AutoVarCtl::onDisplayAsChar()
{
    selectedChangeDisplayFormat(VarCtl::DISP_CHAR);
}


/**
 * @brief Change display format for the currently selected items.
 */
void AutoVarCtl::selectedChangeDisplayFormat(VarCtl::DispFormat fmt)
{
    // Loop through the selected items.
    QList<QTreeWidgetItem *> items = m_autoWidget->selectedItems();
    for(int i =0;i < items.size();i++)
    {
        QTreeWidgetItem *item = items[i];
    
        QString varName = getTreeWidgetItemPath(item);
        if(m_autoVarDispInfo.contains(varName))
        {
            VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varName];
            if(dispInfo.orgFormat == VarCtl::DISP_DEC)
            {
                QString valStr = dispInfo.orgValue;
                
                dispInfo.dispFormat = fmt;
                
                QString valueText = VarCtl::valueDisplay(valStr, dispInfo.dispFormat);

                item->setText(COLUMN_VALUE, valueText);
            }
        }
    }

}




