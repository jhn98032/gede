#include "variableinfowindow.h"

#include <QFontMetrics>

#include "core.h"


VariableInfoWindow::VariableInfoWindow(QFont *font)
    : QWidget()
    ,m_font(font)
{
    // setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(windowFlags() | Qt::ToolTip); //Qt::WindowStaysOnTopHint);

    //setFrameStyle(QFrame::Panel | QFrame::Raised);

}


VariableInfoWindow::~VariableInfoWindow()
{

}


void VariableInfoWindow::hide()
{
    m_expr = "";
    QWidget::hide();
}

void VariableInfoWindow::show(QString expr)
{
    Core &core = Core::getInstance();

    if(expr != m_expr)
    {
    QString value;

    core.evaluateExpression(expr, &value);

    m_expr = expr;
    m_text = m_expr + "=" + value;
    if(m_text.length() > 120)
        m_text = m_text.left(120) + "...";
        
    QFontMetrics m_fontInfo(*m_font);
    int textHeight = m_fontInfo.lineSpacing()+1;

    int w = 10 + m_fontInfo.width(m_text)+10;
    int h = 5 + textHeight + 5;

    resize(w,h);

    }   
    QWidget::show();
}

void drawFrame(QPainter &paint, const QRect &r)
{
    QLine lines[4];
    lines[0] = QLine(r.topLeft(), r.topRight());
    lines[1] = QLine(r.topRight(), r.bottomRight());
    lines[2] = QLine(r.bottomLeft(), r.bottomRight());
    lines[3] = QLine(r.topLeft(), r.bottomLeft());
    
    paint.drawLines(lines,4);
}

void VariableInfoWindow::paintEvent(QPaintEvent *pe)
{
    Q_UNUSED(pe);


    QRect border = QRect(QPoint(0,0), size());

    // Create painter
    QPainter paint;   
    paint.begin(this);

    // Set font
    paint.setFont(*m_font);
    QFontMetrics m_fontInfo(*m_font);
    
    // Draw text
    paint.setPen(Qt::black);
    paint.drawText(10, 5+m_fontInfo.ascent(), m_text);

    // Draw widget frame
    paint.setPen(QColor(0x45,0x45,0x45));
    drawFrame(paint, border); 
    paint.setPen(QColor(0xa3,0xa3,0xa3));
    paint.drawLine(1,1, 1, height()-2);
    paint.drawLine(1, height()-2, width()-2, height()-2);
    paint.setPen(QColor(0x87,0x87,0x87));
    paint.drawLine(2,1, width()-2, 1);
    paint.drawLine(width()-2, 1, width()-2, height()-3);

    paint.end();

}

void VariableInfoWindow::resizeEvent(QResizeEvent *re)
{
    Q_UNUSED(re);
    setMask(rect());

    //QWidget::resizeEvent(re);
}



