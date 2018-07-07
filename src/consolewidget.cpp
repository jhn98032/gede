/*
 * Copyright (C) 2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "consolewidget.h"

#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QDebug>
#include <QPaintEvent>
#include <QClipboard>

//#define ENABLE_DEBUGMSG

#include "log.h"
#include "core.h"


#define ASCII_ESC           0x1B
#define ASCII_BELL          0x7
#define ASCII_BACKSPACE     '\b'

static QColor red(255,0,0);

ConsoleWidget::ConsoleWidget(QWidget *parent)
    : QWidget(parent)
    ,m_fontInfo(NULL)
    ,m_ansiState(ST_IDLE)
{
    m_cursorX = 0;
    m_cursorY = 0;

    m_fgColor = Qt::black;
    m_bgColor = Qt::white;

    setMonoFont(QFont("Monospace", 18));

    setMinimumSize(100,1);

    setFocusPolicy(Qt::StrongFocus);
}

ConsoleWidget::~ConsoleWidget()
{
 delete m_fontInfo;

}

void ConsoleWidget::setMonoFont(QFont font)
{
    m_font = font;
    delete m_fontInfo;
    m_fontInfo = new QFontMetrics(m_font);
    setFont(m_font);
}

QString printable(QString str)
{
    QString line;
    for(int i = 0;i < str.size();i++)
    {
        QChar c = str[i];
        if(0x21 <= c &&  c <= 0x7e)
            line += c;
        else if(c == '\t')
            line += "\\t";
        else if(c == '\n')
            line += "\\n";
        else if(c == '\r')
            line += "\\r";
        else if(c == '\b')
            line += "\\b";
        else if(c == ' ')
            line += " ";
        else if(c == ASCII_ESC)
            line += "<ESC>";
        else
        {
            QString codeStr;
            codeStr.sprintf("<0x%x>", (int)c.unicode());
            line += codeStr;
        }
    }

    return line;
}

void ConsoleWidget::clearAll()
{
    m_lines.clear();
    setMinimumSize(100,1);
    m_cursorX = 0;
    m_cursorY = 0;
}



/**
 * @brief Returns the height of a text row in pixels.
 */
int ConsoleWidget::getRowHeight()
{
    int rowHeight = m_fontInfo->lineSpacing()+1;
    return rowHeight;
}

void ConsoleWidget::paintEvent ( QPaintEvent * event )
{
    int rowHeight = getRowHeight();
    QPainter painter(this);
    QColor clrBackground = m_bgColor;

   painter.fillRect(event->rect(), clrBackground);

    painter.setFont(m_font);

    QRect r;
    int charWidth = m_fontInfo->width(" ");
    r.setX(charWidth*m_cursorX+5);
    r.setY(rowHeight*m_cursorY);
    r.setWidth(charWidth);
    r.setHeight(rowHeight);
    painter.fillRect(r, QBrush(Qt::black));
    
    for(int rowIdx = 0;rowIdx < m_lines.size();rowIdx++)
    {
        Line &line = m_lines[rowIdx];

        int y = rowHeight*rowIdx;
        int x = 5;
        int curCharIdx = 0;
        
        // Draw line number
        int fontY = y+(rowHeight-(m_fontInfo->ascent()+m_fontInfo->descent()))/2+m_fontInfo->ascent();
    
        for(int blockIdx = 0;blockIdx < line.size();blockIdx++)
        {
            Block &blk = line[blockIdx];
            painter.setPen(blk.m_fgColor);
            if(m_cursorY == rowIdx && curCharIdx <= m_cursorX && m_cursorX < curCharIdx+blk.text.size())
            {
                int cutIdx = m_cursorX-curCharIdx;
                QString leftText = blk.text.left(cutIdx);
                QChar cutLetter = blk.text[cutIdx];
                QString rightText = blk.text.mid(cutIdx+1);
                painter.drawText(x, fontY, leftText);
                painter.setPen(blk.m_bgColor);
                painter.drawText(x+m_fontInfo->width(leftText), fontY, QString(cutLetter));
                painter.setPen(blk.m_fgColor);
                painter.drawText(x+m_fontInfo->width(leftText + cutLetter), fontY, rightText);

            }
            else
                painter.drawText(x, fontY, blk.text);
            x += m_fontInfo->width(blk.text);
            curCharIdx += blk.text.size();
        }
        
    }


}

