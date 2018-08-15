
//#define ENABLE_DEBUGMSG

/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "autovarctl.h"

#include "log.h"
#include "util.h"
#include "core.h"
#include "memorydialog.h"
#include "autosignalblocker.h"

enum
{
    COLUMN_NAME = 0,
    COLUMN_VALUE = 1,
    COLUMN_TYPE = 2
};
#define DATA_COLUMN         (COLUMN_NAME) 


AutoVarCtl::AutoVarCtl()
    : m_autoWidget(0)
{


}


void AutoVarCtl::ICore_onStateChanged(ICore::TargetState state)
{
    if(state == ICore::TARGET_STARTING || state == ICore::TARGET_RUNNING)
        m_autoWidget->setEnabled(false);
    else
        m_autoWidget->setEnabled(true);
    
}
    
QString AutoVarCtl::getTreeWidgetItemPath(QTreeWidgetItem *item)
{
    QTreeWidgetItem *parent = item->parent();
    if(parent)
        return getTreeWidgetItemPath(parent) + "/" + item->text(COLUMN_NAME);
    else
        return item->text(COLUMN_NAME);
}


void AutoVarCtl::setWidget(QTreeWidget *autoWidget)
{
    m_autoWidget = autoWidget;

        //
    m_autoWidget->setColumnCount(3);
    m_autoWidget->setColumnWidth(COLUMN_NAME, 120);
    m_autoWidget->setColumnWidth(COLUMN_VALUE, 140);
    QStringList names;
    names += "Name";
    names += "Value";
    names += "Type";
    m_autoWidget->setHeaderLabels(names);
    connect(m_autoWidget, SIGNAL(itemChanged ( QTreeWidgetItem * ,int)), this, 
                            SLOT(onAutoWidgetCurrentItemChanged(QTreeWidgetItem * ,int)));
    connect(m_autoWidget, SIGNAL(itemDoubleClicked ( QTreeWidgetItem * , int  )), this,
                            SLOT(onAutoWidgetItemDoubleClicked(QTreeWidgetItem *, int )));
    connect(m_autoWidget, SIGNAL(itemExpanded ( QTreeWidgetItem * )), this,
                            SLOT(onAutoWidgetItemExpanded(QTreeWidgetItem * )));
    connect(m_autoWidget, SIGNAL(itemCollapsed ( QTreeWidgetItem *  )), this,
                            SLOT(onAutoWidgetItemCollapsed(QTreeWidgetItem * )));

    m_autoWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_autoWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onContextMenu(const QPoint&)));




}


/**
 * @brief Called when the user right clicks anywhere.
 */
void AutoVarCtl::onContextMenu ( const QPoint &pos)
{
    QAction *action = NULL;

    m_popupMenu.clear();

           
    // Add menu entries
    m_popupMenu.addSeparator();
    
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


/**
 * @brief Returns the address of the data the variable points to or the actual variable.
 */
quint64 AutoVarCtl::getAddress(VarWatch &w)
{
    Core &core = Core::getInstance();
    if(w.hasPointerAddress())
    {
        return w.getPointerAddress();
    }
    return core.getAddress(w);
}

void AutoVarCtl::onShowMemory()
{
    QList<QTreeWidgetItem *> selectedItems = m_autoWidget->selectedItems();
    Core &core = Core::getInstance();
    if(!selectedItems.empty())
    {
        QTreeWidgetItem *item = selectedItems[0];

        QString watchId = getWatchId(item);
        VarWatch *watch = NULL;
        if(!watchId.isEmpty())
            watch = core.getVarWatchInfo(watchId);
        if(watch)
        {
            
            MemoryDialog dlg;
            dlg.setConfig(&m_cfg);
            dlg.setStartAddress(getAddress(*watch));
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
    debugMsg("%s(varPath:'%s')", __func__, stringToCStr(varPath));
    if(m_autoVarDispInfo.contains(varPath))
    {
        VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varPath];
        dispInfo.isExpanded = true;

    }
    else
    assert(0);

    Core &core = Core::getInstance();
    //QTreeWidget *varWidget = m_autoWidget;

    // Get watchid of the item
    QString watchId = getWatchId(item);
    

    // Get the children
    //if(!watchId.isEmpty())
    core.gdbExpandVarWatchChildren(watchId);
    

}


void AutoVarCtl::onAutoWidgetItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QTreeWidget *varWidget = m_autoWidget;

    if(column == COLUMN_VALUE)
        varWidget->editItem(item,column);
    else
    {
        AutoSignalBlocker autoBlocker(m_autoWidget);

        QString varName = item->text(COLUMN_NAME);
        QString watchId = getWatchId(item);
        QString varPath = getTreeWidgetItemPath(item);
             
        if(m_autoVarDispInfo.contains(varPath))
        {
            VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varPath];
            {
                
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

                QString valueText = getDisplayString(watchId, varPath);
                
                item->setText(1, valueText);
            }
        }
    }
}


