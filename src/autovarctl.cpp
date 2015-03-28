#include "autovarctl.h"

#include "mainwindow.h"

AutoVarCtl::AutoVarCtl()
    : m_autoWidget(0)
{

}

void AutoVarCtl::setWidget(QTreeWidget *autoWidget)
{
    m_autoWidget = autoWidget;


}

