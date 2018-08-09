/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */


#include "memorydialog.h"

#include "core.h"
#include "util.h"

#define SCROLL_ADDR_RANGE   0x10000ULL

QByteArray MemoryDialog::getMemory(uint64_t startAddress, int count)
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

    m_ui.verticalScrollBar->setRange(0, SCROLL_ADDR_RANGE/16);
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

void MemoryDialog::setStartAddress(uint64_t addr)
{
    uint64_t addrAligned = addr & ~0xfULL;

    if(addrAligned < (SCROLL_ADDR_RANGE/2))
        m_startScrollAddress = 0;
    else
        m_startScrollAddress = addrAligned - (SCROLL_ADDR_RANGE/2);
    
    m_ui.memorywidget->setStartAddress(addrAligned);
    m_ui.verticalScrollBar->setValue((addrAligned-m_startScrollAddress)/16);

    QString addrText = addrToString(addr);
    m_ui.lineEdit_address->setText(addrText);
}


void MemoryDialog::onVertScroll(int pos)
{
    uint64_t addr = m_startScrollAddress + ((uint64_t)pos*16ULL);
    m_ui.memorywidget->setStartAddress(addr);
}

void MemoryDialog::setConfig(Settings *cfg)
{
    m_ui.memorywidget->setConfig(cfg);
}


