/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "codeviewtab.h"

#include <assert.h>
#include <QScrollBar>
#include <QFile>

#include "config.h"
#include "util.h"
#include "log.h"


CodeViewTab::CodeViewTab(QWidget *parent)
  : QWidget(parent)
{
    m_ui.setupUi(this);

    connect(m_ui.comboBox_funcList, SIGNAL(activated(int)), SLOT(onFuncListItemActivated(int)));

    
}

CodeViewTab::~CodeViewTab()
{
}

static bool compareTagsByLineNo(const Tag &t1, const Tag &t2)
{
        return t1.getLineNo() < t2.getLineNo();
}

static bool compareTagsByName(const Tag &t1, const Tag &t2)
{
        return t1.getLongName() < t2.getLongName();
}

void CodeViewTab::fillInFunctions(QList<Tag> tagList)
{
    m_ui.comboBox_funcList->clear();
    if(m_cfg->m_tagSortByName)
        qSort(tagList.begin(),tagList.end(), compareTagsByName);
    else
        qSort(tagList.begin(),tagList.end(), compareTagsByLineNo);
    for(int tagIdx = 0;tagIdx < tagList.size();tagIdx++)
    {
        Tag &tag = tagList[tagIdx];
        if(tag.m_type == Tag::TAG_FUNC)
        {
            QString text;
            if(m_cfg->m_tagShowLineNumbers)
            {
                text.sprintf("L%d: ", tag.getLineNo());
                text = text.leftJustified(6, ' ');
            }
            text += tag.getLongName();
            m_ui.comboBox_funcList->addItem(text, QVariant(tag.getLineNo()));
        }
        
    }

}

int CodeViewTab::open(QString filename, QList<Tag> tagList)
{
    m_filepath = filename;
    QString extension = getExtensionPart(filename);
    QString text;
// Read file content
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errorMsg("Failed to open '%s'", stringToCStr(filename));
        return -1;
    }
    while (!file.atEnd())
    {
         QByteArray line = file.readLine();
         text += line;
    }

    if(extension.toLower() == ".bas")
        m_ui.codeView->setPlainText(text, CodeView::CODE_BASIC);
    else if(extension.toLower() == ".f" || extension.toLower() == ".f95"
             || extension.toLower() == ".for")
        m_ui.codeView->setPlainText(text, CodeView::CODE_FORTRAN);
    else if(extension.toLower() == RUST_FILE_EXTENSION)
        m_ui.codeView->setPlainText(text, CodeView::CODE_RUST);
    else if(extension.toLower() == GOLANG_FILE_EXTENSION)
        m_ui.codeView->setPlainText(text, CodeView::CODE_GOLANG);
    else
        m_ui.codeView->setPlainText(text, CodeView::CODE_CXX);

    m_ui.scrollArea_codeView->setWidgetResizable(true);

    // Fill in the functions
    fillInFunctions(tagList);
    m_tagList = tagList;


    return 0;
}


/**
 * @brief Ensures that a specific line is visible.
 */
void CodeViewTab::ensureLineIsVisible(int lineIdx)
{
    
    if(lineIdx < 0)
        lineIdx = 0;


    // Find the function in the function combobox that matches the line
    int bestFitIdx = -1;
    int bestFitDist = -1;
    for(int u = 0;u < m_ui.comboBox_funcList->count();u++)
    {
        int funcLineNo = m_ui.comboBox_funcList->itemData(u).toInt();
        int dist = lineIdx-funcLineNo;
        if((bestFitDist > dist || bestFitIdx == -1) && dist >= 0)
        {
            bestFitDist = dist;
            bestFitIdx = u;
        }
    }

    if(m_ui.comboBox_funcList->count() > 0)
    {

        if(bestFitIdx == -1)
        {
            //m_ui.comboBox_funcList->hide();
        }
        else
        {
            m_ui.comboBox_funcList->show();
            m_ui.comboBox_funcList->setCurrentIndex(bestFitIdx);
        }

    }

    // Select the function in the function combobox
    int comboBoxIdx = m_ui.comboBox_funcList->findData(QVariant(lineIdx));
    if(comboBoxIdx >= 0)
    {
        if(m_ui.comboBox_funcList->currentIndex() != comboBoxIdx)
        {
            m_ui.comboBox_funcList->setCurrentIndex(comboBoxIdx);
        }
    }

    m_ui.scrollArea_codeView->ensureVisible(0, m_ui.codeView->getRowHeight()*lineIdx-1);
    m_ui.scrollArea_codeView->ensureVisible(0, m_ui.codeView->getRowHeight()*lineIdx-1);
}

void CodeViewTab::onFuncListItemActivated(int index)
{
        
    int funcLineNo = m_ui.comboBox_funcList->itemData(index).toInt();
    int lineIdx = funcLineNo-2;
    if(lineIdx < 0)
        lineIdx = 0;
    m_ui.scrollArea_codeView->verticalScrollBar()->setValue(m_ui.codeView->getRowHeight()*lineIdx);
}

void CodeViewTab::setBreakpoints(const QVector<int> &numList)
{
    m_ui.codeView->setBreakpoints(numList);
    m_ui.codeView->update();
}


void CodeViewTab::setConfig(Settings *cfg)
{
    m_cfg = cfg;
    m_ui.codeView->setConfig(cfg);

    fillInFunctions(m_tagList);
    
}

void CodeViewTab::disableCurrentLine()
{
    m_ui.codeView->disableCurrentLine();
}

void CodeViewTab::setCurrentLine(int currentLine)
{
    m_ui.codeView->setCurrentLine(currentLine);
}


void CodeViewTab::setInterface(ICodeView *inf)
{
    m_ui.codeView->setInterface(inf);
}


