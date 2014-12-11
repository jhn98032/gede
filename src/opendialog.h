#ifndef FILE__ABOUTDIALOG_H
#define FILE__ABOUTDIALOG_H

#include <QDialog>

#include "ui_opendialog.h"


class OpenDialog : public QDialog
{
    Q_OBJECT

public:

    OpenDialog(QWidget *parent);

    void setProgram(QString program);
    void setArguments(QString program);
    QString getProgram();
    QString getArguments();
    

private slots:

    void onSelectProgram();

private:
    Ui_OpenDialog m_ui;
    
};

#endif // FILE__ABOUTDIALOG_H

