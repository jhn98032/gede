/*
 * Copyright (C) 2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "gotodialog.h"

#include <QProcess>

#include "util.h"
#include "log.h"



GoToDialog::GoToDialog(QWidget *parent, Settings *cfg, QString currentFilename)
    : QDialog(parent)
    ,m_currentFilename(currentFilename)
{
    Q_UNUSED(cfg);
    
    m_ui.setupUi(this);

    connect(m_ui.pushButton, SIGNAL(clicked()), SLOT(onGo()));

}


GoToDialog::~GoToDialog()
{

}


void stringToLocation(QString str, QString *filename, int *lineNo)
{
    Q_UNUSED(filename);
    *lineNo = str.toInt();
}

void GoToDialog::getSelection(QString *filename, int *lineno)
{
    stringToLocation(m_ui.lineEdit->text(), filename, lineno);
    if(filename->isEmpty())
        *filename = m_currentFilename;
}

    
void GoToDialog::onGo()
{

    accept();
}

