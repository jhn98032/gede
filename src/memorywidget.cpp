#include "memorywidget.h"

#include <stdint.h>
#include <QPainter>
#include <QPaintEvent>
#include <QColor>


static const int PAD_ADDR_LEFT = 10;
static const int PAD_ADDR_RIGHT = 10;
static const int PAD_HEX_MIDDLE = 10;
static const int PAD_HEX_RIGHT = 10;
static const int PAD_DATA  =5;


MemoryWidget::MemoryWidget(QWidget *parent)
 : QWidget(parent)
 ,m_selectionStart(0)
 ,m_selectionEnd(0)
 ,m_inf(0)
{
    m_font = QFont("Monospace", 8);
    m_fontInfo = new QFontMetrics(m_font);




    setMinimumSize(400,getRowHeight()*10);
    update();

    m_startAddress = 0;
}

void MemoryWidget::setInterface(IMemoryWidget *inf)
{
    m_inf = inf;
}
    
void MemoryWidget::setStartAddress(unsigned int addr)
{

    m_startAddress = addr;
    update();
}


/**
 * @brief Returns the height of a text row in pixels.
 */
int MemoryWidget::getRowHeight()
{
    int rowHeight = m_fontInfo->lineSpacing()+2;
    return rowHeight;
}

MemoryWidget::~MemoryWidget()
{
    delete m_fontInfo;
}


int MemoryWidget::getHeaderHeight()
{
    return getRowHeight()+5;
}



void MemoryWidget::paintEvent ( QPaintEvent * event )
{
    QPainter painter(this);
    const int rowHeight = getRowHeight();
    const int charWidth = m_fontInfo->width("H");
    QString text;
    int HEADER_HEIGHT = getHeaderHeight();
    int x;
    int rowCount = ((size().height()-HEADER_HEIGHT)/rowHeight)+1;
    unsigned int startAddress = m_startAddress;
    
    painter.setFont(m_font);

    QByteArray content;
    if(m_inf)
        content = m_inf->getMemory(startAddress, rowCount*16);
    

    //if((0xffffffffU-startAddress) < rowCount*16)
    //    startAddress = 0xffffffffU-((rowCount-2)*16);
    
    // Draw background
    QRect rect2(0,0,PAD_ADDR_LEFT+charWidth*9+PAD_ADDR_RIGHT/2, event->rect().bottom()+1);
    painter.fillRect(rect2, Qt::lightGray);


    QRect rect3(0,0, event->rect().right()+1, HEADER_HEIGHT);
    painter.fillRect(rect3, Qt::cyan);

    rect3 = QRect(0,HEADER_HEIGHT, event->rect().right(), HEADER_HEIGHT);
    painter.setPen(Qt::black);
    painter.drawLine(0, HEADER_HEIGHT, event->rect().right(), HEADER_HEIGHT);
    
    // Draw header
    text.sprintf("Address");
    x = PAD_ADDR_LEFT;
    painter.drawText(PAD_ADDR_LEFT, rowHeight, text);
    x += (charWidth*9)+PAD_ADDR_RIGHT;
    for(int off = 0;off < 16;off++)
    {
        text.sprintf("%x", off);
        painter.drawText(x, rowHeight, text);
        x += (charWidth*2)+PAD_DATA;
        if(off==8)
            x += PAD_HEX_MIDDLE;
    }
    x += PAD_HEX_RIGHT;

    rect2 = QRect(x,HEADER_HEIGHT+1,x+charWidth*16, event->rect().bottom());
    painter.fillRect(rect2, Qt::lightGray);

    for(int off = 0;off < 16;off++)
    {
        text.sprintf("%x", off);
        painter.drawText(x, rowHeight, text);
        x += charWidth;
    }

    // Draw data
    for(int rowIdx= 0;rowIdx < rowCount;rowIdx++)
    {
        int y = HEADER_HEIGHT+rowHeight*rowIdx+rowHeight;
        x = PAD_ADDR_LEFT;
        
        unsigned int memoryAddr = startAddress + rowIdx*16;
        if(memoryAddr < startAddress)
            break;
            
        text.sprintf("%04x_%04x", (unsigned int)(memoryAddr>>16),(unsigned int)(memoryAddr&0xffffUL));
        painter.drawText(x, y, text);
        x += charWidth*text.length();
        x += PAD_ADDR_RIGHT;
        
        for(int off = 0;off < 16;off++)
        {
            int dataIdx = rowIdx*16+off;
            if(dataIdx < content.size())
            {
            uint8_t d = content[dataIdx];

            if(m_selectionStart != 0 || m_selectionEnd != 0)
            {
                if(m_selectionStart <= off+memoryAddr && off+memoryAddr <=  m_selectionEnd)
                    painter.setPen(Qt::red);
                else
                    painter.setPen(Qt::black);
            }
            
            text.sprintf("%02u", d);
            painter.drawText(x, y, text);
            }
        
            x += charWidth*text.length()+5;

            if(off == 8)
                x += PAD_HEX_MIDDLE;
        }
        x += PAD_HEX_RIGHT;

        painter.setPen(Qt::black);
            
        for(int off = 0;off < 16;off++)
        {
            int dataIdx = rowIdx*16+off;
            if(dataIdx < content.size())
            {
            uint8_t d = content[dataIdx];
            if(isalnum(d))
                text.sprintf("%c", (char)d);
            else
                text.sprintf(".");

            if(m_selectionStart != 0 || m_selectionEnd != 0)
            {
                if(m_selectionStart <= off+memoryAddr && off+memoryAddr <=  m_selectionEnd)
                    painter.setPen(Qt::red);
                else
                    painter.setPen(Qt::black);
            }
            
            painter.drawText(x, y, text);
            }
            x += charWidth*text.length();
        }


    }

    // Draw border
    painter.setPen(Qt::black);
    painter.drawRect(0,0, frameSize().width()-2,frameSize().height()-2);

}



unsigned int MemoryWidget::getAddrAtPos(QPoint pos)
{
    const int rowHeight = getRowHeight();
    const int charWidth = m_fontInfo->width("H");
    unsigned int addr;
    
    addr = m_startAddress+(pos.y()-getHeaderHeight())/rowHeight*16;


    int x = pos.x();
    x -= PAD_ADDR_LEFT+(charWidth*9)+PAD_ADDR_RIGHT;
    if(x > 0)
    {
        if(x > (PAD_HEX_MIDDLE+8*((charWidth*2)+5)))
            x -= PAD_HEX_MIDDLE;
        x/= ((charWidth*2)+5);
    }
    if(x < 0)
        x = 0;
    if(16 < x)
        x = 16;

    addr += x;
    return addr;
 } 

void MemoryWidget::mouseMoveEvent ( QMouseEvent * event )
{
    m_selectionEnd = getAddrAtPos(event->pos());
    //printf("%s()\n", __func__);
    //printf("%x\n", getAddrAtPos(event->pos()));
    update();
}
    
void MemoryWidget::mouseReleaseEvent(QMouseEvent * event)
{
    //printf("%s()\n", __func__);
    update();
    
}

void MemoryWidget::mousePressEvent(QMouseEvent * event)
{
    //printf("%s()\n", __func__);
    m_selectionStart = getAddrAtPos(event->pos());
    m_selectionEnd = m_selectionStart;
    update();
    
}


