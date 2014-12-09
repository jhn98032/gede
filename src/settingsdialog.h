#ifndef FILE__SETTINGSDIALOG_H
#define FILE__SETTINGSDIALOG_H

#include <QDialog>

#include "ini.h"
#include "ui_settingsdialog.h"


class SettingsDialog : public QDialog
{
    Q_OBJECT

public:

    SettingsDialog(QWidget *parent, Ini *ini);

    void getConfig(Ini *ini);
    
private:    
    void saveConfig();
    void loadConfig();
    

private slots:

    void onSelectFont();

private:
    Ui_SettingsDialog m_ui;
    Ini *m_ini;

    QString m_settingsFontFamily;    
    int m_settingsFontSize;
};

#endif // FILE__SETTINGSDIALOG_H

