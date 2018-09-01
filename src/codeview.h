/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__CODEVIEW_H
#define FILE__CODEVIEW_H

#include <QWidget>
#include <QStringList>
#include "syntaxhighlightercxx.h"
#include "syntaxhighlighterbasic.h"
#include "syntaxhighlighterrust.h"
#include "settings.h"
#include <QTimer>
#include "variableinfowindow.h"


class ICodeView
{
    public:
    ICodeView(){};

    virtual void ICodeView_onRowDoubleClick(int lineNo) = 0;
    virtual void ICodeView_onContextMenu(QPoint pos, int lineNo, QStringList symbolList) = 0;
    virtual void ICodeView_onContextMenuIncFile(QPoint pos, int lineNo, QString incFile) = 0;
    

};


class CodeView : public QWidget
{
    Q_OBJECT

public:

    CodeView();
    virtual ~CodeView();

    typedef enum {CODE_CXX,CODE_BASIC,CODE_RUST} CodeType;
    
    void setPlainText(QString content, CodeType type);

    void setConfig(Settings *cfg);
    void paintEvent ( QPaintEvent * event );

    void setCurrentLine(int lineNo);
    void disableCurrentLine();
    
    void setInterface(ICodeView *inf) { m_inf = inf; };

    void setBreakpoints(QVector<int> numList);

    int getRowHeight();


    void incSearchStart(QString text);
    void incSearchNext();
    void incSearchPrev();
    
public slots:
    void onTimerTimeout();

    
private:
    int getBorderWidth();
    void mouseReleaseEvent( QMouseEvent * event );
    void mouseDoubleClickEvent( QMouseEvent * event );
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void focusOutEvent ( QFocusEvent * event );

public:
    QFont m_font;
    QFontMetrics *m_fontInfo;
    int m_cursorY;
    ICodeView *m_inf;
    QVector<int> m_breakpointList;
    SyntaxHighlighter *m_highlighter;
    Settings *m_cfg;
    QString m_text;
    QTimer m_timer;
    VariableInfoWindow m_infoWindow;


    int m_incSearchStartPosRow;
    int m_incSearchStartPosColumn;
    QString m_incSearchText;
    
};


#endif // FILE__CODEVIEW_H



