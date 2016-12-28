/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
 
#include "watchvarctl.h"

#include "log.h"
#include "util.h"
#include "core.h"

enum
{
    COLUMN_NAME = 0,
    COLUMN_VALUE = 1,
    COLUMN_TYPE = 2
};
#define DATA_COLUMN         (COLUMN_NAME) 

class AutoSignalBlocker
{
public:
    bool m_signalBlocked;
    QObject *m_obj;
    AutoSignalBlocker(QObject *obj)
        :m_obj(obj)
    {
         m_signalBlocked = m_obj->blockSignals(true);
    };
    virtual ~AutoSignalBlocker()
    {
         m_obj->blockSignals(m_signalBlocked);
    }

    private:
        AutoSignalBlocker(){};
    
};

WatchVarCtl::WatchVarCtl()
{


}

void WatchVarCtl::setWidget(QTreeWidget *varWidget)
{
    m_varWidget = varWidget;

        //
    m_varWidget->setColumnCount(3);
    m_varWidget->setColumnWidth(0, 120);
    QStringList names;
    names += "Name";
    names += "Value";
    names += "Type";
    m_varWidget->setHeaderLabels(names);
    connect(m_varWidget, SIGNAL(itemChanged(QTreeWidgetItem * ,int)), this, SLOT(onWatchWidgetCurrentItemChanged(QTreeWidgetItem * ,int)));
    connect(m_varWidget, SIGNAL(itemDoubleClicked( QTreeWidgetItem * , int  )), this, SLOT(onWatchWidgetItemDoubleClicked(QTreeWidgetItem *, int )));
    connect(m_varWidget, SIGNAL(itemExpanded( QTreeWidgetItem * )), this, SLOT(onWatchWidgetItemExpanded(QTreeWidgetItem * )));
    connect(m_varWidget, SIGNAL(itemCollapsed( QTreeWidgetItem *)), this, SLOT(onWatchWidgetItemCollapsed(QTreeWidgetItem *)));

    m_varWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_varWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onContextMenu(const QPoint&)));



    fillInVars();


}



void WatchVarCtl::ICore_onWatchVarChanged(VarWatch &watch)
{
    QTreeWidget *varWidget = m_varWidget;
    Core &core = Core::getInstance();

    AutoSignalBlocker autoBlocker(m_varWidget);

    QStringList watchIdParts = watch.getWatchId().split('.');

    QTreeWidgetItem* rootItem = varWidget->invisibleRootItem();
    VarWatch *rootWatch = core.getVarWatchInfo(watchIdParts[0]);
    assert(rootWatch != NULL);
    if(rootWatch)
        sync(rootItem, *rootWatch);
}

QString WatchVarCtl::getWatchId(QTreeWidgetItem* item)
{
    return item->data(DATA_COLUMN, Qt::UserRole).toString();
}


