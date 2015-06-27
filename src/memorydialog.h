#ifndef FILE_MEMORYDIALOG_H
#define FILE_MEMORYDIALOG_H

#include "ui_memorydialog.h"
#include <QDialog>


class MemoryDialog : public QDialog, public IMemoryWidget
{
    Q_OBJECT
public:
    MemoryDialog(QWidget *parent = NULL);

    virtual QByteArray getMemory(unsigned int startAddress, int count);
    void setStartAddress(unsigned int addr);

    void setConfig(Settings *cfg);

public slots:
    void onVertScroll(int pos);
    void onUpdate();

private:
    Ui_MemoryDialog m_ui;

};





#endif // FILE_MEMORYDIALOG_H

