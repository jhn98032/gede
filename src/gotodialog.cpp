//#define ENABLE_DEBUGMSG

/*
 * Copyright (C) 2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "gotodialog.h"

#include <QProcess>
#include <QLineEdit>
#include <QComboBox>
#include <QKeyEvent>

#include "qtutil.h"
#include "util.h"
#include "log.h"
#include "config.h"


#define MAX_TAGS   2000


GoToDialog::GoToDialog(QWidget *parent, Locator *locator, Settings *cfg, QString currentFilename)
    : QDialog(parent)
    ,m_currentFilename(currentFilename)
    ,m_locator(locator)
{
    Q_UNUSED(cfg);
    
    m_ui.setupUi(this);

    connect(m_ui.pushButton, SIGNAL(clicked()), SLOT(onGo()));

    connect(m_ui.comboBox, SIGNAL(editTextChanged( const QString &  )), SLOT(onSearchTextEdited(const QString &)));
 

    connect(m_ui.listWidget, SIGNAL(itemClicked(QListWidgetItem *)), SLOT(onItemClicked(QListWidgetItem *)));


    // Insert the list of old search terms in the combobox
    QStringList list = cfg->getGoToList();
    for(int i = 0;i < list.size();i++)
    {
        m_ui.comboBox->addItem(" " + list[i]);
    }
    m_ui.comboBox->clearEditText();

    onSearchTextEdited("");
   
    setFocusPolicy(Qt::StrongFocus);


    m_ui.comboBox->installEventFilter(this);

}


GoToDialog::~GoToDialog()
{

}


/**
 * @brief Called to save settings used by the GoTo dialog.
 */
void GoToDialog::saveSettings(Settings *cfg)
{
    // Get the list of items in the combobox
    QComboBox *comboBox = m_ui.comboBox;
    QStringList list;
    QString curText = comboBox->currentText();
    curText = curText.trimmed();
    if(!curText.isEmpty())
        list.append(curText);
    for(int i = 0; i < comboBox->count();i++)
    {
        list.append(comboBox->itemText(i).trimmed());
    }


    cfg->setGoToList(list);
}


/**
 * @brief Get the text left to the cursor in the search QLineEdit.
 */
QString getTextLeftToCursor(QComboBox *comboBox)
{
    // 
    int cursorPos = comboBox->lineEdit()->cursorPosition();
    QString oldExpr = comboBox->currentText();
    if(cursorPos < oldExpr.size())
    {
        oldExpr = oldExpr.left(cursorPos);
    }
    return oldExpr;
}



/**
 * @brief The user has pressed the tab key while the search combobox is selected.
 */
void GoToDialog::onComboBoxTabKey()
{
    QLineEdit *lineEdit = m_ui.comboBox->lineEdit();

    // Move cursor to end
    if(lineEdit->text().size() > 0)
        lineEdit->setCursorPosition(lineEdit->text().size()-1);


    // Set selection to before any selected text
    if(lineEdit->hasSelectedText())
    {
        int pos = lineEdit->text().size()-lineEdit->selectedText().size();
        lineEdit->setCursorPosition(pos);
    }

    

    if(m_ui.listWidget->count() == 1)
    {
        // Simulate a click on the item
        QListWidgetItem *item =  m_ui.listWidget->item(0);
        if(item)
            onItemClicked(item);
    }
    // No items in the list?
    else if(m_ui.listWidget->count() == 0)
    {
        // Add a space seperator
        QString editText = lineEdit->text();
        if(!editText.endsWith(" "))
            lineEdit->setText(editText + " ");
    }
    else
    {
        // Loop through all items in the list
        QString commonText;
        QListWidgetItem *item = m_ui.listWidget->item(0);
        commonText = item->text();
        for(int i = 1;i < m_ui.listWidget->count() && !(commonText.isEmpty());i++)
        {
            // Get the text of the item in the list
            item =  m_ui.listWidget->item(i);
            QString compText = item->text();

            // How many characters do they have in common?
            if(compText.size() < commonText.size())
                commonText = commonText.left(compText.size());
            
            for(int i = 0;i < qMin(commonText.size(), compText.size());i++)
            {
                if(compText[i] != commonText[i])
                    commonText = commonText.left(i);
            }
        }

        // User pressed 'tab' when there entries with a common beginning
        if(!commonText.isEmpty())
        {

            QString curText = getTextLeftToCursor(m_ui.comboBox);

            // Split the entered text into fields
            QStringList fields = curText.split(" ");
            
            // Replace the last part with the common beginning from the listwidget
            if(fields.size() == 0)
                fields.append(commonText);
            else
                fields[fields.size()-1] = commonText;

            // Update the combobox
            lineEdit->setText(fields.join(" "));
            
        }
            
    }
    
}


/**
 * @brief Called when the user has clicked on one of the QListWidget suggestions.
 */
void GoToDialog::onItemClicked ( QListWidgetItem * item )
{
    QString itemText = item->text().trimmed();


    //
    QString newText;
    QString oldExpr = getTextLeftToCursor(m_ui.comboBox);
    int lastSep = oldExpr.lastIndexOf(' ');
    if(lastSep == -1)
    {
        newText = itemText;
    }
    else
    {
        newText = oldExpr.left(lastSep+1) + itemText;
    }

/*
    if(newText.contains(' '))
        showListWidget(false);
*/
    m_ui.comboBox->lineEdit()->setText(newText + " ");
    m_ui.comboBox->setFocus();
    m_ui.comboBox->lineEdit()->deselect();
    
}


