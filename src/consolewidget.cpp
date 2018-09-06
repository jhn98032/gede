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
#include <QScrollBar>

//#define ENABLE_DEBUGMSG

#include "log.h"
#include "core.h"


#define ASCII_ESC           0x1B
#define ASCII_BELL          0x7
#define ASCII_BACKSPACE     '\b'
#define ANSI_CSI           "\033["

static QColor red(255,0,0);

ConsoleWidget::ConsoleWidget(QWidget *parent)
    : QWidget(parent)
    ,m_fontInfo(NULL)
    ,m_ansiState(ST_IDLE)
    ,m_cursorMode(STEADY)
    ,m_origoY(0)
    ,m_dispOrigoY(0)
    ,m_cfg(NULL)
    ,m_verticalScrollBar(NULL)
{
    m_cursorX = 0;
    m_cursorY = 0;

    m_fgColor = -1;
    m_bgColor = -1;

    setMonoFont(QFont("Monospace", 18));

  
    setFocusPolicy(Qt::StrongFocus);

    installEventFilter(this);

    setMinimumSize(400,getRowHeight()*2);

    m_timer.setInterval(500);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
    
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
    debugMsg("%s()", __func__);
    
    m_lines.clear();
    m_origoY = 0;
    m_dispOrigoY = 0;
    m_cursorX = 0;
    m_cursorY = 0;
}

void ConsoleWidget::setConfig(Settings *cfg)
{
    m_cfg = cfg;
}
    

/**
 * @brief Returns the height of a text row in pixels.
 */
int ConsoleWidget::getRowHeight()
{
    int rowHeight = m_fontInfo->lineSpacing()+1;
    return rowHeight;
}

QColor ConsoleWidget::getBgColor(int code)
{
    QColor clr = Qt::black;
    if(!m_cfg)
        return clr;
    
    if(40 <= code && code <= 47)
        clr = m_cfg->m_progConColorNorm[code-40];
    else if(100 <= code && code <= 107)
        clr = m_cfg->m_progConColorBright[code-100];
    else
        clr = m_cfg->m_progConColorBg;
    return clr;
}

QColor ConsoleWidget::getFgColor(int code)
{
    QColor clr = Qt::black;
    if(!m_cfg)
        return clr;
    
    if(30 <= code && code <= 37)
        clr = m_cfg->m_progConColorNorm[code-30];
    else if(90 <= code && code <= 97)
        clr = m_cfg->m_progConColorBright[code-90];
    else
        clr = m_cfg->m_progConColorFg;

    return clr;
}


/**
 * @brief Returns the number of text rows that the widget can show at a time.
 */
int ConsoleWidget::getRowsPerScreen()
{
    int rowHeight = getRowHeight();
    int rowCount = (size().height()+(rowHeight-1))/rowHeight;
    return rowCount;
}
    
void ConsoleWidget::paintEvent ( QPaintEvent * event )
{
    int rowHeight = getRowHeight();
    QPainter painter(this);



    if(m_cfg)
        painter.fillRect(event->rect(), m_cfg->m_progConColorBg);

    painter.setFont(m_font);

    // Display cursor
    if(m_cursorMode == BLINK_ON || m_cursorMode == STEADY)
    {
        QRect r;
        int charWidth = m_fontInfo->width(" ");
        r.setX(charWidth*m_cursorX+5);
        r.setY(rowHeight*((m_cursorY+m_origoY)-m_dispOrigoY));
        r.setWidth(charWidth);
        r.setHeight(rowHeight);
        if(m_cfg)
        {
            if(hasFocus())
                painter.fillRect(r, QBrush(m_cfg->m_progConColorCursor));
            else
            {
                painter.setPen(m_cfg->m_progConColorCursor);
                painter.drawRect(r);
            }

        }
    }

    // Display text
    for(int rowIdx = m_dispOrigoY;rowIdx < m_lines.size();rowIdx++)
    {
        Line &line = m_lines[rowIdx];

        int y = rowHeight*(rowIdx-m_dispOrigoY);
        int x = 5;
        int curCharIdx = 0;
        
        // Draw line number
        int fontY = y+(rowHeight-(m_fontInfo->ascent()+m_fontInfo->descent()))/2+m_fontInfo->ascent();
    
        for(int blockIdx = 0;blockIdx < line.size();blockIdx++)
        {
            Block &blk = line[blockIdx];
            painter.setPen(getFgColor(blk.m_fgColor));
            if((m_cursorY+m_origoY) == rowIdx && curCharIdx <= m_cursorX && m_cursorX < curCharIdx+blk.text.size())
            {
                int cutIdx = m_cursorX-curCharIdx;
                QString leftText = blk.text.left(cutIdx);
                QChar cutLetter = blk.text[cutIdx];
                QString rightText = blk.text.mid(cutIdx+1);
                painter.drawText(x, fontY, leftText);
                painter.setPen(getBgColor(blk.m_bgColor));
                painter.drawText(x+m_fontInfo->width(leftText), fontY, QString(cutLetter));
                painter.setPen(getFgColor(blk.m_fgColor));
                painter.drawText(x+m_fontInfo->width(leftText + cutLetter), fontY, rightText);

            }
            else
            {
                //debugMsg("drawing '%s' at %d:%d", qPrintable(blk.text), x, fontY);
                painter.drawText(x, fontY, blk.text);
            }
            x += m_fontInfo->width(blk.text);
            curCharIdx += blk.text.size();
        }
        
    }


}