void AutoVarCtl::ICore_onWatchVarChanged(VarWatch &watch)
{
    Q_UNUSED(watch);
}

QString AutoVarCtl::getWatchId(QTreeWidgetItem* item)
{
    return item->data(DATA_COLUMN, Qt::UserRole).toString();
}




void AutoVarCtl::ICore_onWatchVarChildAdded(VarWatch &watch)
{
    Core &core = Core::getInstance();
    QTreeWidget *varWidget = m_autoWidget;
    QString watchId = watch.getWatchId();
    QString name = watch.getName();
    QString varType = watch.getVarType();
    bool hasChildren  = watch.hasChildren();
    bool inScope = watch.inScope();

    debugMsg("%s(name:'%s')",__func__, stringToCStr(name));


    AutoSignalBlocker autoBlocker(m_autoWidget);

    //
    QTreeWidgetItem * rootItem = varWidget->invisibleRootItem();
    QStringList watchIdParts = watchId.split('.');
    QString thisWatchId;


    for(int partIdx = 0; partIdx < watchIdParts.size();partIdx++)
    {
        // Get the watchid to look for
        if(thisWatchId != "")
            thisWatchId += ".";
        thisWatchId += watchIdParts[partIdx];

        // Look for the item with the specified watchId
        QTreeWidgetItem* foundItem = NULL;
        for(int i = 0;foundItem == NULL && i < rootItem->childCount();i++)
        {
            QTreeWidgetItem* item =  rootItem->child(i);
            QString itemKey = getWatchId(item);

            if(thisWatchId == itemKey)
            {
                foundItem = item;
            }
        }

        // This watch belonged to the WatchWidget?
        if(partIdx == 0 && foundItem == NULL)
        {
            return;
        }
        
        // Did not find one?
        QTreeWidgetItem *item;
        if(foundItem == NULL)
        {
            debugMsg("Adding '%s'", stringToCStr(name));

            // Create the item
            QStringList nameList;
            nameList += name;
            nameList += "";
            nameList += varType;
            item = new QTreeWidgetItem(nameList);
            item->setData(DATA_COLUMN, Qt::UserRole, thisWatchId);
            item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            rootItem->addChild(item);
            rootItem = item;


            if(hasChildren)
                rootItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

            
        
        }
        else
        {
            item = foundItem;
            rootItem = foundItem;
        }

        // The last part of the id?
        if(partIdx+1 == watchIdParts.size())
        {
            // Update the text
            QString varPath = getTreeWidgetItemPath(item);
            VarCtl::DispInfo dispInfo;
            if(m_autoVarDispInfo.contains(varPath) == false)
            {
                dispInfo.dispFormat = DISP_NATIVE;
                dispInfo.isExpanded = false;

                debugMsg("Adding '%s'", stringToCStr(varPath));


                m_autoVarDispInfo[varPath] = dispInfo;
            }
            else
            {
                dispInfo = m_autoVarDispInfo[varPath];
            }
            QString valueString = getDisplayString(watchId, varPath);
  
            bool enable = inScope;

            if(!enable)
                item->setDisabled(true);
            else
                item->setDisabled(false);
            

            item->setText(COLUMN_VALUE, valueString);

            // Color the text based on if the value is different
            VarCtl::DispInfo &newDispInfo = m_autoVarDispInfo[varPath];
            QBrush b;
            if(newDispInfo.lastData != valueString)
                b = QBrush(Qt::red);
            else
                b = QBrush(Qt::black);
            item->setForeground(COLUMN_VALUE,b);
            newDispInfo.lastData = valueString;


            if(dispInfo.isExpanded && hasChildren)
            {
                


                // Get the children
                core.gdbExpandVarWatchChildren(watchId);
                varWidget->expandItem(item);
            }
        }
    }

    
}



