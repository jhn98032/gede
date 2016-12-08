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


    fillInVars();


}



void WatchVarCtl::ICore_onWatchVarChanged(VarWatch &watch)
{
    QTreeWidget *varWidget = m_varWidget;
    Core &core = Core::getInstance();

    QStringList watchIdParts = watch.getWatchId().split('.');

    QTreeWidgetItem* rootItem = varWidget->invisibleRootItem();
    VarWatch *rootWatch = core.getVarWatchInfo(watchIdParts[0]);
    assert(rootWatch != NULL);
    if(rootWatch)
        sync(rootItem, *rootWatch);
}

QString WatchVarCtl::getWatchId(QTreeWidgetItem* item)
{
    return item->data(0, Qt::UserRole).toString();
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
        treeItem->setData(0, Qt::UserRole, watchId);
        treeItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
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
    
    treeItem->setText(1, valueString);
    treeItem->setText(2, varType);
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
            QString itemKey = item->data(0, Qt::UserRole).toString();

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
            item->setData(0, Qt::UserRole, thisWatchId);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
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
            if(m_watchVarDispInfo.contains(thisWatchId))
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
                

            item->setText(1, valueString);
        }
    }
}



void 
WatchVarCtl::onWatchWidgetCurrentItemChanged( QTreeWidgetItem * current, int column )
{
    QTreeWidget *varWidget = m_varWidget;
    Core &core = Core::getInstance();
    QString oldKey = current->data(0, Qt::UserRole).toString();
    QString oldName  = oldKey == "" ? "" : core.gdbGetVarWatchName(oldKey);
    QString newName = current->text(0);

    if(column != 0)
        return;

    if(oldKey != "" && oldName == newName)
        return;
    

    if(newName == "...")
        newName = "";
    if(oldName == "...")
        oldName = "";
        
    // Nothing to do?
    if(oldName == "" && newName == "")
    {
        current->setText(0, "...");
        current->setText(1, "");
        current->setText(2, "");
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
            current->setData(0, Qt::UserRole, watchId);
            current->setText(1, value);
            current->setText(2, varType);

            VarCtl::DispInfo dispInfo;
            dispInfo.orgValue = value;
            dispInfo.orgFormat = VarCtl::findVarType(value);
            dispInfo.dispFormat = dispInfo.orgFormat;
            m_watchVarDispInfo[watchId] = dispInfo;

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
            current->setText(0, "...");
            current->setText(1, "");
            current->setText(2, "");
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

            current->setData(0, Qt::UserRole, watchId);
            current->setText(1, value);
            current->setText(2, varType);

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
    QString watchId = item->data(0, Qt::UserRole).toString();


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

    
    if(column == 0)
        varWidget->editItem(item,column);
    else if(column == 1)
    {
        QString varName = item->text(0);
        QString watchId = item->data(0, Qt::UserRole).toString();

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
    lastItem->setText(0, varName);

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
        QString watchId = item->data(0, Qt::UserRole).toString();
        if(watchId != "")
        {
            rootItem->removeChild(item);
            core.gdbRemoveVarWatch(watchId);
        }
    }

}

    

    
