/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__CODEVIEWTAB_H
#define FILE__CODEVIEWTAB_H

#include "ui_codeviewtab.h"

#include "tagscanner.h"
#include <QWidget>

class CodeViewTab : public QWidget
{
Q_OBJECT

public:
    CodeViewTab(QWidget *parent);
    virtual ~CodeViewTab();

    void ensureLineIsVisible(int lineIdx);
    
    void setConfig(Settings *cfg);
    void disableCurrentLine();
    
    void setCurrentLine(int currentLine);
                    

    int open(QString filename, QList<Tag> tagList);

    void setInterface(ICodeView *inf);
    
    void setBreakpoints(const QVector<int> &numList);

    QString getFilePath() { return m_filepath; };

private:
    
    void fillInFunctions(QList<Tag> tagList);

public slots:
    void onFuncListItemActivated(int index);

private:
    Ui_CodeViewTab m_ui;
    QString m_filepath;
    Settings *m_cfg;
    QList<Tag> m_tagList;
};

#endif


