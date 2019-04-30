#ifndef FILE_MEMORYDIALOG_H
#define FILE_MEMORYDIALOG_H

#include "ui_memorydialog.h"


#include <QDialog>
#include <QWheelEvent>


class MemoryDialog : public QDialog, public IMemoryWidget
{
    Q_OBJECT
public:
    MemoryDialog(QWidget *parent = NULL);

    virtual QByteArray getMemory(quint64 startAddress, int count);
    void setStartAddress(quint64 addr);

    void setConfig(Settings *cfg);

public slots:
    void onVertScroll(int pos);
    void onUpdate();

private:
    quint64 inputTextToAddress(QString text);
    void wheelEvent(QWheelEvent * event);
    
private:
    Ui_MemoryDialog m_ui;
    quint64 m_startScrollAddress; //!< The minimum address the user can scroll to.
};





#endif // FILE_MEMORYDIALOG_H

