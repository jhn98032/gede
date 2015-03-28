#ifndef WATCHVAR_CTL_H
#define WATCHVAR_CTL_H

#include <QObject>
#include <QString>
#include <QTreeWidget>

#include "varctl.h"


class WatchVarCtl : public VarCtl
{
    Q_OBJECT

public:
    WatchVarCtl();
    
    void setWidget(QTreeWidget *varWidget);

    void ICore_onWatchVarExpanded(QString watchId_, QString name, QString valueString, QString varType);

public slots:
    void onWatchWidgetItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onWatchWidgetCurrentItemChanged ( QTreeWidgetItem * current, int column );
    void onWatchWidgetItemExpanded(QTreeWidgetItem *item );
    void onWatchWidgetItemCollapsed(QTreeWidgetItem *item);


private:
    void fillInVars();

private:
    QTreeWidget *m_varWidget;
    VarCtl::DispInfoMap m_watchVarDispInfo;
};

#endif // WATCHVAR_CTL_H