void ConsoleWidget::insert(QChar c)
{
    debugMsg("%s('%s')", __func__, c == '\n' ? "\\n" : qPrintable(QString(c)));
    
    // Insert new lines?
    while(m_lines.size() <= m_cursorY)
    {
        Line line;
        Block blk;
        blk.m_fgColor = m_fgColor;
        blk.m_bgColor = m_bgColor;
        line.append(blk);
        m_lines.append(line);
    }

    // New line?
    if(c == '\n')
    {
        if(m_cursorY+1 == m_lines.size())
        {
            Line line;
            Block blk;
            blk.m_fgColor = m_fgColor;
            blk.m_bgColor = m_bgColor;
            line.append(blk);
            m_lines.append(line);
        }
    
        m_cursorY++;
        m_cursorX = 0;


        setMinimumSize(400,getRowHeight()*(m_lines.size()));

    }
    else
    {
        Line &line = m_lines[m_cursorY];
        int x = 0;
        for(int blkIdx = 0;blkIdx < line.size();blkIdx++)
        {
            Block *blk = &line[blkIdx];
            if(m_cursorX <= x+blk->text.size())
            {
                int ipos = m_cursorX - x;

                // Insert new block to the right?
                if(ipos+1 != blk->text.size() && blk->m_fgColor != m_fgColor)
                {
                    Block newBlk;
                    newBlk = *blk;
                    newBlk.text = newBlk.text.mid(ipos+1);
                    line.insert(blkIdx+1, newBlk);
                    
                    blk = &line[blkIdx];
                    blk->text = blk->text.left(ipos+1);
                }

                // Insert new block to the left?
                if(ipos != 0 && blk->m_fgColor != m_fgColor)
                {
                    Block newBlk;
                    newBlk = *blk;
                    newBlk.text = newBlk.text.left(ipos);
                    line.insert(blkIdx, newBlk);

                    blk = &line[blkIdx+1];
                    blk->text = blk->text.mid(ipos);
                    ipos = 0;
                }

                QString text = blk->text;
                
                blk->m_fgColor = m_fgColor;
                blk->m_bgColor = m_bgColor;                
                blk->text = text.left(ipos) + c + text.mid(ipos+1);

                m_cursorX++;
                return ;
            }
            x += blk->text.size();
        }
    }
}


void ConsoleWidget::appendLog ( QString text )
{
    debugMsg("%s(%d bytes)", __func__, text.size());
    for(int i = 0;i < text.size();i++)
    {
        QChar c = text[i];
        if(c == '\r')
            continue;
        if(c == '\n')
        {
            insert('\n');
            
        }
        else
        {
            switch(m_ansiState)
            {
                case ST_IDLE:
                {
                    if(c == ASCII_ESC)
                    {
                        m_ansiState = ST_SECBYTE;
                    }
                    else if(c == ASCII_BELL)
                    {
                    }
                    else if(c == ASCII_BACKSPACE)
                    {
                        m_cursorX = std::max(m_cursorX-1, 0);
                    }
                    else
                    {
                        insert(c);
                    }
                };break;
                case ST_SECBYTE: // Second byte in ANSI escape sequence
                {
                    if(c == '[')
                        m_ansiState = ST_CSI_PARAM;
                    else if(c == ']')
                        m_ansiState = ST_OSC_PARAM;
                    else
                    {
                        debugMsg("ANSI>%s<", qPrintable(QString(c)));
                        m_ansiState = ST_IDLE;
                    }
                };break;
                case ST_OSC_PARAM:
                {
                    if(c == ';')
                    {
                        m_ansiState = ST_OSC_STRING;
                    }
                    else
                    {
                        m_ansiParamStr += c;
                    }
                };break;
                case ST_OSC_STRING: // Operating System Command
                {
                    if(c == ASCII_BELL)
                    {
                        debugMsg("ANSI.OSC>%s<>%s<", qPrintable(m_ansiParamStr), qPrintable(m_ansiOscString));
                        
                        m_ansiState = ST_IDLE;
                        m_ansiOscString.clear();
                        m_ansiParamStr.clear();
                    }
                    else
                        m_ansiOscString += c;
                };break;
                case ST_CSI_PARAM:
                {
                    if(0x30 <= c && c <= 0x3F)
                    {
                        m_ansiParamStr += c;
                    }
                    else
                    {
                        i--;
                        m_ansiState = ST_CSI_INTER;
                    }
                };break;
                case ST_CSI_INTER:
                {
                    if(0x20 <= c && c <= 0x2F)
                    {
                        m_ansiInter += c;
                    }
                    else
                    {
                        switch(c.toLatin1())
                        {
                            case 'm':
                            {
                                //debugMsg("SGR:>%s<", qPrintable(m_ansiParamStr));

                                QStringList paramList =  m_ansiParamStr.split(';');
                                for(int i = 0;i < paramList.size();i++)
                                {
                                    int p = paramList[i].toInt();
                                    //debugMsg("c:%d", p);
                                    switch(p)
                                    {
                                        case 0:
                                        {
                                            m_fgColor = Qt::black;
                                            m_bgColor = Qt::white;
                                        };break;
                                        case 30: m_fgColor = Qt::black;break;
                                        case 31: m_fgColor = Qt::red;break;
                                        case 32: m_fgColor = Qt::green;break;
                                        case 33: m_fgColor = Qt::yellow;break;
                                        case 34: m_fgColor = Qt::blue;break;
                                        case 35: m_fgColor = Qt::magenta;break;
                                        case 36: m_fgColor = Qt::cyan;break;
                                        case 37: m_fgColor = Qt::white;break;
                                        default:;break;
                                    }
                                }
                                
                            };break;
                            case 'A': // CUU – Cursor Up
                            {
                                m_cursorY = std::max(0, m_cursorY-1);
                                m_cursorX = 0;
                            };break;
                            case 'B': // CUU – Cursor Down
                            {
                                m_cursorY = std::min(m_lines.size()-1, m_cursorY+1);
                                m_cursorX = 0;
                            };break;
                            case 'C': // CUF – Cursor Forward 
                            {
                                m_cursorX++;
                            };break;
                            case 'K': // EL – Erase in Line
                            {
                                int ansiParamVal = m_ansiParamStr.toInt();
                                if(ansiParamVal == 0) // erase from cursor and forward
                                {
                                    if(m_cursorY < m_lines.size())
                                    {
                                        Line &line = m_lines[m_cursorY];

                                        int charIdx = 0;
                                        for(int blkIdx = 0;blkIdx < line.size();blkIdx++)
                                        {
                                            Block *blk = &line[blkIdx];
                                            if(charIdx <= m_cursorX && m_cursorX < charIdx+blk->text.size()) 
                                            {
                                                int cutIdx = m_cursorX-charIdx;
                                                blk->text = blk->text.left(cutIdx);
                                            }
                                            charIdx += blk->text.size();
                                        }
                                    }
                                        
                                }
                            };break;
                            default:
                            {
                                warnMsg("Got unknown ANSI control sequence 'CSI %s %c'", qPrintable(m_ansiParamStr), c.toLatin1());

                            };break;
                        }
                        
                        QString ansiStr = m_ansiParamStr + m_ansiInter;
                        debugMsg("ANSI.CSI %c>%s<", c.toLatin1(), qPrintable(ansiStr));

                        

                        m_ansiParamStr.clear();
                        m_ansiInter.clear();
                        m_ansiState = ST_IDLE;
                    }
                };break;
                
            
            }
        }
    }

    update();
            
}


