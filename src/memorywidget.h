#ifndef FILE__MEMORYWIDGET_H
#define FILE__MEMORYWIDGET_H

#include <QWidget>
#include <QFont>
#include <QScrollBar>
#include <QMenu>

#include <stdint.h>

#include "settings.h"


class IMemoryWidget
{
public:
    virtual QByteArray getMemory(uint64_t startAddress, int count) = 0;

};

class MemoryWidget : public QWidget
{
    Q_OBJECT

public:

    MemoryWidget(QWidget *parent = NULL);
    virtual ~MemoryWidget();

 void paintEvent ( QPaintEvent * event );
    void setInterface(IMemoryWidget *inf);

    void setConfig(Settings *cfg);
    
private:
    int getRowHeight();
    uint64_t getAddrAtPos(QPoint pos);
    int getHeaderHeight();
    char byteToChar(uint8_t d);

    virtual void keyPressEvent(QKeyEvent *e);
    
public slots:
    void setStartAddress(uint64_t addr);
    void onCopy();
    
private:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent ( QMouseEvent * event );
    void mouseReleaseEvent(QMouseEvent * event);
    
private:
    QFont m_font;
    QFontMetrics *m_fontInfo;

    bool m_selectionStartValid;
    uint64_t m_startAddress;
    uint64_t m_selectionStart, m_selectionEnd;
    IMemoryWidget *m_inf;
    QMenu m_popupMenu;
    int m_addrCharWidth;
};

#endif // FILE__MEMORYWIDGET_H