void AutoVarCtl::ICore_onLocalVarChanged(QStringList varNames)
{
    debugMsg("%s()", __func__);

    clear();

    for(int i = 0;i < varNames.size();i++)
        addNewWatch(varNames[i]);

}


/**
 * @brief Returns the value text to show for an item.
 */
QString AutoVarCtl::getDisplayString(QString watchId, QString varPath)
{
    QString displayValue;
    Core &core = Core::getInstance();

    //debugMsg("%s(watchId:'%s' varPath:'%s')", __func__, qPrintable(watchId), qPrintable(varPath));

    // Create value if not exist
    if(!m_autoVarDispInfo.contains(varPath))
    {
        VarCtl::DispInfo dispInfo;
        dispInfo.dispFormat = DISP_NATIVE;
        dispInfo.isExpanded = false;
        m_autoVarDispInfo[varPath] = dispInfo;
    }

    VarWatch *watch = NULL;
    if(!watchId.isEmpty())
        watch = core.getVarWatchInfo(watchId);
    if(watch)
    {
        VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varPath];

        switch(dispInfo.dispFormat)
        {
            default:
            case DISP_NATIVE:
                displayValue = watch->getValue(CoreVar::FMT_NATIVE);break;
            case DISP_DEC:
                displayValue = watch->getValue(CoreVar::FMT_DEC);break;
            case DISP_BIN:
                displayValue = watch->getValue(CoreVar::FMT_BIN);break;
            case DISP_HEX:
                displayValue = watch->getValue(CoreVar::FMT_HEX);break;
            case DISP_CHAR:
                displayValue = watch->getValue(CoreVar::FMT_CHAR);break;
        }

    }
    return displayValue;
}

