/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "aboutdialog.h"
#include "version.h"


AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    
    m_ui.setupUi(this);

    //
    QString verStr;
    verStr.sprintf("Version: v%d.%d.%d", GD_MAJOR, GD_MINOR, GD_PATCH);
    m_ui.label_version->setText(verStr);

    //
    QString buildStr;
    buildStr = __DATE__;
    buildStr += " ";
    buildStr += __TIME__;
    m_ui.label_buildDate->setText("Built: " + buildStr);

}


    
