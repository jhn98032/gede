#ifndef FILE__CODEVIEW_H
#define FILE__CODEVIEW_H

#include <QWidget>
#include <QStringList>
#include "syntaxhighlighter.h"
#include "settings.h"

class ICodeView
{
    public:
    ICodeView(){};

    virtual void ICodeView_onRowDoubleClick(int lineNo) = 0;
    virtual void ICodeView_onContextMenu(QPoint pos, int lineNo, QStringList text) = 0;
    

};


class CodeView : public QWidget
{
    Q_OBJECT

public:

    CodeView();
    virtual ~CodeView();
    
    void setPlainText(QString text);

    void setConfig(Settings *cfg);
    void paintEvent ( QPaintEvent * event );

    void setCurrentLine(int lineNo);
    void disableCurrentLine();
    
    void setInterface(ICodeView *inf) { m_inf = inf; };

    void setBreakpoints(QVector<int> numList);

    int getRowHeight();
    
private:
    void mouseReleaseEvent( QMouseEvent * event );
    void mouseDoubleClickEvent( QMouseEvent * event );
    void mousePressEvent(QMouseEvent * event);

public:
    QFont m_font;
    QFontMetrics *m_fontInfo;
    int m_cursorY;
    ICodeView *m_inf;
    QVector<int> m_breakpointList;
    SyntaxHighlighter m_highlighter;
    Settings *m_cfg;
};


#endif // FILE__CODEVIEW_H



