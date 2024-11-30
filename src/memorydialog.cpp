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

#if QT_VERSION >= QT_VERSION_CHECK(5,8,0)
#include <QRegularExpression>
#endif

#define SCROLL_ADDR_RANGE   0x10000ULL

QByteArray MemoryDialog::getMemory(quint64 startAddress, int count)
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

/**
 * @brief Converts a string entered by the user to an address.
 */
quint64 MemoryDialog::inputTextToAddress(QString text)
{
    // Remove leading zeroes
    while(text.startsWith('0') && text.length() > 1)
        text = text.mid(1);

    // Starts with a '0x...' or '0X..'?
    if(text.startsWith("x") || text.startsWith("X"))
        text = "0" + text;
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    else if(text.lastIndexOf(QRegularExpression("[a-zA-Z]+")) != -1)
#else
    else if(text.lastIndexOf(QRegExp("[a-zA-Z]+")) != -1)
#endif
    {
        text = "0x" + text;
    }
    
    return stringToLongLong(text);
}


void MemoryDialog::onUpdate()
{
    quint64 addr = inputTextToAddress(m_ui.lineEdit_address->text());
    setStartAddress(addr);
}

void MemoryDialog::setStartAddress(quint64 addr)
{
    quint64 addrAligned = addr & ~0xfULL;

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
    quint64 addr = m_startScrollAddress + ((quint64)pos*16ULL);
    m_ui.memorywidget->setStartAddress(addr);
}

void MemoryDialog::setConfig(Settings *cfg)
{
    m_ui.memorywidget->setConfig(cfg);
}


void MemoryDialog::wheelEvent(QWheelEvent *event)
{
    int dy = -event->angleDelta().y()/32;
    int oldPos = m_ui.verticalScrollBar->value();
    m_ui.verticalScrollBar->setValue(oldPos+dy);
}
