/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "settingsdialog.h"
#include "version.h"
#include "log.h"
#include "util.h"

#include <QStyleFactory>
#include <QFontDialog>


SettingsDialog::SettingsDialog(QWidget *parent, Settings *cfg)
    : QDialog(parent)
    ,m_cfg(cfg)
{
    
    m_ui.setupUi(this);

    connect(m_ui.pushButton_selectFont, SIGNAL(clicked()), SLOT(onSelectFont()));
    connect(m_ui.pushButton_selectMemoryFont, SIGNAL(clicked()), SLOT(onSelectMemoryFont()));
    connect(m_ui.pushButton_selectOutputFont, SIGNAL(clicked()), SLOT(onSelectOutputFont()));
    connect(m_ui.pushButton_selectGdbOutputFont, SIGNAL(clicked()), SLOT(onSelectGdbOutputFont()));

    QObject::connect(m_ui.buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButtonBoxClicked(QAbstractButton*)));

    QStyleFactory sf;

    QStringList styleList = sf.keys();
    for(int i = 0;i < styleList.size();i++)
    {
        m_ui.comboBox_guiStyle->insertItem(i, styleList[i], QVariant(styleList[i]));
    }
    m_ui.comboBox_guiStyle->insertItem(0, "Qt default", QVariant(QString("")));

    m_ui.comboBox_currentLineStyle->insertItem(0, "Hollow rectangle", QVariant((int)Settings::HOLLOW_RECT));
    m_ui.comboBox_currentLineStyle->insertItem(1, "Filled rectangle", QVariant((int)Settings::FILLED_RECT));
   

    loadConfig();
    
    updateGui();
    
}

