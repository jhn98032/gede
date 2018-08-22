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



GoToDialog::GoToDialog(QWidget *parent, Locator *locator, Settings *cfg, QString currentFilename)
    : QDialog(parent)
    ,m_currentFilename(currentFilename)
    ,m_locator(locator)
{
    Q_UNUSED(cfg);
    
    m_ui.setupUi(this);

    connect(m_ui.pushButton, SIGNAL(clicked()), SLOT(onGo()));

}


GoToDialog::~GoToDialog()
{

}


void GoToDialog::getSelection(QString *filename, int *lineno)
{
    QString expr = m_ui.lineEdit->text();
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