void WatchVarCtl::sync(QTreeWidgetItem* parentItem, VarWatch &watch)
{
    Core &core = Core::getInstance();
    QString watchId = watch.getWatchId();
    QString name = watch.getName();
    QString varType = watch.getVarType();
    QString valueString = watch.m_varValue;
    bool inScope = watch.inScope();

        
    // Look for the item with the specified watchId
    QTreeWidgetItem* foundTreeItem = NULL;
    for(int i = 0;foundTreeItem == NULL && i < parentItem->childCount();i++)
    {
        QTreeWidgetItem* treeItem =  parentItem->child(i);
        QString itemKey = getWatchId(treeItem);
        if(watchId == itemKey)
        {
            foundTreeItem = treeItem;
        }
    }

    // Add if we did not find one
    QTreeWidgetItem *treeItem = foundTreeItem;
    if(foundTreeItem == NULL)
    {
        debugMsg("Adding %s=%s", stringToCStr(name), stringToCStr(valueString));

        // Create the item
        QStringList nameList;
        nameList += name;
        nameList += valueString;
        nameList += varType;
        treeItem = new QTreeWidgetItem(nameList);
        treeItem->setData(DATA_COLUMN, Qt::UserRole, watchId);
        treeItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        parentItem->addChild(treeItem);
        
        if(watch.hasChildren())
            treeItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

    }
    else
    {
        debugMsg("Found %s='%s'", stringToCStr(name), stringToCStr(valueString));
    }

    // Remove treeitems that does not exist anymore
    QList <VarWatch*> watchList = core.getWatchChildren(watch);
    for(int i = 0;i < treeItem->childCount();i++)
    {
        QTreeWidgetItem* childTreeItem =  treeItem->child(i);
        QString childItemKey = getWatchId(childTreeItem);
        
        bool found = false;
        for(int j = 0;j < watchList.size();j++)
        {
            VarWatch* watchChild = watchList[j];
            if(watchChild->getWatchId() == childItemKey)
            {
                found = true;
                watchList.removeAt(j--);
            }
        }

        debugMsg("child '%s' %s", stringToCStr(childItemKey), found ? "found" : "not found");
            
        if(found == false)
        {
            debugMsg("removing watchId:'%s'", stringToCStr(watchId));
            if(m_watchVarDispInfo.contains(childItemKey))
            {
                debugMsg("removing watchId:'%s' from dispinfo", stringToCStr(watchId));

                m_watchVarDispInfo.remove(childItemKey);
            
            }
            treeItem->removeChild(childTreeItem);
            i = 0;
        }
    }

    // Update the text
    if(m_watchVarDispInfo.contains(watchId))
    {
        VarCtl::DispInfo &dispInfo = m_watchVarDispInfo[watchId];
        dispInfo.orgValue = valueString;

        

        VarCtl::DispFormat orgFormat = VarCtl::findVarType(valueString);

        dispInfo.orgFormat = orgFormat;
        
        // Update the variable value
        if(orgFormat == VarCtl::DISP_DEC)
        {
            valueString = VarCtl::valueDisplay(valueString.toLongLong(0,0), dispInfo.dispFormat);
        }
    }
    if(!inScope)
        treeItem->setDisabled(true);
    else
        treeItem->setDisabled(false);

    // Add display info
    if(m_watchVarDispInfo.contains(watchId) == false)
    {
                VarCtl::DispInfo dispInfo;
            dispInfo.orgValue = valueString;
            dispInfo.orgFormat = VarCtl::findVarType(valueString);
            dispInfo.dispFormat = dispInfo.orgFormat;
            m_watchVarDispInfo[watchId] = dispInfo;
    }

    treeItem->setText(COLUMN_VALUE, valueString);
    treeItem->setText(COLUMN_TYPE, varType);
    if(watch.hasChildren())
        treeItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    else
        treeItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);


    QList <VarWatch*> watchList2 = core.getWatchChildren(watch);
    for(int j = 0;j < watchList2.size();j++)
    {
        VarWatch* childWatch = watchList2[j];
        sync(treeItem, *childWatch);
    }
}


