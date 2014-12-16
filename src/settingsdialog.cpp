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

    loadConfig();
    
    updateGui();
    
}

void SettingsDialog::updateGui()
{
    QString labelText;
    labelText.sprintf("%s  %d", stringToCStr(m_settingsFontFamily), m_settingsFontSize);
    m_ui.pushButton_selectFont->setText(labelText);

}    

void SettingsDialog::loadConfig()
{
    m_settingsFontFamily = m_cfg->m_fontFamily;
    m_settingsFontSize = m_cfg->m_fontSize;
}

void SettingsDialog::saveConfig()
{
    getConfig(m_cfg);
}

void SettingsDialog::getConfig(Settings *cfg)
{
    cfg->m_fontFamily = m_settingsFontFamily;
    cfg->m_fontSize = m_settingsFontSize;

}


    
void SettingsDialog::onSelectFont()
{
        
    bool ok;
    QFont font = QFontDialog::getFont(
                 &ok, QFont(m_settingsFontFamily, m_settingsFontSize), this);
    if (ok)
    {
        m_settingsFontFamily = font.family();
        m_settingsFontSize = font.pointSize();

        updateGui();
    
    } else
    {
    }
}
