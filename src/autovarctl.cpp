/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "autovarctl.h"

#include "mainwindow.h"

AutoVarCtl::AutoVarCtl()
    : m_autoWidget(0)
{

}

QString getTreeWidgetItemPath(QTreeWidgetItem *item)
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


void AutoVarCtl::ICore_onLocalVarChanged(QString name, CoreVarValue varValue)
{
    Tree *valueTree = varValue.toTree();
    
    QTreeWidget *autoWidget = m_autoWidget;
    QTreeWidgetItem *item;
    QStringList names;


    item = insertTreeWidgetItem(&m_autoVarDispInfo, name, name, varValue.toString());
    autoWidget->insertTopLevelItem(0, item);

    // Expand it?
    QString varPath = getTreeWidgetItemPath(item);
    if(m_autoVarDispInfo.contains(varPath))
    {
        VarCtl::DispInfo &dispInfo = m_autoVarDispInfo[varPath];
        if(dispInfo.isExpanded)
        {
            autoWidget->expandItem(item);
        }
    }

    // Insert children
    if(valueTree)
    {
        addVariableDataTree(autoWidget, &m_autoVarDispInfo, item, valueTree->getRoot());
    }
    
    delete valueTree;
}




void AutoVarCtl::addVariableDataTree(
                QTreeWidget *treeWidget,
                VarCtl::DispInfoMap *map,
                QTreeWidgetItem *item, TreeNode *rootNode)
{
    QList <TreeNode *> m_list;

    QString parentPath = getTreeWidgetItemPath(item);

    for(int i = 0;i < rootNode->getChildCount();i++)
    {
        TreeNode *child = rootNode->getChild(i);

        QString varPath = parentPath + "/" + child->getName();
        QTreeWidgetItem *childItem;

        if(child->getChildCount() == 0)
        {
            childItem = insertTreeWidgetItem(
                    map,
                    varPath,
                    child->getName(),
                    child->getData());

            item->addChild(childItem);

        }
        else
        {
            QStringList names;
            names += child->getName();

            childItem = new QTreeWidgetItem(names);
            item->addChild(childItem);

            addVariableDataTree(treeWidget, map, childItem, child);
        }


        // Expand it?
        if(m_autoVarDispInfo.contains(varPath))
        {
            VarCtl::DispInfo &dispInfo = (*map)[varPath];

            if(dispInfo.isExpanded)
            {
                treeWidget->expandItem(childItem);
            }
        }
        else
        {
            // Add it to the dispinfomap
            VarCtl::DispInfo dispInfo;
            dispInfo.isExpanded = false;
            (*map)[varPath] = dispInfo;
        }
    }
}

        
QTreeWidgetItem *AutoVarCtl::insertTreeWidgetItem(
                    VarCtl::DispInfoMap *map,
                    QString fullPath,
                    QString name,
                    QString value)
{
    QString displayValue = value;
    VarCtl::DispFormat orgFormat = VarCtl::findVarType(value);

    //
    if(map->contains(fullPath))
    {
        VarCtl::DispInfo &dispInfo = (*map)[fullPath];
        dispInfo.orgValue = value;

        // Update the variable value
        if(orgFormat == VarCtl::DISP_DEC)
        {
            displayValue = VarCtl::valueDisplay(value.toLongLong(0,0), dispInfo.dispFormat);
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

    return item;
}

