#include "settingsdialog.h"
#include "version.h"
#include "log.h"
#include "util.h"


#include <QFontDialog>


SettingsDialog::SettingsDialog(QWidget *parent, Settings *ini)
    : QDialog(parent)
    ,m_ini(ini)
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
    m_settingsFontFamily = m_ini->m_fontFamily;
    m_settingsFontSize = m_ini->m_fontSize;
}

void SettingsDialog::saveConfig()
{
    getConfig(m_ini);
}

void SettingsDialog::getConfig(Settings *ini)
{
    ini->m_fontFamily = m_settingsFontFamily;
    ini->m_fontSize = m_settingsFontSize;

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
