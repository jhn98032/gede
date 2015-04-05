
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
    m_varWidget->setColumnWidth(0, 80);
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



         
void WatchVarCtl::ICore_onWatchVarChildAdded(QString watchId_, QString name, QString valueString, QString varType, bool hasChildren)
{
    QTreeWidget *varWidget = m_varWidget;
    QStringList names;

    Q_UNUSED(name);

    //
    QTreeWidgetItem * rootItem = varWidget->invisibleRootItem();
    QStringList watchIdParts = watchId_.split('.');
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

        // Did not find one
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
            if(item->childCount() != 0)
            {
                if(valueString == "")
                    item->setDisabled(true);
                else
                    item->setDisabled(false);
                

            }
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
    
    debugMsg("oldKey:'%s' oldName:'%s' newName:'%s' ", stringToCStr(oldKey), stringToCStr(oldName), stringToCStr(newName));

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
        QTreeWidgetItem *item = varWidget->invisibleRootItem();
        item->removeChild(current);

        core.gdbRemoveVarWatch(oldKey);

        m_watchVarDispInfo.remove(oldKey);
    }
    // Add a new variable?
    else if(oldName == "")
    {
        //debugMsg("%s", stringToCStr(current->text(0)));
        QString value;
        QString watchId;
        QString varType;
        bool hasChildren = false;
        if(core.gdbAddVarWatch(newName, &varType, &value, &watchId, &hasChildren) == 0)
        {
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

        QString value;
        QString watchId;
        QString varType;
        bool hasChildren = false;
        if(core.gdbAddVarWatch(newName, &varType, &value, &watchId, &hasChildren) == 0)
        {
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
    QList<QTreeWidgetItem *> items = m_varWidget->selectedItems();

    for(int i =0;i < items.size();i++)
    {
        QTreeWidgetItem *item = items[i];
    
        // Delete the item
        Core &core = Core::getInstance();
        QTreeWidgetItem *rootItem = m_varWidget->invisibleRootItem();
        QString watchId = item->data(0, Qt::UserRole).toString();
        if(watchId != "")
        {
            rootItem->removeChild(item);
            core.gdbRemoveVarWatch(watchId);
        }
    }

}

    

    
