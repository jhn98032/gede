/*
 * Copyright (C) 2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__CONSOLEWIDGET_H
#define FILE__CONSOLEWIDGET_H

#include <QPlainTextEdit>


class ConsoleWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    ConsoleWidget(QWidget *parent);
    virtual ~ConsoleWidget();

    void append(QString text);
    
protected:

    void keyPressEvent ( QKeyEvent * event );


};

#endif // FILE__CONSOLEWIDGET_H

