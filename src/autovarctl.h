/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__AUTO_VAR_CTL_H
#define FILE__AUTO_VAR_CTL_H

#include "tree.h"
#include "core.h"
#include <QTreeWidget>
#include "varctl.h"
#include <QMenu>

#include "settings.h"

class AutoVarCtl : public VarCtl
{
    Q_OBJECT
public:


public:
    AutoVarCtl();


    void setWidget(QTreeWidget *autoWidget);

    void ICore_onLocalVarChanged(QString name, CoreVarValue varValue);

    void setConfig(Settings *cfg);

private:
    void addVariableDataTree(
                QTreeWidget *treeWidget,
                VarCtl::DispInfoMap *map,
                QTreeWidgetItem *item, TreeNode *rootNode);
    QTreeWidgetItem *insertTreeWidgetItem(
                    VarCtl::DispInfoMap *map,
                    QString fullPath,
                    QString name,
                    QString value);
                                
public slots:

    void onContextMenu ( const QPoint &pos);
    void onAutoWidgetItemCollapsed(QTreeWidgetItem *item);
    void onAutoWidgetItemExpanded(QTreeWidgetItem *item);
    void onAutoWidgetItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onShowMemory();


    
public:
    QTreeWidget *m_autoWidget;
    QMenu m_popupMenu;
    
    VarCtl::DispInfoMap m_autoVarDispInfo;
    Settings m_cfg;
};



#endif // FILE__AUTO_VAR_CTL_H

