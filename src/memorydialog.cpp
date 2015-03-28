#include "memorydialog.h"


MemoryDialog::MemoryDialog(QWidget *parent)
    : QDialog(parent)
{
    
    m_ui.setupUi(this);

    m_ui.verticalScrollBar->setRange(0,0xffffffffUL/16);
    connect(m_ui.verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(onVertScroll(int)));

}


void MemoryDialog::onVertScroll(int pos)
{
    m_ui.memorywidget->setStartAddress(((unsigned int)pos)*16UL);
}



