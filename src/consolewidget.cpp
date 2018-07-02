/*
 * Copyright (C) 2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "consolewidget.h"

//#define ENABLE_DEBUGMSG

#include "log.h"
#include "core.h"


ConsoleWidget::ConsoleWidget(QWidget *parent)
    : QPlainTextEdit(parent)
{

}

ConsoleWidget::~ConsoleWidget()
{

}

void ConsoleWidget::append ( QString text )
{
    moveCursor(QTextCursor::End);
    insertPlainText(text);
}

void ConsoleWidget::keyPressEvent ( QKeyEvent * event )
{
    Core &core = Core::getInstance();

    debugMsg("%s()", __func__);

    QString text = event->text();
    core.writeTargetStdin(text); 
}
    

void ConsoleWidget::clearAll()
{
    clear();
}


