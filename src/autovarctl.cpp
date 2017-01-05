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
        if(!m_autoVarDispInfo.contains(varName))
        {
            VarCtl::DispInfo dispInfo;
            dispInfo.dispFormat = VarCtl::DISP_DEC;
            dispInfo.isExpanded = false;
            m_autoVarDispInfo[varName] = dispInfo;
        }
        
        
        VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varName];


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
        else
        {
            dispInfo.dispFormat = VarCtl::DISP_HEX;
        }
        

        CoreVar *var = getVar(*item);
        if(var)
        {
            QString valueText = getDisplayString(var, varName);
        
            item->setText(1, valueText);
        }

    
    }
}

void AutoVarCtl::ICore_onLocalVarReset()
{
    QTreeWidget *autoWidget = m_autoWidget;

    autoWidget->clear();
}


void AutoVarCtl::ICore_onLocalVarChanged(CoreVar *varValue)
{
    QString name = varValue->getName();

    assert(varValue != NULL);


    createTreeWidgetItem(NULL, &m_autoVarDispInfo, name, varValue);
   
}


/**
 * @brief Returns the value text to show for an item.
 */
QString AutoVarCtl::getDisplayString(CoreVar *var, QString fullPath)
{
    QString displayValue;
    if(m_autoVarDispInfo.contains(fullPath))
    {
        VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[fullPath];

        switch(dispInfo.dispFormat)
        {
            default:
            case DISP_NATIVE:
                displayValue = var->getData(CoreVar::FMT_NATIVE);break;
            case DISP_DEC:
                displayValue = var->getData(CoreVar::FMT_DEC);break;
            case DISP_BIN:
                displayValue = var->getData(CoreVar::FMT_BIN);break;
            case DISP_HEX:
                displayValue = var->getData(CoreVar::FMT_HEX);break;
            case DISP_CHAR:
                displayValue = var->getData(CoreVar::FMT_CHAR);break;
        }

    }
    else
    {
        displayValue = var->getData(CoreVar::FMT_NATIVE);

        VarCtl::DispInfo dispInfo;
        dispInfo.dispFormat = DISP_NATIVE;
        dispInfo.isExpanded = false;
        m_autoVarDispInfo[fullPath] = dispInfo;
    }
    return displayValue;
}


/**
 * @brief Create a item and adds it to the tree widget.
 */        
void AutoVarCtl::createTreeWidgetItem(
                    QTreeWidgetItem *parentItem,
                    VarCtl::DispInfoMap *map,
                    QString fullPath,
                    CoreVar *varValue)
{
    QString name = varValue->getName();
    QTreeWidget *autoWidget = m_autoWidget;

    
    //
    
    //
    QString displayValue = getDisplayString(varValue, fullPath);
    
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
        CoreVar *child = varValue->getChild(i);

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
 * @brief Get the variable associated with the item.
 */
CoreVar *AutoVarCtl::getVar(QTreeWidgetItem &item)
{
    QTreeWidgetItem *parentTreeItem = item.parent();
    if(parentTreeItem == NULL)
    {
        Core &core = Core::getInstance();
        QVector <CoreVar*> list = core.getLocalVars();
        for(int j = 0;j < list.size();j++)
        {
            CoreVar* var = list[j];
            if(var->getName() == item.text(0))
                return var;
        }
    }
    else
    {
        CoreVar *parentVar = getVar(*parentTreeItem);
        for(int j = 0;j < parentVar->getChildCount();j++)
        {
            CoreVar* var = parentVar->getChild(j);
            if(var->getName() == item.text(0))
                return var;
        }
    }
    return NULL;
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
    
        QString varPath = getTreeWidgetItemPath(item);
        CoreVar *var = getVar(*item);
        if(var != NULL)
        {
            if(!m_autoVarDispInfo.contains(varPath))
            {
                VarCtl::DispInfo dispInfo;
                dispInfo.dispFormat = DISP_HEX;
                dispInfo.isExpanded = false;
                m_autoVarDispInfo[varPath] = dispInfo;
            }
        
            VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varPath];

            dispInfo.dispFormat = fmt;

            QString valueText = getDisplayString(var, varPath);
            
            item->setText(1, valueText);
        }
    }

}




