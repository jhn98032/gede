#ifndef FILE__CODEVIEWTAB_H
#define FILE__CODEVIEWTAB_H

#include "ui_codeviewtab.h"

#include <QWidget>

class CodeViewTab : public QWidget
{
Q_OBJECT

public:
    CodeViewTab(QWidget *parent);
    virtual ~CodeViewTab();

    void ensureLineIsVisible(int lineIdx);
    

    QString m_filepath;
    
//private:
    Ui_CodeViewTab m_ui;
   
};

#endif