void SettingsDialog::onButtonBoxClicked(QAbstractButton* button)
{
    // Clicked on the "Restore Defaults" button?
    if(m_ui.buttonBox->standardButton(button) == QDialogButtonBox::RestoreDefaults)
    {
        m_cfg->loadDefaultsGui();
        m_cfg->loadDefaultsAdvanced();
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

    labelText.sprintf("%s  %d", stringToCStr(m_settingsGdbOutputFontFamily), m_settingsGdbOutputFontSize);
    m_ui.pushButton_selectGdbOutputFont->setText(labelText);
}    

void SettingsDialog::loadConfig()
{
    m_ui.spinBox_variableInfoWindowDelay->setValue(m_cfg->m_variablePopupDelay);

    m_settingsFontFamily = m_cfg->m_fontFamily;
    m_settingsFontSize = m_cfg->m_fontSize;
    m_settingsMemoryFontFamily = m_cfg->m_memoryFontFamily;
    m_settingsMemoryFontSize = m_cfg->m_memoryFontSize;
    m_settingsOutputFontFamily = m_cfg->m_outputFontFamily;
    m_settingsOutputFontSize = m_cfg->m_outputFontSize;
    m_settingsGdbOutputFontFamily = m_cfg->m_gdbOutputFontFamily;
    m_settingsGdbOutputFontSize = m_cfg->m_gdbOutputFontSize;

    m_ui.spinBox_tabIndent->setValue(m_cfg->getTabIndentCount());

    m_ui.spinBox_maxTabs->setValue(m_cfg->m_maxTabs);

    m_ui.lineEdit_sourceIgnoreDirs->setText(m_cfg->m_sourceIgnoreDirs.join(";"));

    m_ui.pushButton_clr_background->setColor(m_cfg->m_clrBackground);
    m_ui.pushButton_clr_comment->setColor(m_cfg->m_clrComment);
    m_ui.pushButton_clr_string->setColor(m_cfg->m_clrString);
    m_ui.pushButton_clr_incString->setColor(m_cfg->m_clrIncString);
    m_ui.pushButton_clr_keyword->setColor(m_cfg->m_clrKeyword);
    m_ui.pushButton_clr_cppKeyword->setColor(m_cfg->m_clrCppKeyword);
    m_ui.pushButton_clr_curLine->setColor(m_cfg->m_clrCurrentLine);
    m_ui.pushButton_clr_number->setColor(m_cfg->m_clrNumber);
    m_ui.pushButton_clr_foreground->setColor(m_cfg->m_clrForeground);

    m_ui.checkBox_showLineNo->setCheckState(m_cfg->m_showLineNo ? Qt::Checked : Qt::Unchecked);

    m_ui.checkBox_showLineNumbers->setCheckState(m_cfg->m_tagShowLineNumbers ? Qt::Checked : Qt::Unchecked);

    m_ui.checkBox_enableDebugLog->setCheckState(m_cfg->m_enableDebugLog ? Qt::Checked : Qt::Unchecked);

    int guiStyleIdx = m_ui.comboBox_guiStyle->findData(QVariant(m_cfg->m_guiStyleName));
    if(guiStyleIdx != -1)
        m_ui.comboBox_guiStyle->setCurrentIndex(guiStyleIdx);

    int lineStyleIdx = m_ui.comboBox_currentLineStyle->findData(QVariant((int)m_cfg->m_currentLineStyle));
    if(lineStyleIdx != -1)
        m_ui.comboBox_currentLineStyle->setCurrentIndex(lineStyleIdx);


    m_ui.spinBox_progConScrollback->setValue(m_cfg->m_progConScrollback);

    m_ui.pushButton_progConBgClr->setColor(m_cfg->m_progConColorBg); 
    m_ui.pushButton_progConFgClr->setColor(m_cfg->m_progConColorFg); 
    m_ui.pushButton_progConCursorClr->setColor(m_cfg->m_progConColorCursor); 

    m_ui.pushButton_progConClrNormal1->setColor(m_cfg->m_progConColorNorm[0]);
    m_ui.pushButton_progConClrNormal2->setColor(m_cfg->m_progConColorNorm[1]);
    m_ui.pushButton_progConClrNormal3->setColor(m_cfg->m_progConColorNorm[2]);
    m_ui.pushButton_progConClrNormal4->setColor(m_cfg->m_progConColorNorm[3]);
    m_ui.pushButton_progConClrNormal5->setColor(m_cfg->m_progConColorNorm[4]);
    m_ui.pushButton_progConClrNormal6->setColor(m_cfg->m_progConColorNorm[5]);
    m_ui.pushButton_progConClrNormal7->setColor(m_cfg->m_progConColorNorm[6]);
    m_ui.pushButton_progConClrNormal8->setColor(m_cfg->m_progConColorNorm[7]);

    m_ui.pushButton_progConClrBright1->setColor(m_cfg->m_progConColorBright[0]);
    m_ui.pushButton_progConClrBright2->setColor(m_cfg->m_progConColorBright[1]);
    m_ui.pushButton_progConClrBright3->setColor(m_cfg->m_progConColorBright[2]);
    m_ui.pushButton_progConClrBright4->setColor(m_cfg->m_progConColorBright[3]);
    m_ui.pushButton_progConClrBright5->setColor(m_cfg->m_progConColorBright[4]);
    m_ui.pushButton_progConClrBright6->setColor(m_cfg->m_progConColorBright[5]);
    m_ui.pushButton_progConClrBright7->setColor(m_cfg->m_progConColorBright[6]);
    m_ui.pushButton_progConClrBright8->setColor(m_cfg->m_progConColorBright[7]);

    m_ui.comboBox_backspaceKey->setCurrentIndex(m_cfg->m_progConBackspaceKey);
    m_ui.comboBox_deleteKey->setCurrentIndex(m_cfg->m_progConDelKey);

    m_ui.checkBox_globalProjConfig->setCheckState(m_cfg->m_globalProjConfig ? Qt::Checked : Qt::Unchecked);
    
            
    int comboIdx = 0;
    if(m_cfg->m_tagSortByName)
        comboIdx = 1;
    else
        comboIdx = 0;
    m_ui.comboBox_sortTags->setCurrentIndex(comboIdx);
}

void SettingsDialog::getConfig(Settings *cfg)
{
    m_cfg->m_variablePopupDelay = m_ui.spinBox_variableInfoWindowDelay->value();

    cfg->m_fontFamily = m_settingsFontFamily;
    cfg->m_fontSize = m_settingsFontSize;

    cfg->m_memoryFontFamily = m_settingsMemoryFontFamily;
    cfg->m_memoryFontSize = m_settingsMemoryFontSize;

    cfg->m_outputFontFamily = m_settingsOutputFontFamily;
    cfg->m_outputFontSize = m_settingsOutputFontSize;

    cfg->m_gdbOutputFontFamily = m_settingsGdbOutputFontFamily;
    cfg->m_gdbOutputFontSize = m_settingsGdbOutputFontSize;

    cfg->m_maxTabs = m_ui.spinBox_maxTabs->value();

    cfg->m_tabIndentCount = m_ui.spinBox_tabIndent->value();

    cfg->m_sourceIgnoreDirs = m_ui.lineEdit_sourceIgnoreDirs->text().split(';');

    cfg->m_showLineNo = (m_ui.checkBox_showLineNo->checkState() == Qt::Unchecked) ? false : true;

    cfg->m_tagShowLineNumbers = (m_ui.checkBox_showLineNumbers->checkState() == Qt::Unchecked) ? false : true;

    cfg->m_enableDebugLog = (m_ui.checkBox_enableDebugLog->checkState() == Qt::Unchecked) ? false : true;

    int guiStyleIdx = m_ui.comboBox_guiStyle->currentIndex();
    if(guiStyleIdx != -1)
        m_cfg->m_guiStyleName = m_ui.comboBox_guiStyle->itemData(guiStyleIdx).toString();

    int lineStyleIdx = m_ui.comboBox_currentLineStyle->currentIndex();
    if(lineStyleIdx != -1)
    {
        int intLineStyle = m_ui.comboBox_currentLineStyle->itemData(lineStyleIdx).toInt();
        m_cfg->m_currentLineStyle = (Settings::CurrentLineStyle)intLineStyle;
    }
    



    cfg->m_clrBackground = m_ui.pushButton_clr_background->getColor();
    cfg->m_clrComment = m_ui.pushButton_clr_comment->getColor();
    cfg->m_clrString = m_ui.pushButton_clr_string->getColor();
    cfg->m_clrIncString = m_ui.pushButton_clr_incString->getColor();
    cfg->m_clrKeyword = m_ui.pushButton_clr_keyword->getColor();
    cfg->m_clrCppKeyword = m_ui.pushButton_clr_cppKeyword->getColor();
    cfg->m_clrCurrentLine = m_ui.pushButton_clr_curLine->getColor();
    cfg->m_clrNumber = m_ui.pushButton_clr_number->getColor();
    cfg->m_clrForeground = m_ui.pushButton_clr_foreground->getColor();


    int comboIdx = m_ui.comboBox_sortTags->currentIndex();
    if(comboIdx == 0)
        m_cfg->m_tagSortByName = false;
    if(comboIdx == 1)
        m_cfg->m_tagSortByName = true;
    

    m_cfg->m_progConScrollback = m_ui.spinBox_progConScrollback->value();

    m_cfg->m_progConColorBg = m_ui.pushButton_progConBgClr->getColor(); 
    m_cfg->m_progConColorFg = m_ui.pushButton_progConFgClr->getColor(); 
    m_cfg->m_progConColorCursor = m_ui.pushButton_progConCursorClr->getColor(); 

    m_cfg->m_progConColorNorm[0] = m_ui.pushButton_progConClrNormal1->getColor();
    m_cfg->m_progConColorNorm[1] = m_ui.pushButton_progConClrNormal2->getColor();
    m_cfg->m_progConColorNorm[2] = m_ui.pushButton_progConClrNormal3->getColor();
    m_cfg->m_progConColorNorm[3] = m_ui.pushButton_progConClrNormal4->getColor();
    m_cfg->m_progConColorNorm[4] = m_ui.pushButton_progConClrNormal5->getColor();
    m_cfg->m_progConColorNorm[5] = m_ui.pushButton_progConClrNormal6->getColor();
    m_cfg->m_progConColorNorm[6] = m_ui.pushButton_progConClrNormal7->getColor();
    m_cfg->m_progConColorNorm[7] = m_ui.pushButton_progConClrNormal8->getColor();

    m_cfg->m_progConColorBright[0] = m_ui.pushButton_progConClrBright1->getColor();
    m_cfg->m_progConColorBright[1] = m_ui.pushButton_progConClrBright2->getColor();
    m_cfg->m_progConColorBright[2] = m_ui.pushButton_progConClrBright3->getColor();
    m_cfg->m_progConColorBright[3] = m_ui.pushButton_progConClrBright4->getColor();
    m_cfg->m_progConColorBright[4] = m_ui.pushButton_progConClrBright5->getColor();
    m_cfg->m_progConColorBright[5] = m_ui.pushButton_progConClrBright6->getColor();
    m_cfg->m_progConColorBright[6] = m_ui.pushButton_progConClrBright7->getColor();
    m_cfg->m_progConColorBright[7] = m_ui.pushButton_progConClrBright8->getColor();

    m_cfg->m_progConBackspaceKey = m_ui.comboBox_backspaceKey->currentIndex();
    m_cfg->m_progConDelKey = m_ui.comboBox_deleteKey->currentIndex();

    m_cfg->m_globalProjConfig = (m_ui.checkBox_globalProjConfig->checkState() == Qt::Unchecked) ? false : true;
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

void SettingsDialog::onSelectGdbOutputFont()
{
    showFontSelection(&m_settingsGdbOutputFontFamily, &m_settingsGdbOutputFontSize);
}