/**
 * @brief Copies content of console to clipboard.
 */
void ConsoleWidget::onCopyContent()
{
    QString text;
    for(int i = 0;i < m_lines.size();i++)
    {
        Line &line = m_lines[i];
        for(int lineIdx = 0;lineIdx < line.size();lineIdx++)
        {
            Block &blk = line[lineIdx];
            text += blk.text;
        }
        text += "\n";
    }
    QClipboard * clipboard = QApplication::clipboard();
    clipboard->setText(text);

}

void ConsoleWidget::onClearAll()
{
    clearAll();
}

void ConsoleWidget::mousePressEvent( QMouseEvent * event )
{
    if(event->button() == Qt::RightButton)
    {
        QPoint pos = event->globalPos();
        showPopupMenu(pos);
    }
}


void ConsoleWidget::showPopupMenu(QPoint pos)
{
    m_popupMenu.clear();

    // Add 'open'
    QAction *action;
    action = m_popupMenu.addAction("Copy");
    connect(action, SIGNAL(triggered()), this, SLOT(onCopyContent()));

    action = m_popupMenu.addAction("Clear All");
    connect(action, SIGNAL(triggered()), this, SLOT(onClearAll()));

    m_popupMenu.popup(pos);
}

void ConsoleWidget::keyPressEvent ( QKeyEvent * event )
{
    Core &core = Core::getInstance();

    debugMsg("%s()", __func__);

    switch(event->key())
    {
        case Qt::Key_End: core.writeTargetStdin("\033[F");break;
        case Qt::Key_Home: core.writeTargetStdin("\033[H");break;
        case Qt::Key_Left: core.writeTargetStdin("\033[D");break;
        case Qt::Key_Right: core.writeTargetStdin("\033[C");break;
        case Qt::Key_Up: core.writeTargetStdin("\033[A");break;
        case Qt::Key_Down: core.writeTargetStdin("\033[B");break;
        case Qt::Key_Backspace: core.writeTargetStdin("\x7f");break;
        default:
        {
            QString text = event->text();
            core.writeTargetStdin(text); 
        };break;
    }
}
    
