#ifndef FILE__AUTO_VAR_CTL_H
#define FILE__AUTO_VAR_CTL_H

#include "tree.h"
#include "core.h"
#include <QTreeWidget>
#include "varctl.h"


class AutoVarCtl : public VarCtl
{
    Q_OBJECT
public:


public:
    AutoVarCtl();


    void setWidget(QTreeWidget *autoWidget);
                    
public:
    QTreeWidget *m_autoWidget;


};



#endif // FILE__AUTO_VAR_CTL_H

