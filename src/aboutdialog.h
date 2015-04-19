/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__ABOUTDIALOG_H
#define FILE__ABOUTDIALOG_H

#include <QDialog>

#include "settings.h"
#include "ui_aboutdialog.h"


class AboutDialog : public QDialog
{
    Q_OBJECT

public:

    AboutDialog(QWidget *parent, Settings *cfg);


private:
    QString getGdbVersion(QString gdbPath);


    Ui_AboutDialog m_ui;
    
};

#endif // FILE__ABOUTDIALOG_H

