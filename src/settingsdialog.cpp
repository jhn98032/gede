/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "settingsdialog.h"
#include "version.h"
#include "log.h"
#include "util.h"


#include <QFontDialog>


SettingsDialog::SettingsDialog(QWidget *parent, Settings *cfg)
    : QDialog(parent)
    ,m_cfg(cfg)
{
    
    m_ui.setupUi(this);

    connect(m_ui.pushButton_selectFont, SIGNAL(clicked()), SLOT(onSelectFont()));
    connect(m_ui.pushButton_selectMemoryFont, SIGNAL(clicked()), SLOT(onSelectMemoryFont()));
    connect(m_ui.pushButton_selectOutputFont, SIGNAL(clicked()), SLOT(onSelectOutputFont()));

    QObject::connect(m_ui.buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButtonBoxClicked(QAbstractButton*)));



    loadConfig();
    
    updateGui();
    
}

void SettingsDialog::onButtonBoxClicked(QAbstractButton* button)
{
    // Clicked on the "Restore Defaults" button?
    if(m_ui.buttonBox->standardButton(button) == QDialogButtonBox::RestoreDefaults)
    {
        m_cfg->loadDefaultsGui();
        loadConfig();

        updateGui();
    }
}


void SettingsDialog::updateGui()
{
    QString labelText;
    labelText.sprintf("%s  %d", stringToCStr(m_settingsFontFamily), m_settingsFontSize);
    m_ui.pushButton_selectFont->setText(labelText);

    labelText.sprintf("%s  %d", stringToCStr(m_settingsMemoryFontFamily), m_settingsMemoryFontSize);
    m_ui.pushButton_selectMemoryFont->setText(labelText);

    labelText.sprintf("%s  %d", stringToCStr(m_settingsOutputFontFamily), m_settingsOutputFontSize);
    m_ui.pushButton_selectOutputFont->setText(labelText);
}    

void SettingsDialog::loadConfig()
{
    m_settingsFontFamily = m_cfg->m_fontFamily;
    m_settingsFontSize = m_cfg->m_fontSize;
    m_settingsMemoryFontFamily = m_cfg->m_memoryFontFamily;
    m_settingsMemoryFontSize = m_cfg->m_memoryFontSize;
    m_settingsOutputFontFamily = m_cfg->m_outputFontFamily;
    m_settingsOutputFontSize = m_cfg->m_outputFontSize;
}

void SettingsDialog::saveConfig()
{
    getConfig(m_cfg);
}

void SettingsDialog::getConfig(Settings *cfg)
{
    cfg->m_fontFamily = m_settingsFontFamily;
    cfg->m_fontSize = m_settingsFontSize;

    cfg->m_memoryFontFamily = m_settingsMemoryFontFamily;
    cfg->m_memoryFontSize = m_settingsMemoryFontSize;

    cfg->m_outputFontFamily = m_settingsOutputFontFamily;
    cfg->m_outputFontSize = m_settingsOutputFontSize;
}


    
void SettingsDialog::onSelectFont()
{
    showFontSelection(&m_settingsFontFamily, &m_settingsFontSize);
}

void SettingsDialog::showFontSelection(QString *fontFamily, int *fontSize)
{
    bool ok;
    QFont font = QFontDialog::getFont(
                 &ok, QFont(*fontFamily, *fontSize), this);
    if (ok)
    {
        *fontFamily = font.family();
        *fontSize = font.pointSize();

        updateGui();
    
    } else
    {
    }

}


void SettingsDialog::onSelectMemoryFont()
{
    showFontSelection(&m_settingsMemoryFontFamily, &m_settingsMemoryFontSize);
}


void SettingsDialog::onSelectOutputFont()
{
    showFontSelection(&m_settingsOutputFontFamily, &m_settingsOutputFontSize);
}