void WatchVarCtl::ICore_onWatchVarChildAdded(VarWatch &watch)
{
    Core &core = Core::getInstance();
    QTreeWidget *varWidget = m_varWidget;
    QString watchId = watch.getWatchId();
    QString name = watch.getName();
    QString varType = watch.getVarType();
    
    QString valueString = watch.m_varValue;
    bool hasChildren  = watch.hasChildren();
    bool inScope = watch.inScope();

    AutoSignalBlocker autoBlocker(m_varWidget);

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
            QString itemKey = item->data(DATA_COLUMN, Qt::UserRole).toString();

            if(thisWatchId == itemKey)
            {
                foundItem = item;
            }
        }

        // Did not find one?
        QTreeWidgetItem *item;
        if(foundItem == NULL)
        {
            debugMsg("Adding %s=%s", stringToCStr(name), stringToCStr(valueString));

            // Create the item
            QStringList nameList;
            nameList += name;
            nameList += valueString;
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
            if(m_watchVarDispInfo.contains(thisWatchId) == false)
            {
                VarCtl::DispInfo dispInfo;
                dispInfo.orgValue = valueString;
                dispInfo.orgFormat = VarCtl::findVarType(valueString);
                dispInfo.dispFormat = dispInfo.orgFormat;
                m_watchVarDispInfo[watchId] = dispInfo;
            }
            else
            {
                VarCtl::DispInfo &dispInfo = m_watchVarDispInfo[thisWatchId];
                dispInfo.orgValue = valueString;

                VarCtl::DispFormat orgFormat = VarCtl::findVarType(valueString);

                dispInfo.orgFormat = orgFormat;
                
                // Update the variable value
                if(orgFormat == VarCtl::DISP_DEC)
                {
                    valueString = VarCtl::valueDisplay(valueString.toLongLong(0,0), dispInfo.dispFormat);
                }
            }

            bool enable = inScope;

                if(!enable)
                    item->setDisabled(true);
                else
                    item->setDisabled(false);
                

            item->setText(COLUMN_VALUE, valueString);
        }
    }
}


/**
 * @brief Change display format for the currently selected items.
 */
void WatchVarCtl::selectedChangeDisplayFormat(VarCtl::DispFormat fmt)
{
    AutoSignalBlocker autoBlocker(m_varWidget);

        

    // Loop through the selected items.
    QList<QTreeWidgetItem *> items = m_varWidget->selectedItems();
    for(int i =0;i < items.size();i++)
    {
        QTreeWidgetItem *item = items[i];
    
        QString varName = item->text(COLUMN_NAME);
        QString watchId = item->data(DATA_COLUMN, Qt::UserRole).toString();

        
        if(m_watchVarDispInfo.contains(watchId))
        {
            VarCtl::DispInfo &dispInfo = m_watchVarDispInfo[watchId];
            if(dispInfo.orgFormat == VarCtl::DISP_DEC)
            {
                long long val = dispInfo.orgValue.toLongLong(0,0);

                dispInfo.dispFormat = fmt;
                
                QString valueText = VarCtl::valueDisplay(val, dispInfo.dispFormat);

                item->setText(COLUMN_VALUE, valueText);
            }
        }
        else
        {
            debugMsg("Watch with watchId:%s not found", stringToCStr(watchId));
        }
    }

}

void WatchVarCtl::onDisplayAsDec()
{
    selectedChangeDisplayFormat(VarCtl::DISP_DEC);
}

void WatchVarCtl::onDisplayAsHex()
{
    selectedChangeDisplayFormat(VarCtl::DISP_HEX);
}

void WatchVarCtl::onDisplayAsBin()
{
    selectedChangeDisplayFormat(VarCtl::DISP_BIN);
}

void WatchVarCtl::onDisplayAsChar()
{
    selectedChangeDisplayFormat(VarCtl::DISP_CHAR);
}


void WatchVarCtl::onRemoveWatch()
{
    deleteSelected();
}



/**
 * @brief Called when the user right clicks anywhere.
 */
void WatchVarCtl::onContextMenu ( const QPoint &pos)
{

    m_popupMenu.clear();

           
    // Add menu entries
    m_popupMenu.addSeparator();
    
    QAction *action = m_popupMenu.addAction("Display as dec");
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
    action = m_popupMenu.addAction("Remove watch");
    action->setData(0);
    connect(action, SIGNAL(triggered()), this, SLOT(onRemoveWatch()));

        
    m_popupMenu.popup(m_varWidget->mapToGlobal(pos));
}


