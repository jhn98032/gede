#include "codeviewtab.h"

#include "log.h"



CodeViewTab::CodeViewTab(QWidget *parent)
  : QWidget(parent)
{
    m_ui.setupUi(this);

    
}

CodeViewTab::~CodeViewTab()
{
}


/**
 * @brief Ensures that a specific line is visible.
 */
void CodeViewTab::ensureLineIsVisible(int lineIdx)
{
    
        
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
            m_ui.comboBox_funcList->hide();
        }
        else
        {
            m_ui.comboBox_funcList->show();
            m_ui.comboBox_funcList->setCurrentIndex(bestFitIdx);
        }

    }
}