void AutoVarCtl::clear()
{
    QTreeWidget *autoWidget = m_autoWidget;
    QTreeWidgetItem *rootItem = m_autoWidget->invisibleRootItem();

    debugMsg("%s()", __func__);

    // Get all items
    QList<QTreeWidgetItem *>items;
    for(int i =0;i < rootItem->childCount();i++)
    {
        QTreeWidgetItem *item = rootItem->child(i);

        items.push_back(item);
    }
    

    // Loop through the items
    while(!items.isEmpty())
    {
        QTreeWidgetItem *item = items.takeFirst();
    
        // Delete the item
        Core &core = Core::getInstance();
        QString watchId = getWatchId(item);
        if(watchId != "")
        {
            core.gdbRemoveVarWatch(watchId);
        }
    }

    autoWidget->clear();

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






void 
AutoVarCtl::onAutoWidgetCurrentItemChanged( QTreeWidgetItem * current, int column )
{
    //QTreeWidget *varWidget = m_autoWidget;
    Core &core = Core::getInstance();
    QString oldKey = getWatchId(current);
    
    if(column != COLUMN_VALUE)
        return;
        
    AutoSignalBlocker autoBlocker(m_autoWidget);

    //debugMsg("%s(key:'%s')", __func__, qPrintable(oldKey));
    
    
    VarWatch *watch = NULL;
    watch = core.getVarWatchInfo(oldKey);
    if(watch)
    {
        QString oldValue  = watch->getValue();
        QString varPath = getTreeWidgetItemPath(current);
        QString oldValueText = getDisplayString(oldKey, varPath);
        QString newValue = current->text(COLUMN_VALUE);

        if (oldValueText != newValue)
        {
            if(core.changeWatchVariable(oldKey, newValue))
            {
                current->setText(COLUMN_VALUE, oldValue);
            }
        }
    }


}




/**
 * @brief Change display format for the currently selected items.
 */
void AutoVarCtl::selectedChangeDisplayFormat(VarCtl::DispFormat fmt)
{
    AutoSignalBlocker autoBlocker(m_autoWidget);

    // Loop through the selected items.
    QList<QTreeWidgetItem *> items = m_autoWidget->selectedItems();
    for(int i =0;i < items.size();i++)
    {
        QTreeWidgetItem *item = items[i];
    
        QString varPath = getTreeWidgetItemPath(item);
        QString varName = item->text(COLUMN_NAME);
        QString watchId = getWatchId(item);
        
        
        if(m_autoVarDispInfo.contains(varPath))
        {
            VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varPath];
            {
                dispInfo.dispFormat = fmt;
           
                QString valueText = getDisplayString(watchId, varPath);

                item->setText(COLUMN_VALUE, valueText);
            }
        }
        else
        {
            debugMsg("Var path:%s not found", stringToCStr(varPath));
        }
    }

}






/**
 * @brief Adds a new watch item
 * @param varName    The expression to add as a watch.
 */
void AutoVarCtl::addNewWatch(QString varName)
{
    QString newName = varName;
    Core &core = Core::getInstance();
    
     //debugMsg("%s", stringToCStr(current->text(0)));
        VarWatch *watch = NULL;
        if(core.gdbAddVarWatch(newName, &watch) == 0)
        {
            QString watchId = watch->getWatchId();
            QString varType = watch->getVarType();

            bool hasChildren = watch->hasChildren();

    QTreeWidget *varWidget = m_autoWidget;

// Create a new dummy item
            QTreeWidgetItem *item;
            QStringList names;
            names += varName;
            item = new QTreeWidgetItem(names);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            varWidget->addTopLevelItem(item);


            QTreeWidgetItem *current = item;
            

            
            if(hasChildren)
                current->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

            QString varPath = getTreeWidgetItemPath(current);
            QString value  = getDisplayString(watchId, varPath);
            if(!m_autoVarDispInfo.contains(varPath))
            {
                VarCtl::DispInfo newDispInfo;
                newDispInfo.dispFormat = DISP_NATIVE;

                debugMsg("Adding '%s'", stringToCStr(varPath));
                m_autoVarDispInfo[varPath] = newDispInfo;
            }

            VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varPath];
                
        
            current->setData(DATA_COLUMN, Qt::UserRole, watchId);
            current->setText(COLUMN_VALUE, value);
            current->setText(COLUMN_TYPE, varType);

            // Set color based on if the value has changed
            QBrush b;
            if(dispInfo.lastData != value)
                b = QBrush(Qt::red);
            else
                b = QBrush(Qt::black);
            current->setForeground(COLUMN_VALUE,b);
            dispInfo.lastData = value;

            

    //
    if(m_autoVarDispInfo.contains(varPath))
    {
        if(m_autoVarDispInfo[varPath].isExpanded)
        {
            m_autoWidget->expandItem(current);
        }
    }
        }

}



void AutoVarCtl::onKeyPress(QKeyEvent *keyEvent)
{
    if(keyEvent->key() == Qt::Key_Return)
    {
        // Get the active unit
        QTreeWidgetItem * item = m_autoWidget->currentItem();
        if(item)
        {
            m_autoWidget->editItem(item,COLUMN_VALUE);
        }   
    }

}
    

    
