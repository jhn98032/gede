/*
 * Copyright (C) 2014-2017 Johan Henriksson.
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
#include <QTime>

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
                    
    void incSearchStart(QString text) { return m_ui.codeView->incSearchStart(text); };
    void incSearchNext() { return m_ui.codeView->incSearchNext(); };
    void incSearchPrev() { return m_ui.codeView->incSearchNext(); };
    
    int open(QString filename, QList<Tag> tagList);

    void setInterface(ICodeView *inf);
    
    void setBreakpoints(const QVector<int> &numList);

    QString getFilePath() { return m_filepath; };

    void updateLastAccessStamp() { m_lastOpened = QTime::currentTime(); };
    QTime getLastAccessTime() { return m_lastOpened; };
private:
    
    void fillInFunctions(QList<Tag> tagList);

public slots:
    void onFuncListItemActivated(int index);

private:
    Ui_CodeViewTab m_ui;
    QString m_filepath;
    Settings *m_cfg;
    QList<Tag> m_tagList;
    QTime m_lastOpened; //!< When the tab was last accessed
};

#endif