void 
WatchVarCtl::onWatchWidgetCurrentItemChanged( QTreeWidgetItem * current, int column )
{
    QTreeWidget *varWidget = m_varWidget;
    Core &core = Core::getInstance();
    QString oldKey = current->data(DATA_COLUMN, Qt::UserRole).toString();
    QString oldName  = oldKey == "" ? "" : core.gdbGetVarWatchName(oldKey);
    QString newName = current->text(COLUMN_NAME);

    AutoSignalBlocker autoBlocker(m_varWidget);


    if(column == COLUMN_VALUE)
    {
        VarWatch *watch = NULL;
        if(newName == "...")
            current->setText(COLUMN_VALUE, "");
        else
        {
        watch = core.getVarWatchInfo(oldKey);
        QString oldValue  = watch->getValue();
        QString newValue = current->text(COLUMN_VALUE);

            VarCtl::DispInfo dispInfo = m_watchVarDispInfo[oldKey];
            if (dispInfo.orgValue != newValue)
            {

                 if(core.changeWatchVariable(oldKey, newValue))
                    current->setText(COLUMN_VALUE, oldValue);
    
            }
         }   
        
    }

    // Only allow changes to 'Value' and 'Name' column
    if(column != 0)
        return;

    // Changed name to the same name?
    if(oldKey != "" && oldName == newName)
        return;

    // Only allow name changes on root items
    if(current->parent() != NULL)
    {
        current->setText(COLUMN_NAME, oldName);
        return;
    }

    if(newName == "...")
        newName = "";
    if(oldName == "...")
        oldName = "";
        
    // Nothing to do?
    if(oldName == "" && newName == "")
    {
        current->setText(COLUMN_NAME, "...");
        current->setText(COLUMN_VALUE, "");
        current->setText(COLUMN_TYPE, "");
    }
    // Remove a variable?
    else if(newName.isEmpty())
    {
        QTreeWidgetItem *rootItem = varWidget->invisibleRootItem();
        rootItem->removeChild(current);

        core.gdbRemoveVarWatch(oldKey);

        m_watchVarDispInfo.remove(oldKey);
    }
    // Add a new variable?
    else if(oldName == "")
    {
        //debugMsg("%s", stringToCStr(current->text(0)));
        VarWatch *watch = NULL;
        if(core.gdbAddVarWatch(newName, &watch) == 0)
        {
            QString watchId = watch->getWatchId();
            QString varType = watch->getVarType();
            QString value  = watch->getValue();
            bool hasChildren = watch->hasChildren();

            
            if(hasChildren)
                current->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

            VarCtl::DispInfo dispInfo;
            dispInfo.orgValue = value;
            dispInfo.orgFormat = VarCtl::findVarType(value);
            dispInfo.dispFormat = dispInfo.orgFormat;
            m_watchVarDispInfo[watchId] = dispInfo;

            current->setData(DATA_COLUMN, Qt::UserRole, watchId);
            current->setText(COLUMN_VALUE, value);
            current->setText(COLUMN_TYPE, varType);


            // Create a new dummy item
            QTreeWidgetItem *item;
            QStringList names;
            names += "...";
            item = new QTreeWidgetItem(names);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            varWidget->addTopLevelItem(item);
            
        }
        else
        {
            current->setText(COLUMN_NAME, "...");
            current->setText(COLUMN_VALUE, "");
            current->setText(COLUMN_TYPE, "");
        }
    
    }
    // Change a existing variable?
    else
    {
        //debugMsg("'%s' -> %s", stringToCStr(current->text(0)), stringToCStr(current->text(0)));

        // Remove any children
        while(current->childCount())
        {
            QTreeWidgetItem *childItem =  current->takeChild(0);
            delete childItem;
        }
        

        // Remove old watch
        core.gdbRemoveVarWatch(oldKey);

        m_watchVarDispInfo.remove(oldKey);

        VarWatch *watch = NULL;
        if(core.gdbAddVarWatch(newName, &watch) == 0)
        {
            QString watchId = watch->getWatchId();
            QString varType = watch->getVarType();
            QString value  = watch->getValue();
            bool hasChildren = watch->hasChildren();

            current->setData(DATA_COLUMN, Qt::UserRole, watchId);
            current->setText(COLUMN_VALUE, value);
            current->setText(COLUMN_TYPE, varType);

            if(hasChildren)
            {
                current->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
            }
            core.gdbExpandVarWatchChildren(watchId);
            
            // Add display information
            VarCtl::DispInfo dispInfo;
            dispInfo.orgValue = value;
            dispInfo.orgFormat = VarCtl::findVarType(value);
            dispInfo.dispFormat = dispInfo.orgFormat;
            m_watchVarDispInfo[watchId] = dispInfo;
            
        }
        else
        {
            QTreeWidgetItem *rootItem = varWidget->invisibleRootItem();
            rootItem->removeChild(current);
        }
    }

}



