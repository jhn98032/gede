#ifndef FILE__MEMORYWIDGET_H
#define FILE__MEMORYWIDGET_H

#include <QWidget>
#include <QFont>
#include <QScrollBar>
#include <QMenu>


#include "settings.h"


class IMemoryWidget
{
public:
    virtual QByteArray getMemory(quint64 startAddress, int count) = 0;

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
    quint64 getAddrAtPos(QPoint pos);
    int getHeaderHeight();
    char byteToChar(quint8 d);

    virtual void keyPressEvent(QKeyEvent *e);
    
public slots:
    void setStartAddress(quint64 addr);
    void onCopy();
    
private:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent ( QMouseEvent * event );
    void mouseReleaseEvent(QMouseEvent * event);
    
private:
    QFont m_font;
    QFontMetrics *m_fontInfo;

    bool m_selectionStartValid;
    quint64 m_startAddress;
    quint64 m_selectionStart, m_selectionEnd;
    IMemoryWidget *m_inf;
    QMenu m_popupMenu;
    int m_addrCharWidth;
};

#endif // FILE__MEMORYWIDGET_H

