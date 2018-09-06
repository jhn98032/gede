/*
 * Copyright (C) 2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__CONSOLEWIDGET_H
#define FILE__CONSOLEWIDGET_H

#include <QMenu>
#include <QTextEdit>
#include <QVector>
#include <QFont>
#include <QTimer>

#include "settings.h"


class ConsoleWidget : public QWidget
{
    Q_OBJECT
public:
    ConsoleWidget(QWidget *parent = NULL);
    virtual ~ConsoleWidget();

    void appendLog(QString text);

    void clearAll();

    void setMonoFont(QFont font);
    void setConfig(Settings *cfg);
    void setScrollBar(QScrollBar *verticalScrollBar);

public slots:
    void onCopyContent();
    void onClearAll();
    void onScrollBar_valueChanged(int value);
    void onTimerTimeout();

private:
    void decodeCSI(QChar c);
    void resizeEvent ( QResizeEvent * event );
    int getRowHeight();
    void insert(QChar c);
    void showPopupMenu(QPoint pos);
    void mousePressEvent( QMouseEvent * event );
    bool eventFilter(QObject *obj, QEvent *event);
    QColor getFgColor(int code);
    QColor getBgColor(int code);

    int getRowsPerScreen();
    void setOrigoY(int origoY );
    void updateScrollBars();
    
protected:
    void paintEvent ( QPaintEvent * event );
    void keyPressEvent ( QKeyEvent * event );

public:

    QFont m_font;
    QFontMetrics *m_fontInfo;
    
public:

    enum { ST_IDLE, ST_OSC_PARAM, ST_OSC_STRING, ST_SECBYTE, ST_CSI_PARAM, ST_CSI_INTER } m_ansiState;

    QString m_ansiParamStr;
    QString m_ansiInter;
    QString m_ansiOscString;
    int m_fgColor;
    int m_bgColor;

    struct Block
    {
        public:
        int m_fgColor;
        int m_bgColor;
        QString text;
    };
    typedef QVector <Block> Line;
    QVector <Line> m_lines;

    enum {STEADY, HIDDEN, BLINK_ON, BLINK_OFF} m_cursorMode;
    int m_cursorX; // Current cursor position column (0=first column)
    int m_cursorY; // Current cursor position row index (0=first row)
    int m_origoY; //!< Upper displayable line index for cursor pos 0 (0=first line)
    int m_dispOrigoY; //!< Upper displayable line (0=first line)
    QMenu m_popupMenu;
    Settings *m_cfg;
    QScrollBar *m_verticalScrollBar;
    QTimer m_timer;
};

#endif // FILE__CONSOLEWIDGET_H