void WatchVarCtl::onWatchWidgetItemExpanded(QTreeWidgetItem *item )
{
    Core &core = Core::getInstance();
    //QTreeWidget *varWidget = m_varWidget;

    // Get watchid of the item
    QString watchId = item->data(DATA_COLUMN, Qt::UserRole).toString();


    // Get the children
    core.gdbExpandVarWatchChildren(watchId);
    

}

void WatchVarCtl::onWatchWidgetItemCollapsed(QTreeWidgetItem *item)
{
    Q_UNUSED(item);
    
}



void WatchVarCtl::onWatchWidgetItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QTreeWidget *varWidget = m_varWidget;

    
    if(column == COLUMN_NAME || column == COLUMN_VALUE)
        varWidget->editItem(item,column);
    else
    {
        AutoSignalBlocker autoBlocker(m_varWidget);

        QString varName = item->text(COLUMN_NAME);
        QString watchId = item->data(DATA_COLUMN, Qt::UserRole).toString();

        if(m_watchVarDispInfo.contains(watchId))
        {
            VarCtl::DispInfo &dispInfo = m_watchVarDispInfo[watchId];
            if(dispInfo.orgFormat == VarCtl::DISP_DEC)
            {
                long long val = dispInfo.orgValue.toLongLong(0,0);

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

                QString valueText = VarCtl::valueDisplay(val, dispInfo.dispFormat);

                item->setText(1, valueText);
            }
        }
    }
}


     
void WatchVarCtl::fillInVars()
{
    QTreeWidget *varWidget = m_varWidget;
    QTreeWidgetItem *item;
    QStringList names;
    
    varWidget->clear();



    names.clear();
    names += "...";
    item = new QTreeWidgetItem(names);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    varWidget->insertTopLevelItem(0, item);
   

}


/**
 * @brief Adds a new watch item
 * @param varName    The expression to add as a watch.
 */
void WatchVarCtl::addNewWatch(QString varName)
{
    // Add the new variable to the watch list
    QTreeWidgetItem* rootItem = m_varWidget->invisibleRootItem();
    QTreeWidgetItem* lastItem = rootItem->child(rootItem->childCount()-1);
    lastItem->setText(COLUMN_NAME, varName);

}

void WatchVarCtl::deleteSelected()
{
    QTreeWidgetItem *rootItem = m_varWidget->invisibleRootItem();
        
    QList<QTreeWidgetItem *> items = m_varWidget->selectedItems();

    // Get the root item for each item in the list
    for(int i =0;i < items.size();i++)
    {
        QTreeWidgetItem *item = items[i];
        while(item->parent() != NULL)
        {
            item = item->parent();
        }
        items[i] = item;
    }

    // Loop through the items
    QSet<QTreeWidgetItem *> itemSet = items.toSet();
    QSet<QTreeWidgetItem *>::const_iterator setItr = itemSet.constBegin();
    for (;setItr != itemSet.constEnd();++setItr)
    {
        QTreeWidgetItem *item = *setItr;
    
        // Delete the item
        Core &core = Core::getInstance();
        QString watchId = item->data(DATA_COLUMN, Qt::UserRole).toString();
        if(watchId != "")
        {
            rootItem->removeChild(item);
            core.gdbRemoveVarWatch(watchId);
        }
    }

}

    

    
