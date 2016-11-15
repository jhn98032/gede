#ifndef FILE__COLORBUTTON_H
#define FILE__COLORBUTTON_H

#include <QPushButton>

/**
* @brief A button used to select colors. Pressing the button triggers a color select dialog.
*
* The button will have a square in the selected color drawn on top of the button.
*/
class MColorButton : public QPushButton
{
    Q_OBJECT

public:

    MColorButton(QWidget *parent);
    virtual ~MColorButton();

    void setColor(QColor clr);
    QColor getColor();

private:
    virtual void paintEvent ( QPaintEvent * e );
    void showColorSelectDialog(QColor *color);

private slots:
    void onButtonClicked();
    
private:
    QColor m_clr;
};

#endif // FILE__COLORBUTTON_H