/**
 * @brief Shows or hides the suggestion listwidget
 */
void GoToDialog::showListWidget (bool show )
{
    if(show)
        m_ui.listWidget->show();
    else
        m_ui.listWidget->hide();

    int oldWidth = size().width();
    int oldHeight = size().height();
    int newHeight = show ? 250 : 10;

    if(oldHeight != newHeight)
    {
        adjustSize();
        //
        resize(oldWidth, newHeight);
    }
}

/**
 * @brief Cleans up the text entered by the user in the line edit box.
 */ 
QString GoToDialog::cleanupLinEditText(QString rawText)
{
    QString str = rawText.trimmed();

    // Replace double spaces and strange spaces
    str.replace("\t", " ");
    while(str.contains("  "))
        str.replace("  ", " ");

    // Add trailing space
    if(rawText.endsWith(' '))
        str += ' ';
    return str;
}


/**
 * @brief Returns the (cleaned up current entered text).
 */ 
QString GoToDialog::getCurrentLineEditText()
{
    QLineEdit *lineEdit = m_ui.comboBox->lineEdit();
    return cleanupLinEditText(lineEdit->text());
}


/**
 * @brief Called when the user changed the search text in the QLineEdit.
 */
void GoToDialog::onSearchTextEdited ( const QString & text2 )
{
    QString text = cleanupLinEditText(text2);

    debugMsg("%s('%s')", __func__ ,qPrintable(text));


    m_ui.listWidget->clear();


    // Get the last expression
    QStringList expList = text.split(' ');
    QString expr;
    enum { SHOW_NONE, SHOW_FUNC, SHOW_FUNC_AND_FILE} showSuggestion = SHOW_FUNC_AND_FILE;
    if(expList.size() == 0)
        expList.append("");
    QString lastExpr = expList.last();

    // No suggestion if the last field is a integer
    if(isInteger(lastExpr))
    {
        showListWidget(false);
        return;
    }

    //
    if(expList.size() <= 1)
    {
        if(isInteger(lastExpr) && !lastExpr.isEmpty())
        {
            m_ui.labelHelp->setText("Enter linenumber");
            showSuggestion = SHOW_NONE;
        }
        else
        {
            m_ui.labelHelp->setText("Syntax: [file.c] [func()] [lineno]");
            showSuggestion = SHOW_FUNC_AND_FILE;
        }
    }
    else if(expList.size() == 2)
    {
        if(isInteger(lastExpr) && !lastExpr.isEmpty())
        {
            m_ui.labelHelp->setText("Enter linenumber");
            showSuggestion = SHOW_NONE;
        }
        else if(expList[0].contains("("))
        {
            m_ui.labelHelp->setText("Enter linenumber");
            showSuggestion = SHOW_NONE;
        }
        else
        {
            m_ui.labelHelp->setText("Enter function or linenumber");
            showSuggestion = SHOW_FUNC;
        }
    }
    else if(expList.size() == 3)
    {
        m_ui.labelHelp->setText("Enter function linenumber offset");
        showSuggestion = SHOW_NONE;
    }
    else
    {
        m_ui.labelHelp->setText("To many arguments!");
        showSuggestion = SHOW_NONE;
    }
    expr = expList.last();
        
    // Ask the locator for files and tags that match
    QStringList exprList;
    if(showSuggestion == SHOW_FUNC_AND_FILE)
        exprList = m_locator->searchExpression(expr);
    else if(showSuggestion == SHOW_FUNC)
        exprList = m_locator->searchExpression(expList[0], expr);
    
    // Add the found ones to to the list
    for(int i = 0;i < qMin(exprList.size(), MAX_TAGS);i++)
    {
        QString fieldText = exprList[i];
        QListWidgetItem *item = new QListWidgetItem(fieldText);
        item->setSizeHint(QSize(GOTO_LISTWIDGET_ITEM_WIDTH,20));
        m_ui.listWidget->addItem(item);
    }
    m_ui.listWidget->sortItems(Qt::AscendingOrder);

    if(showSuggestion == SHOW_NONE)
        showListWidget(false);
    else
        showListWidget(true);
    
}


/**
 * @brief Returns the file and linenumber the user wants to go to.
 */
void GoToDialog::getSelection(QString *filename, int *lineno)
{
    QString expr = getCurrentLineEditText();
    QVector<Location> locList = m_locator->locate(expr);
    if(locList.size() >= 1)
    {
        Location loc = locList[0];
        *filename = loc.m_filename;
        *lineno = loc.m_lineNo;
    }
    else
        debugMsg("No location found!");
}

/**
 * @brief Called when the user pressed the go button.
 */
void GoToDialog::onGo()
{

    accept();
}


/**
 * @brief Event filter.
 */
bool GoToDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==m_ui.comboBox)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

            if(keyEvent->key() == Qt::Key_Tab)
            {

                onComboBoxTabKey();
                
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        // pass the event on to the parent class
        return QWidget::eventFilter(obj, event);
    }
}


