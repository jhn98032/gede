/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__MAINWINDOW_H
#define FILE__MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QMap>
#include <QLabel>

#include "ui_mainwindow.h"
#include "core.h"
#include "codeview.h"
#include "settings.h"
#include "tagscanner.h"
#include "autovarctl.h"
#include "watchvarctl.h"
#include "codeviewtab.h"
#include "tagmanager.h"


class FileInfo
{
public:
    QString name;
    QString fullName;
};


class MainWindow : public QMainWindow, public ICore, public ICodeView
{
  Q_OBJECT
public:
    MainWindow(QWidget *parent);
    virtual ~MainWindow();

    CodeViewTab* open(QString filename);
    

public:
    void insertSourceFiles();
    void setStatusLine(Settings &cfg);
    
public:
    void ICore_onStopped(ICore::StopReason reason, QString path, int lineNo);
    void ICore_onLocalVarReset();
    void ICore_onLocalVarChanged(CoreVar *varValue);
    void ICore_onWatchVarChanged(VarWatch &watch);
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
    void ICore_onSourceFileListChanged();
    
    void ICodeView_onRowDoubleClick(int lineNo);
    void ICodeView_onContextMenu(QPoint pos, int lineNo, QStringList text);
    void ICodeView_onContextMenuIncFile(QPoint pos, int lineNo, QString incFile);
    
    void ICore_onWatchVarChildAdded(VarWatch &watch);
    
private:
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *e);

    void showWidgets();

public:
        
private:
    void setConfig();
    
    void wrapSourceTree(QTreeWidget *treeWidget);

    QTreeWidgetItem *addTreeWidgetPath(QTreeWidget *treeWidget, QTreeWidgetItem *parent, QString path);
    void fillInStack();

    bool eventFilter(QObject *obj, QEvent *event);
    void loadConfig();
    QTreeWidgetItem *insertTreeWidgetItem(
                    VarCtl::DispInfoMap *map,
                    QString fullPath,
                    QString name,
                    QString value);
    void addVariableDataTree(
                QTreeWidget *treeWidget,
                VarCtl::DispInfoMap *map,
                QTreeWidgetItem *item, TreeNode *rootNode);

    CodeViewTab* createTab(QString filename);
    CodeViewTab* currentTab();
    void updateCurrentLine(QString filename, int lineno);
    void onCurrentLineChanged(int lineno);
    void onCurrentLineDisabled();


public slots:
    void onFolderViewItemActivated ( QTreeWidgetItem * item, int column );
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
    void onCodeViewContextMenuOpenFile();
    void onCodeViewContextMenuShowDefinition();
    void onCodeViewContextMenuShowCurrentLocation();
    void onSettings();
    void onCodeViewContextMenuToggleBreakpoint();
    void onCodeViewTab_tabCloseRequested ( int index );
    void onCodeViewTab_currentChanged( int tabIdx);
    
    void onViewStack();
    void onViewBreakpoints();
    void onViewThreads();
    void onViewWatch();
    void onViewAutoVariables();
    void onViewTargetOutput();
    void onViewGdbOutput();
    void onViewFileBrowser();
    void onDefaultViewSetup();

private:
    QByteArray m_gui_default_mainwindowState;
    QByteArray m_gui_default_mainwindowGeometry;
    QByteArray m_gui_default_splitter1State;
    QByteArray m_gui_default_splitter2State;
    QByteArray m_gui_default_splitter3State;
    QByteArray m_gui_default_splitter4State;

private:
    Ui_MainWindow m_ui;
    QIcon m_fileIcon;
    QIcon m_folderIcon;
    QString m_currentFile; //!< The file which the program counter points to.
    int m_currentLine; //!< The linenumber (first=1) which the program counter points to.
    QList<StackFrameEntry> m_stackFrameList;
    QMenu m_popupMenu;
    
    Settings m_cfg;
    TagManager m_tagManager;
    QList<FileInfo> m_sourceFiles;

    AutoVarCtl m_autoVarCtl;
    WatchVarCtl m_watchVarCtl;
    QFont m_outputFont;
    QFont m_gdbOutputFont;
    QLabel m_statusLineWidget;
};


#endif


