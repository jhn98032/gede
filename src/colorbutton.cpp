#include "colorbutton.h"
 #include <QPaintEvent>
 #include <QPainter>
 #include <QColorDialog>
 
MColorButton::MColorButton(QWidget *parent)
    : QPushButton(parent)
     ,m_clr(Qt::black)
{
    connect(this, SIGNAL(clicked()), SLOT(onButtonClicked()));

}

MColorButton::~MColorButton()
{

}

/**
 * @brief Sets the current color.
 */
void MColorButton::setColor ( QColor clr)
{
    m_clr = clr;
    update();
}

/**
 * @brief Returns the current selected color.
 */
QColor MColorButton::getColor()
{
    return m_clr;
}

/**
 * @brief Paint function.
 */
void MColorButton::paintEvent ( QPaintEvent * e )
{
    QPushButton::paintEvent(e);
    QPainter painter(this);

    QRect size(10,6,frameSize().width()-20,frameSize().height()-12);
    painter.fillRect(size, QBrush(m_clr));

    painter.setPen(Qt::black);
    painter.drawRect(size);


//    QRect textRect = QRect(QPoint(10,0), frameSize());
//    painter.drawText( textRect, Qt::AlignVCenter | Qt::AlignLeft, "Example");
   
}


/**
 * @brief Shows a dialog for selecting a color.
 */
void MColorButton::showColorSelectDialog(QColor *color)
{
    QColor newColor;
    newColor = QColorDialog::getColor(*color, this);

    if(newColor.isValid())
        *color = newColor;

}


/**
 * @brief The user has pressed the button.
 */  
void MColorButton::onButtonClicked()
{
    showColorSelectDialog(&m_clr);
    
}

 