void ConsoleWidget::insert(QChar c)
{
    debugMsg("%s('%s')", __func__, c == '\n' ? "\\n" : qPrintable(QString(c)));
    
    // Insert missing lines?
    while(m_lines.size() <= (m_cursorY+m_origoY))
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
        if(m_cursorY+m_origoY+1 == m_lines.size())
        {
            Line line;
            Block blk;
            blk.m_fgColor = m_fgColor;
            blk.m_bgColor = m_bgColor;
            line.append(blk);
            m_lines.append(line);
        }

        if(m_cursorY+1 >= getRowsPerScreen())
        {
            m_origoY = (m_lines.size()-1 - getRowsPerScreen() + 1);
        }
        else
            m_cursorY++;
        m_cursorX = 0;


        
    }
    else
    {
        Line &line = m_lines[m_cursorY+m_origoY];
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
                updateScrollBars();
                return ;
            }
            x += blk->text.size();
        }


        // Cursor outside of existing line content?
        if(m_cursorX >= 0)
        {
            Block blk;
            blk.m_fgColor = m_fgColor;
            blk.m_bgColor = m_bgColor;
            if(m_cursorX > x)
                blk.text = QString((int)(m_cursorX-x), QChar(' '));
            blk.text += c;
            line.append(blk);
            m_cursorX++;
        }
        
    }

    updateScrollBars();
}

void ConsoleWidget::updateScrollBars()
{
    if(m_verticalScrollBar)
    {
        debugMsg("setting range to %d", m_lines.size()-1);
        int rangeMax = qMax(0, m_lines.size()-1-getRowsPerScreen()+1);
        m_verticalScrollBar->setRange(0, rangeMax);
        int newPos = qMax(0, m_lines.size()-1-getRowsPerScreen()+1);
        m_verticalScrollBar->setValue(newPos);
    }
}


/**
 * @brief Dispatches a ANSI escape sequence.
 */
