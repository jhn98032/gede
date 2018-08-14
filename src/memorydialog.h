#ifndef FILE_MEMORYDIALOG_H
#define FILE_MEMORYDIALOG_H

#include "ui_memorydialog.h"

#include <stdint.h>

#include <QDialog>


class MemoryDialog : public QDialog, public IMemoryWidget
{
    Q_OBJECT
public:
    MemoryDialog(QWidget *parent = NULL);

    virtual QByteArray getMemory(uint64_t startAddress, int count);
    void setStartAddress(uint64_t addr);

    void setConfig(Settings *cfg);

public slots:
    void onVertScroll(int pos);
    void onUpdate();

private:
    uint64_t inputTextToAddress(QString text);
    
private:
    Ui_MemoryDialog m_ui;
    uint64_t m_startScrollAddress; //!< The minimum address the user can scroll to.
};





#endif // FILE_MEMORYDIALOG_H

