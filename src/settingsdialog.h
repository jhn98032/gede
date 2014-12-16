#ifndef FILE__SETTINGSDIALOG_H
#define FILE__SETTINGSDIALOG_H

#include <QDialog>

#include "settings.h"
#include "ui_settingsdialog.h"


class SettingsDialog : public QDialog
{
    Q_OBJECT

public:

    SettingsDialog(QWidget *parent, Settings *cfg);

    void getConfig(Settings *cfg);
    
private:    
    void saveConfig();
    void loadConfig();
    void updateGui();
    

private slots:

    void onSelectFont();

private:
    Ui_SettingsDialog m_ui;
    Settings *m_cfg;

    QString m_settingsFontFamily;    
    int m_settingsFontSize;
};

#endif // FILE__SETTINGSDIALOG_H

