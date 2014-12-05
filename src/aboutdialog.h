#ifndef FILE__ABOUTDIALOG_H
#define FILE__ABOUTDIALOG_H

#include <QDialog>

#include "ui_aboutdialog.h"


class AboutDialog : public QDialog
{
    Q_OBJECT

    public:

    AboutDialog(QWidget *parent);



    Ui_AboutDialog m_ui;
    
};

#endif // FILE__ABOUTDIALOG_H

