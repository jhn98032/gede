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
    
public slots:
    void onFuncListItemActivated(int index);

private:
    Ui_CodeViewTab m_ui;
    QString m_filepath;
   
};

#endif