void ConsoleWidget::decodeCSI(QChar c)
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
                        m_fgColor = -1;
                        m_bgColor = -1;
                    };break;
                    case 40:
                    case 41:
                    case 42:
                    case 43:
                    case 44:
                    case 45:
                    case 46:
                    case 47:
                    case 100:
                    case 101:
                    case 102:
                    case 103:
                    case 104:
                    case 105:
                    case 106:
                    case 107:
                        m_bgColor = p;break;
                    case 30:
                    case 31:
                    case 32:
                    case 33:
                    case 34:
                    case 35:
                    case 36:
                    case 37:
                    case 90:
                    case 91:
                    case 92:
                    case 93:
                    case 94:
                    case 95:
                    case 96:
                    case 97:
                        m_fgColor = p;break;
                    default:;break;
                }
            }
            
        };break;
        case 'A': // CUU - Cursor Up
        {
            m_cursorY = qMax(0, m_cursorY-1);
            m_cursorX = 0;
        };break;
        case 'B': // CUU - Cursor Down
        {
            m_cursorY = qMin(m_cursorY+1, getRowsPerScreen()-1);
            m_cursorX = 0;
        };break;
        case 'C': // CUF - Cursor Forward 
        {
            m_cursorX++;
        };break;
        case 'D': // CUB - Cursor Back
        {
            m_cursorX = qMax(0, m_cursorX-1);
        };break;
        case 'J':
        {
            int ansiParamVal = m_ansiParamStr.toInt();
            // Erase screen
            if(ansiParamVal == 2)
            {
                debugMsg("erasing %d", getRowsPerScreen());
                // Fill the visible part with newlines
                for(int i4 = 0;i4 < getRowsPerScreen();i4++)
                    insert('\n');
            }
            else
                warnMsg("Got unknown ANSI control sequence 'CSI %s %c'",
                        qPrintable(m_ansiParamStr), c.toLatin1());
            
        };break;
        case 'K': // EL - Erase in Line
        {
            int ansiParamVal = m_ansiParamStr.toInt();
            if(ansiParamVal == 0) // erase from cursor and forward
            {
                if((m_cursorY+m_origoY) < m_lines.size())
                {
                    Line &line = m_lines[m_cursorY+m_origoY];

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
        case 'P': // Delete character
        {
            //int ansiParamVal = m_ansiParamStr.toInt();
            {
                if(m_cursorY+m_origoY < m_lines.size())
                {
                    Line &line = m_lines[m_cursorY+m_origoY];

                    int charIdx = 0;
                    for(int blkIdx = 0;blkIdx < line.size();blkIdx++)
                    {
                        Block *blk = &line[blkIdx];
                        if(charIdx <= m_cursorX && m_cursorX < charIdx+blk->text.size()) 
                        {
                            int cutIdx = m_cursorX-charIdx;
                            blk->text.remove(cutIdx, 1);
                            blkIdx = line.size();
                        }
                        charIdx += blk->text.size();
                    }
                }
                    
            }
        };break;
        case 'h':
        {
            // DECCKM: Cursor key mode: "Set: application sequences".
            if(m_ansiParamStr == "?1")
            {

            }
            // DECTCEM: Makes the cursor visible ("ESC[?25h")
            else if(m_ansiParamStr == "?25")
            {
                debugMsg("show cursor!");
                m_cursorMode = BLINK_ON;
            }
            // AT&T 610: Blink cursor ("ESC[?12h")
            else if(m_ansiParamStr == "?12")
            {
                debugMsg("blink cursor!");
                m_cursorMode = BLINK_ON;
            }
            else
                warnMsg("Got unknown ANSI control sequence 'CSI %s %c'",
                        qPrintable(m_ansiParamStr), c.toLatin1());
        };break;
        case 'l':
        {

            // DECTCEM: Makes the cursor invisible ("ESC[?25l")
            if(m_ansiParamStr == "?25")
            {
                debugMsg("hide cursor!");
                m_cursorMode = HIDDEN;
            }
            // DECCKM: Cursor key mode: "Reset: cursor sequences"
            else if(m_ansiParamStr == "?1")
            {

            }
            // AT&T 610: Steady cursor ("ESC[?12l")
            else if(m_ansiParamStr == "?12")
            {
                debugMsg("steady cursor!");
                m_cursorMode = STEADY;
            }
            else
                warnMsg("Got unknown ANSI control sequence 'CSI %s %c'",
                        qPrintable(m_ansiParamStr), c.toLatin1());
        };break;
        // Cursor home
        case 'H':
        {
            QStringList paramList =  m_ansiParamStr.split(';');
            // Set current cursor position
            if(paramList.size() == 2)
            {
                int row = paramList[0].toInt();
                int column = paramList[1].toInt();  

                m_cursorY  = qBound(0, row-1, 9999);
                m_cursorX = qBound(0, column-1, 9999);

                debugMsg("Setting cursor to L%dC%d", m_cursorY, m_cursorX);
            }
            else
                warnMsg("Got unknown ANSI control sequence 'CSI %s %c'",
                        qPrintable(m_ansiParamStr), c.toLatin1());

        };break;
        case 'n':
        {
            // Request cursor position?
            if(m_ansiParamStr == "6")
            {
                QString resp;
                resp.sprintf(ANSI_CSI "%d;%dR", m_cursorY+1, m_cursorX+1);
                Core &core = Core::getInstance();
                core.writeTargetStdin(resp);
                debugMsg("Sending cursor position to target 'CSI%s'", qPrintable(resp.mid(2)));
            }
            else
                warnMsg("Got unknown ANSI control sequence 'CSI %s %c'",
                        qPrintable(m_ansiParamStr), c.toLatin1());

        };break; 
        default:
        {
            warnMsg("Got unknown ANSI control sequence 'CSI %s %c'", qPrintable(m_ansiParamStr), c.toLatin1());

        };break;
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
                        decodeCSI(c);
                        
                        
                        QString ansiStr = m_ansiParamStr + m_ansiInter;
                        debugMsg(" ANSI.CSI %c>%s<", c.toLatin1(), qPrintable(ansiStr));

                        

                        m_ansiParamStr.clear();
                        m_ansiInter.clear();
                        m_ansiState = ST_IDLE;
                    }
                };break;
                
            
            }
        }
    }

    // Remove oldest history
    if(m_cfg)
    {
        int linesToRemove = 0;
        if(m_lines.size() > m_cfg->m_progConScrollback)
            linesToRemove = m_lines.size() - std::max(2, m_cfg->m_progConScrollback);
        if(linesToRemove > 0)
        {
            m_lines.remove(0, linesToRemove);
            m_origoY -= linesToRemove;
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

bool ConsoleWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==this)
    {
        if (event->type() == QEvent::KeyPress)
        {

            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

            if(keyEvent->key() == Qt::Key_Tab)
            {
                Core &core = Core::getInstance();
                QString text = "\t";
                core.writeTargetStdin(text); 
      
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        // pass the event on to the parent class
        return QWidget::eventFilter(obj, event);
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
    assert(m_cfg != NULL);
    
    debugMsg("%s()", __func__);

    switch(event->key())
    {
        case Qt::Key_End: core.writeTargetStdin("\033[F");break;
        case Qt::Key_Home: core.writeTargetStdin("\033[H");break;
        case Qt::Key_Left: core.writeTargetStdin("\033[D");break;
        case Qt::Key_Right: core.writeTargetStdin("\033[C");break;
        case Qt::Key_Up: core.writeTargetStdin("\033[A");break;
        case Qt::Key_Down: core.writeTargetStdin("\033[B");break;
        case Qt::Key_Backspace:
        {
            if(m_cfg)
            {
                switch(m_cfg->m_progConBackspaceKey)
                {
                    default:
                    case 0: core.writeTargetStdin("\x7f");break;
                    case 1: core.writeTargetStdin("\x08");break;
                    case 2: core.writeTargetStdin("\033[3~");break;
                };break;
            }
        };break;
        case Qt::Key_Delete:
        {
            if(m_cfg)
            {
                switch(m_cfg->m_progConDelKey)
                {
                    default:
                    case 0: core.writeTargetStdin("\x7f");break;
                    case 1: core.writeTargetStdin("\x08");break;
                    case 2: core.writeTargetStdin("\033[3~");break;
                };break;
            }
            
        };break;
        default:
        {
            QString text = event->text();
            core.writeTargetStdin(text); 
        };break;
    }
}
    

void ConsoleWidget::setScrollBar(QScrollBar *verticalScrollBar)
{
    m_verticalScrollBar = verticalScrollBar;

    
}


void ConsoleWidget::resizeEvent ( QResizeEvent * event )
{
    Q_UNUSED(event);
    
    debugMsg("%s(%dx%d)", __func__, size().width(), size().height());
    if(m_verticalScrollBar)
    {
        m_verticalScrollBar->setRange(0, m_lines.size()-1);
    }
}


void ConsoleWidget::onScrollBar_valueChanged(int value)
{
    debugMsg("%s(value:%d)", __func__, value);
    m_dispOrigoY = value;
    update();
}


void ConsoleWidget::onTimerTimeout()
{
    debugMsg("%s()", __func__);
    if(m_cursorMode == BLINK_OFF)
        m_cursorMode = BLINK_ON;
    else if(m_cursorMode == BLINK_ON)
        m_cursorMode = BLINK_OFF;
    update();
}




