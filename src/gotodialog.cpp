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

    onSearchTextEdited("");
   
    setFocusPolicy(Qt::StrongFocus);


    m_ui.comboBox->installEventFilter(this);

}


GoToDialog::~GoToDialog()
{

}


/**
 * @brief The user has pressed the tab key while the search combobox is selected.
 */
void GoToDialog::onComboBoxTabKey()
{
    QLineEdit *lineEdit = m_ui.comboBox->lineEdit();

    
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
            debugMsg("common '%s'", qPrintable(commonText));
            // Split the entered text into fields
            QString editText = lineEdit->text();
            QStringList fields = editText.split(' ');

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


void GoToDialog::onItemClicked ( QListWidgetItem * item )
{
    QString itemText = item->text();

    //
    QString newText;
    QString oldExpr = m_ui.comboBox->currentText();
    int lastSep = oldExpr.lastIndexOf(' ');
    if(lastSep == -1)
    {
        newText = itemText;
    }
    else
    {
        newText = oldExpr.left(lastSep+1) + itemText;
    }

    if(newText.contains(' '))
        showListWidget(false);
    m_ui.comboBox->lineEdit()->setText(newText + " ");
    m_ui.comboBox->setFocus();
    m_ui.comboBox->lineEdit()->deselect();
    
}

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

void GoToDialog::onSearchTextEdited ( const QString & text )
{
    debugMsg("%s('%s')", __func__ ,qPrintable(text));
    
    m_ui.listWidget->clear();

/*    
    if(text.endsWith(' '))
    {
        showListWidget(false);
        return;
    }
*/

    // Get the last expression
    QStringList expList = text.split(' ');
    QString expr;
    enum { SHOW_NONE, SHOW_FUNC, SHOW_FUNC_AND_FILE} showSuggestion = SHOW_FUNC_AND_FILE;
    if(expList.size() == 0)
        expList.append("");
    QString lastExpr = expList.last();
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
        item->setSizeHint(QSize(160,20));
        m_ui.listWidget->addItem(item);
    }

    if(showSuggestion == SHOW_NONE)
        showListWidget(false);
    else
        showListWidget(true);
    
}

void GoToDialog::getSelection(QString *filename, int *lineno)
{
    QString expr = m_ui.comboBox->currentText();
    QVector<Location> locList = m_locator->locate(expr);
    if(locList.size() >= 1)
    {
        Location loc = locList[0];
        *filename = loc.filename;
        *lineno = loc.lineNo;
    }
}

    
void GoToDialog::onGo()
{

    accept();
}


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


