/*
 * Copyright (C) 2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__GOTODIALOG_H
#define FILE__GOTODIALOG_H

#include <QDialog>
#include <QVector>

#include "ui_gotodialog.h"

#include "settings.h"

class Location
{
    public:
        QString m_filename;
        QString m_lineno;
};

class GoToDialog : public QDialog
{
    Q_OBJECT

public:

    GoToDialog(QWidget *parent, Settings *cfg, QString currentFilename);
    virtual ~GoToDialog();

    void getSelection(QString *filename, int *lineno);

public slots:
    void onGo();

private:


    Ui_GoToDialog m_ui;
    QString m_currentFilename;
    
};

#endif // FILE__ABOUTDIALOG_H

