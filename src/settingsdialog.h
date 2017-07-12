/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

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
    void loadConfig();
    void updateGui();
    

private slots:

    void onSelectFont();
    void onSelectMemoryFont();
    void onSelectOutputFont();
    void onSelectGdbOutputFont();

    void showFontSelection(QString *fontFamily, int *fontSize);
    void onButtonBoxClicked(QAbstractButton* button);
    
private:
    Ui_SettingsDialog m_ui;
    Settings *m_cfg;

    QString m_settingsFontFamily;    
    int m_settingsFontSize;
    QString m_settingsMemoryFontFamily;    
    int m_settingsMemoryFontSize;
    QString m_settingsOutputFontFamily;    
    int m_settingsOutputFontSize;
    QString m_settingsGdbOutputFontFamily;    
    int m_settingsGdbOutputFontSize;

    QStringList m_sourceIgnoreDirs;
};

#endif // FILE__SETTINGSDIALOG_H

