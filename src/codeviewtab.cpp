#include "codeviewtab.h"

#include <assert.h>
#include <QScrollBar>
#include <QFile>

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


int CodeViewTab::open(QString filename, QList<Tag> tagList)
{
    m_filepath = filename;
    
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

    m_ui.codeView->setPlainText(text);

    m_ui.scrollArea_codeView->setWidgetResizable(true);


    // Fill in the functions
    m_ui.comboBox_funcList->clear();
    for(int tagIdx = 0;tagIdx < tagList.size();tagIdx++)
    {
        Tag &tag = tagList[tagIdx];
        if(tag.type == Tag::TAG_FUNC)
        {
            m_ui.comboBox_funcList->addItem(tag.getLongName(), QVariant(tag.getLineNo()));
        }
        
    }


    return 0;
}


/**
 * @brief Ensures that a specific line is visible.
 */
void CodeViewTab::ensureLineIsVisible(int lineIdx)
{
    
    if(lineIdx < 0)
        lineIdx = 0;

    m_ui.scrollArea_codeView->ensureVisible(0, m_ui.codeView->getRowHeight()*lineIdx-1);

    // Select the function in the function combobox
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
    m_ui.codeView->setConfig(cfg);
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


