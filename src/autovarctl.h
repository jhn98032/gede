/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__AUTO_VAR_CTL_H
#define FILE__AUTO_VAR_CTL_H

#include <QString>
#include <QTreeWidget>
#include <QMenu>
#include <QKeyEvent>


#include "core.h"
#include "varctl.h"
#include "settings.h"

/**
* @brief Displays local variables (on the stack).
*/
class AutoVarCtl : public VarCtl
{
    Q_OBJECT

public:
    AutoVarCtl();
    
    void setWidget(QTreeWidget *autoWidget);

    void ICore_onWatchVarChanged(VarWatch &watch);
    void ICore_onWatchVarChildAdded(VarWatch &watch);
    void addNewWatch(QString varName);


    void setConfig(Settings *cfg);

    void ICore_onLocalVarChanged(QStringList varNames);

    void onKeyPress(QKeyEvent *keyEvent);

    void ICore_onStateChanged(ICore::TargetState state);
private:
    quint64 getAddress(VarWatch &w);
    QString getWatchId(QTreeWidgetItem* item);

    void selectedChangeDisplayFormat(VarCtl::DispFormat fmt);
    QString getTreeWidgetItemPath(QTreeWidgetItem *item);

    QString getDisplayString(QString watchId, QString varPath);
    
    
public slots:
    void onAutoWidgetItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onAutoWidgetCurrentItemChanged ( QTreeWidgetItem * current, int column );
    void onAutoWidgetItemExpanded(QTreeWidgetItem *item );
    void onAutoWidgetItemCollapsed(QTreeWidgetItem *item);

    void onContextMenu ( const QPoint &pos);
    void onShowMemory();


    void onDisplayAsDec();
    void onDisplayAsHex();
    void onDisplayAsBin();
    void onDisplayAsChar();

private:
    void clear();

private:
    QTreeWidget *m_autoWidget;
    QMenu m_popupMenu;
    
    VarCtl::DispInfoMap m_autoVarDispInfo;
    Settings m_cfg;
    QColor m_textColor; //!< Color to use for text in the widget
};



#endif // FILE__AUTO_VAR_CTL_H

