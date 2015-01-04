#ifndef FILE__MAINWINDOW_H
#define FILE__MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QMap>

#include "ui_mainwindow.h"
#include "core.h"
#include "codeview.h"
#include "settings.h"
#include "tagscanner.h"

class FileInfo
{
public:
    QString name;
    QString fullName;
    QList<Tag> m_tagList;
};


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
    void ICore_onStopped(ICore::StopReason reason, QString path, int lineNo);
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
    void ICore_onSignalReceived(QString sigtype);
    void ICore_onTargetOutput(QString msg);
    void ICore_onStateChanged(TargetState state);
    
    void ICodeView_onRowDoubleClick(int lineNo);
    void ICodeView_onContextMenu(QPoint pos, int lineNo, QStringList text);
private:
    enum DispFormat
    {
        DISP_NATIVE,
        DISP_DEC,
        DISP_BIN,
        DISP_HEX,
        DISP_CHAR,
    };
    typedef struct
    {
        QString orgValue;
        DispFormat orgFormat;
        DispFormat dispFormat;
    }DispInfo;


    QTreeWidgetItem *addTreeWidgetPath(QTreeWidget *treeWidget, QTreeWidgetItem *parent, QString path);
    void fillInStack();

    bool eventFilter(QObject *obj, QEvent *event);
    void loadConfig();
    DispFormat findVarType(QString dataString);
    QString valueDisplay(long long value, DispFormat format);
    
public slots:
    void onFolderViewItemActivated ( QTreeWidgetItem * item, int column );
    void onWatchWidgetCurrentItemChanged ( QTreeWidgetItem * current, int column );
    void onThreadWidgetSelectionChanged( );
    void onStackWidgetSelectionChanged();
    void onQuit();
    void onNext();
    void onStepIn();
    void onStepOut();
    void onAbout();
    void onStop();
    void onBreakpointsWidgetItemDoubleClicked(QTreeWidgetItem * item,int column);
    void onRun();
    void onContinue();
    void onCodeViewContextMenuAddWatch();
    void onCodeViewContextMenuShowDefinition();
    void onCodeViewContextMenuShowCurrentLocation();
    void onSettings();
    void onWatchWidgetItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onAutoWidgetItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onFuncListItemActivated(int index);
    void onCodeViewContextMenuToggleBreakpoint();
    
    
private:
    Ui_MainWindow m_ui;
    QString m_filename; // Currently displayed file
    QIcon m_fileIcon;
    QIcon m_folderIcon;
    QString m_currentFile; //!< The file which the program counter points to.
    int m_currentLine; //!< The linenumber (first=1) which the program counter points to.
    QList<StackFrameEntry> m_stackFrameList;
    QMenu m_popupMenu;
    QMap<QString, DispInfo> m_watchVarDispInfo;
    QMap<QString, DispInfo> m_autoVarDispInfo;

    Settings m_cfg;
    TagScanner m_tagScanner;
    QList<FileInfo> m_sourceFiles;
};


#endif


