/*
 * Copyright (C) 2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__CONSOLEWIDGET_H
#define FILE__CONSOLEWIDGET_H

#include <QTextEdit>
#include <QVector>

class ConsoleWidget : public QWidget
{
    Q_OBJECT
public:
    ConsoleWidget(QWidget *parent = NULL);
    virtual ~ConsoleWidget();

    void appendLog(QString text);

    void clearAll();

    void setMonoFont(QFont font);
    
private:
    void flush();
    int getRowHeight();
    void insert(QChar c);
    
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
    QColor m_fgColor;
    QColor m_bgColor;

    struct Block
    {
        public:
        QColor m_fgColor;
        QColor m_bgColor;
        QString text;
    };
    typedef QVector <Block> Line;
    QVector <Line> m_lines;

    int m_cursorX;
    int m_cursorY;
};

#endif // FILE__CONSOLEWIDGET_H

