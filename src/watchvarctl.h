/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef WATCHVAR_CTL_H
#define WATCHVAR_CTL_H

#include <QString>
#include <QTreeWidget>
#include <QMenu>
#include <QKeyEvent>


#include "core.h"
#include "varctl.h"


class WatchVarCtl : public VarCtl
{
    Q_OBJECT

public:
    WatchVarCtl();
    
    void setWidget(QTreeWidget *varWidget);

    void ICore_onWatchVarChanged(VarWatch &watch);
    void ICore_onWatchVarChildAdded(VarWatch &watch);
    void addNewWatch(QString varName);
    void deleteSelected();

    void onKeyPress(QKeyEvent *keyEvent);

private:
    QString getWatchId(QTreeWidgetItem* item);

    void selectedChangeDisplayFormat(VarCtl::DispFormat fmt);

    
    void sync(QTreeWidgetItem * parentItem, VarWatch &watch);
    
public slots:
    void onWatchWidgetItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onWatchWidgetCurrentItemChanged ( QTreeWidgetItem * current, int column );
    void onWatchWidgetItemExpanded(QTreeWidgetItem *item );
    void onWatchWidgetItemCollapsed(QTreeWidgetItem *item);

    void onContextMenu ( const QPoint &pos);


    void onDisplayAsDec();
    void onDisplayAsHex();
    void onDisplayAsBin();
    void onDisplayAsChar();
    void onRemoveWatch();

private:
    void fillInVars();

private:
    QTreeWidget *m_varWidget;
    VarCtl::DispInfoMap m_watchVarDispInfo;
    QMenu m_popupMenu;

};

#endif // WATCHVAR_CTL_H
