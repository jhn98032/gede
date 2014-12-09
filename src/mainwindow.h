#ifndef FILE__MAINWINDOW_H
#define FILE__MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>

#include "ui_mainwindow.h"
#include "core.h"
#include "codeview.h"

 

class MainWindow : public QMainWindow, public ICore, public ICodeView
{
  Q_OBJECT
public:
      MainWindow(QWidget *parent);

void open(QString filename);
void fillInVars();
void ensureLineIsVisible(int lineIdx);


public:
    void insertSourceFiles();
    
public:
    void ICore_onStopped(QString path, int lineno);
    void ICore_onLocalVarReset();
    void ICore_onLocalVarChanged(QString name, QString data);
    void ICore_onWatchVarChanged(int watchId, QString name, QString value);
    void ICore_onConsoleStream(QString text);
    void ICore_onBreakpointsChanged();
    void ICore_onThreadListChanged();
    void ICore_onCurrentThreadChanged(int threadId);
    void ICore_onStackFrameChange(QList<StackFrameEntry> stackFrameList);
    void ICore_onFrameVarReset();
    void ICore_onFrameVarChanged(QString name, QString value);
    void ICore_onMessage(QString message);
    void ICore_onCurrentFrameChanged(int frameIdx);
    
    void ICodeView_onRowDoubleClick(int rowIdx);
    void ICodeView_onContextMenu(QPoint pos, QStringList text);
private:

    QTreeWidgetItem *addTreeWidgetPath(QTreeWidget *treeWidget, QTreeWidgetItem *parent, QString path);
    void fillInStack();

    bool eventFilter(QObject *obj, QEvent *event);
    void loadConfig();
    
public slots:
    void onFolderViewItemActivated ( QTreeWidgetItem * item, int column );
    void onVarWidgetCurrentItemChanged ( QTreeWidgetItem * current, int column );
    void onThreadWidgetSelectionChanged( );
    void onStackWidgetSelectionChanged();
    void onQuit();
    void onNext();
    void onStepIn();
    void onAbout();
    void onStop();
    void onBreakpointsWidgetItemDoubleClicked(QTreeWidgetItem * item,int column);
    void onRun();
    void onContinue();
    void onCodeViewContextMenuItemPressed();
    void onSettings();

    
private:
    Ui_MainWindow m_ui;
    QString m_filename; // Currently displayed file
    QIcon m_fileIcon;
    QIcon m_folderIcon;
    QString m_currentFile; //!< The file which the program counter points to.
    int m_currentLine; //!< The linenumber (first=1) which the program counter points to.
    QList<StackFrameEntry> m_stackFrameList;
    QMenu m_popupMenu;

    Ini m_ini;

};


#endif


