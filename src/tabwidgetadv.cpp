#include "tabwidgetadv.h"

#include <QTabBar>



TabWidgetAdv::TabWidgetAdv(QWidget * parent)
  : QTabWidget(parent)
{

}

TabWidgetAdv::~TabWidgetAdv()
{


}

int TabWidgetAdv::tabAt(QPoint pos)
{
    return tabBar()->tabAt(pos);
}


