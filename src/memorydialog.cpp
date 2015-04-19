#include "memorydialog.h"

#include "core.h"
#include "util.h"

QByteArray MemoryDialog::getMemory(unsigned int startAddress, int count)
{
     Core &core = Core::getInstance();
   
    QByteArray b;
    core.gdbGetMemory(startAddress, count, &b);

    return b;
}

MemoryDialog::MemoryDialog(QWidget *parent)
    : QDialog(parent)
{
    
    m_ui.setupUi(this);

    m_ui.verticalScrollBar->setRange(0,0xffffffffUL/16);
    connect(m_ui.verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(onVertScroll(int)));

    m_ui.memorywidget->setInterface(this);

    setStartAddress(0x0);

   connect(m_ui.pushButton_update, SIGNAL(clicked()), SLOT(onUpdate()));


}


void MemoryDialog::onUpdate()
{
    uint64_t addr = stringToLongLong(m_ui.lineEdit_address->text());
    setStartAddress(addr);
}

void MemoryDialog::setStartAddress(unsigned int addr)
{
    unsigned int addrAligned = addr & ~0xfULL;
    
    m_ui.memorywidget->setStartAddress(addrAligned);
    m_ui.verticalScrollBar->setValue(addrAligned/16);

    QString addrText;
    addrText.sprintf("0x%04x_%04x", addr>>16, addr&0xffff);
    m_ui.lineEdit_address->setText(addrText);
}


void MemoryDialog::onVertScroll(int pos)
{
    m_ui.memorywidget->setStartAddress(((unsigned int)pos)*16UL);
}



