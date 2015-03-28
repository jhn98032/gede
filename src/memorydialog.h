#ifndef FILE_MEMORYDIALOG_H
#define FILE_MEMORYDIALOG_H

#include "ui_memorydialog.h"
#include <QDialog>


class MemoryDialog : public QDialog
{
    Q_OBJECT
public:
    MemoryDialog(QWidget *parent = NULL);

public slots:
    void onVertScroll(int pos);
    
private:
    Ui_MemoryDialog m_ui;

};





#endif // FILE_MEMORYDIALOG_H

