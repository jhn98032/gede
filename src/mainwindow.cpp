//#define ENABLE_DEBUGMSG

/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "mainwindow.h"

#include <QDirIterator>
#include <QMessageBox>
#include <QScrollBar>
#include <QFileInfo>

#include <assert.h>

#include "util.h"
#include "log.h"
#include "core.h"
#include "aboutdialog.h"
#include "settingsdialog.h"
#include "tagscanner.h"
#include "codeview.h"
#include "memorydialog.h"
#include "gotodialog.h"



MainWindow::MainWindow(QWidget *parent)
      : QMainWindow(parent)
      ,m_tagManager(m_cfg)
      ,m_locator(&m_tagManager, &m_sourceFiles)
{
    QStringList names;

    
    m_ui.setupUi(this);

    m_autoVarCtl.setWidget(m_ui.autoWidget);
    m_watchVarCtl.setWidget(m_ui.varWidget);


    m_fileIcon.addFile(QString::fromUtf8(":/images/res/file.png"), QSize(), QIcon::Normal, QIcon::Off);
    m_folderIcon.addFile(QString::fromUtf8(":/images/res/folder.png"), QSize(), QIcon::Normal, QIcon::Off);


    //
    m_ui.treeWidget_breakpoints->setColumnCount(4);
    m_ui.treeWidget_breakpoints->setColumnWidth(0, 120);
    m_ui.treeWidget_breakpoints->setColumnWidth(1, 40);
    m_ui.treeWidget_breakpoints->setColumnWidth(2, 200);
    m_ui.treeWidget_breakpoints->setColumnWidth(3, 140);
    names.clear();
    names += "Filename";
    names += "Line";
    names += "Func";
    names += "Addr";
    m_ui.treeWidget_breakpoints->setHeaderLabels(names);
    connect(m_ui.treeWidget_breakpoints, SIGNAL(itemDoubleClicked ( QTreeWidgetItem * , int  )), this, SLOT(onBreakpointsWidgetItemDoubleClicked(QTreeWidgetItem * ,int)));
    connect(m_ui.treeWidget_breakpoints, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onBreakpointsWidgetContextMenu(const QPoint&)));
    m_ui.treeWidget_breakpoints->setContextMenuPolicy(Qt::CustomContextMenu);






    //
    QTreeWidget *treeWidget = m_ui.treeWidget_file;
    treeWidget->setColumnWidth(0, 200);


    // Thread widget
    treeWidget = m_ui.treeWidget_threads;
    names.clear();
    names += "Name";
    names += "Details";
    treeWidget->setHeaderLabels(names);
    treeWidget->setColumnCount(2);
    treeWidget->setColumnWidth(0, 150);
    treeWidget->setColumnWidth(1, 100);

    connect(m_ui.treeWidget_threads, SIGNAL(itemSelectionChanged()), this,
                SLOT(onThreadWidgetSelectionChanged()));

    // Stack widget
    treeWidget = m_ui.treeWidget_stack;
    names.clear();
    names += "Name";
    treeWidget->setHeaderLabels(names);
    treeWidget->setColumnCount(1);
    treeWidget->setColumnWidth(0, 200);

    connect(m_ui.treeWidget_stack, SIGNAL(itemSelectionChanged()), this,
                SLOT(onStackWidgetSelectionChanged()));



    //
    QList<int> slist;
    slist.append(100);
    slist.append(300);
    m_ui.splitter_1->setSizes(slist);

     
    //
    QList<int> slist2;
    slist2.append(500);
    slist2.append(70);
    m_ui.splitter_2->setSizes(slist2);



    //
    QList<int> slist3;
    slist3.append(300);
    slist3.append(120);
    slist3.append(120);
    m_ui.splitter_3->setSizes(slist3);



    //
    QList<int> slist4;
    slist4.append(300);
    slist4.append(120);
    m_ui.splitter_4->setSizes(slist4);

     

    connect(m_ui.treeWidget_file, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(onFolderViewItemActivated(QTreeWidgetItem*,int)));

    connect(m_ui.actionQuit, SIGNAL(triggered()), SLOT(onQuit()));
    connect(m_ui.actionStop, SIGNAL(triggered()), SLOT(onStop()));
    connect(m_ui.actionNext, SIGNAL(triggered()), SLOT(onNext()));
    connect(m_ui.actionAbout, SIGNAL(triggered()), SLOT(onAbout()));
    connect(m_ui.actionGoToLine, SIGNAL(triggered()), SLOT(onGoToLine()));
    connect(m_ui.actionSearch, SIGNAL(triggered()), SLOT(onSearch()));
    connect(m_ui.actionStep_In, SIGNAL(triggered()), SLOT(onStepIn()));
    connect(m_ui.actionStep_Out, SIGNAL(triggered()), SLOT(onStepOut()));
    connect(m_ui.actionRestart, SIGNAL(triggered()), SLOT(onRestart()));
    connect(m_ui.actionContinue, SIGNAL(triggered()), SLOT(onContinue()));

    connect(m_ui.actionViewStack, SIGNAL(triggered()), SLOT(onViewStack()));
    connect(m_ui.actionViewBreakpoints, SIGNAL(triggered()), SLOT(onViewBreakpoints()));
    connect(m_ui.actionViewThreads, SIGNAL(triggered()), SLOT(onViewThreads()));
    connect(m_ui.actionViewWatch, SIGNAL(triggered()), SLOT(onViewWatch()));
    connect(m_ui.actionViewAutoVariables, SIGNAL(triggered()), SLOT(onViewAutoVariables()));
    connect(m_ui.actionViewTargetOutput, SIGNAL(triggered()), SLOT(onViewTargetOutput()));
    connect(m_ui.actionViewGedeOutput, SIGNAL(triggered()), SLOT(onViewGedeOutput()));
    connect(m_ui.actionViewGdbOutput, SIGNAL(triggered()), SLOT(onViewGdbOutput()));
    connect(m_ui.actionViewFileBrowser, SIGNAL(triggered()), SLOT(onViewFileBrowser()));
    connect(m_ui.actionViewClassFilter, SIGNAL(triggered()), SLOT(onViewClassFilter()));
    connect(m_ui.actionViewFunctionFilter, SIGNAL(triggered()), SLOT(onViewFuncFilter()));
    
    connect(m_ui.actionGoToMain, SIGNAL(triggered()), SLOT(onGoToMain()));

    connect(m_ui.actionDefaultViewSetup, SIGNAL(triggered()), SLOT(onDefaultViewSetup()));

    connect(m_ui.actionSettings, SIGNAL(triggered()), SLOT(onSettings()));

    

    Core &core = Core::getInstance();
    core.setListener(this);

    connect(&m_tagManager, SIGNAL(onAllScansDone()), SLOT(onAllTagScansDone()));

    //Setup the function treewidget
    treeWidget = m_ui.treeWidget_functions;
    treeWidget->setColumnWidth(0, 200);
    connect(m_ui.treeWidget_functions, SIGNAL(itemClicked(QTreeWidgetItem * , int )),
            SLOT(onFuncWidgetItemSelected(QTreeWidgetItem * , int )));

    m_ui.lineEdit_funcFilter->setPlaceholderText("Filter1;Filter2;...");
    connect(m_ui.lineEdit_funcFilter, SIGNAL(textChanged(const QString &)), SLOT(onFuncFilter_textChanged(const QString&)));
    connect(m_ui.pushButton_clearFuncFilter, SIGNAL(clicked()), SLOT(onFuncFilterClear()));
    connect(m_ui.checkBox_funcFilter, SIGNAL(stateChanged (int)), SLOT(onFuncFilterCheckBoxStateChanged(int)));
    
    m_ui.lineEdit_classFilter->setPlaceholderText("Filter1;Filter2;...");
    connect(m_ui.lineEdit_classFilter, SIGNAL(textChanged(const QString &)), SLOT(onClassFilter_textChanged(const QString&)));
    connect(m_ui.pushButton_clearClassFilter, SIGNAL(clicked()), SLOT(onClassFilterClear()));
    connect(m_ui.checkBox_classFilter, SIGNAL(stateChanged (int)), SLOT(onClassFilterCheckBoxStateChanged(int)));
    
    connect(m_ui.lineEdit_search, SIGNAL(textChanged(const QString &)), SLOT(onIncSearch_textChanged(const QString&)));
    connect(m_ui.checkBox_search, SIGNAL(stateChanged (int)), SLOT(onSearchCheckBoxStateChanged(int)));
    connect(m_ui.pushButton_searchNext, SIGNAL(clicked()), SLOT(onSearchNext()));
    connect(m_ui.pushButton_searchPrev, SIGNAL(clicked()), SLOT(onSearchPrev()));
    
    m_ui.widget_search->hide();

    
    //Setup the class treewidget
    treeWidget = m_ui.treeWidget_classes;
    treeWidget->setColumnWidth(0, 200);
    connect(m_ui.treeWidget_classes, SIGNAL(itemClicked(QTreeWidgetItem * , int )),
            SLOT(onFuncWidgetItemSelected(QTreeWidgetItem * , int )));



    installEventFilter(this);

    loadConfig();

    // EditorTabWidget
    connect(m_ui.editorTabWidget, SIGNAL(tabCloseRequested(int)), SLOT(onCodeViewTab_tabCloseRequested(int)));
    connect(m_ui.editorTabWidget, SIGNAL(currentChanged(int)), SLOT(onCodeViewTab_currentChanged(int)));
    m_ui.editorTabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_ui.editorTabWidget, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(onCodeViewTab_launchContextMenu(const QPoint&)));

    statusBar()->addPermanentWidget(&m_statusLineWidget);


    m_gui_default_mainwindowState = saveState();
    m_gui_default_mainwindowGeometry = saveGeometry();
    m_gui_default_splitter1State = m_ui.splitter_1->saveState();
    m_gui_default_splitter2State = m_ui.splitter_2->saveState();
    m_gui_default_splitter3State = m_ui.splitter_3->saveState();
    m_gui_default_splitter4State = m_ui.splitter_4->saveState();

    m_ui.targetOutputView->setConfig(&m_cfg);
    
    connect(m_ui.verticalScrollBar_console, SIGNAL(valueChanged(int)), m_ui.targetOutputView, SLOT(onScrollBar_valueChanged(int)));

    m_ui.targetOutputView->setScrollBar(m_ui.verticalScrollBar_console);

    connect(this, SIGNAL(newInfoMsg(QString)), SLOT(onNewInfoMsg(QString)));
    connect(this, SIGNAL(newWarnMsg(QString)), SLOT(onNewWarnMsg(QString)));
    connect(this, SIGNAL(newErrorMsg(QString)), SLOT(onNewErrorMsg(QString)));
    connect(this, SIGNAL(newCritMsg(QString)), SLOT(onNewCritMsg(QString)));

    connect(m_ui.gdbPrompt, &QLineEdit::returnPressed, this, &MainWindow::onUserGdbCommandEntered);

    loggerRegister(this);

}



MainWindow::~MainWindow()
{
    loggerUnregister(this);
 
}


/**
 * @brief Displays all the widgets (Eg: stack, breakpoints).
 */
void MainWindow::showWidgets()
{
    if(!m_cfg.m_viewFuncFilter)
        m_funcFilterText.clear();
    if(!m_cfg.m_viewClassFilter)
        m_classFilterText.clear();


    m_ui.actionViewFunctionFilter->setChecked(m_cfg.m_viewFuncFilter);
    m_ui.actionViewClassFilter->setChecked(m_cfg.m_viewClassFilter);


    QWidget *currentSelection = m_ui.tabWidget->currentWidget();
    m_ui.tabWidget->clear();

//
    QTreeWidget *stackWidget = m_ui.treeWidget_stack;
    if(m_cfg.m_viewWindowStack)
        m_ui.tabWidget->insertTab(0, stackWidget, "Stack");

//
    QTreeWidget *breakpointsWidget = m_ui.treeWidget_breakpoints;
    if(m_cfg.m_viewWindowBreakpoints)
        m_ui.tabWidget->insertTab(0, breakpointsWidget, "Breakpoints");

//
    QTreeWidget *threadsWidget = m_ui.treeWidget_threads;
    if(m_cfg.m_viewWindowThreads)
        m_ui.tabWidget->insertTab(0, threadsWidget, "Threads");

    int selectionIdx = m_ui.tabWidget->indexOf(currentSelection);
    if(selectionIdx != -1)
        m_ui.tabWidget->setCurrentIndex(selectionIdx);
    m_ui.tabWidget->setVisible(m_ui.tabWidget->count() == 0 ? false : true);
    

    m_ui.lineEdit_funcFilter->setVisible(m_cfg.m_viewFuncFilter);
    m_ui.pushButton_clearFuncFilter->setVisible(m_cfg.m_viewFuncFilter);
    m_ui.checkBox_funcFilter->setVisible(m_cfg.m_viewFuncFilter);
    if(m_cfg.m_viewFuncFilter)
        m_ui.checkBox_funcFilter->setCheckState(Qt::Checked);
    
    m_ui.lineEdit_classFilter->setVisible(m_cfg.m_viewClassFilter);
    m_ui.pushButton_clearClassFilter->setVisible(m_cfg.m_viewClassFilter);
    m_ui.checkBox_classFilter->setVisible(m_cfg.m_viewClassFilter);
    if(m_cfg.m_viewClassFilter)
        m_ui.checkBox_classFilter->setCheckState(Qt::Checked);
    
    
    m_ui.varWidget->setVisible(m_cfg.m_viewWindowWatch);
    m_ui.autoWidget->setVisible(m_cfg.m_viewWindowAutoVariables);
    m_ui.treeWidget_file->setVisible(m_cfg.m_viewWindowFileBrowser);


    currentSelection = m_ui.tabWidget_2->currentWidget();
    m_ui.tabWidget_2->clear();
    if(m_cfg.m_viewWindowTargetOutput)
        m_ui.tabWidget_2->insertTab(0, m_ui.widget_console, "Target Console");
    if(m_cfg.m_viewWindowGedeOutput)
        m_ui.tabWidget_2->insertTab(0, m_ui.gedeOutputWidget, "Gede Output");
    if(m_cfg.m_viewWindowGdbOutput)
        m_ui.tabWidget_2->insertTab(0, m_ui.tab_4, "GDB Output");
    selectionIdx = m_ui.tabWidget_2->indexOf(currentSelection);
    if(selectionIdx != -1)
        m_ui.tabWidget_2->setCurrentIndex(selectionIdx);
    m_ui.tabWidget_2->setVisible(m_ui.tabWidget_2->count() == 0 ? false : true);


}


/**
 * @param Called when user selects 'Restore Default View'.
 */
void MainWindow::onDefaultViewSetup()
{
    m_cfg.m_viewWindowStack = true;
    m_cfg.m_viewWindowBreakpoints = true;
    m_cfg.m_viewWindowThreads = true;
    m_cfg.m_viewWindowWatch = true;
    m_cfg.m_viewWindowAutoVariables = true;
    m_cfg.m_viewWindowTargetOutput = true;
    m_cfg.m_viewWindowGdbOutput = true;
    m_cfg.m_viewWindowFileBrowser = true;
    m_cfg.m_viewFuncFilter = true;
    m_cfg.m_viewClassFilter = true;

    m_cfg.m_gui_mainwindowGeometry = m_gui_default_mainwindowGeometry;
    m_cfg.m_gui_mainwindowState = m_gui_default_mainwindowState;
    m_cfg.m_gui_splitter1State = m_gui_default_splitter1State;
    m_cfg.m_gui_splitter2State = m_gui_default_splitter2State;
    m_cfg.m_gui_splitter3State = m_gui_default_splitter3State;
    m_cfg.m_gui_splitter4State = m_gui_default_splitter4State;



    m_ui.actionViewStack->setChecked(m_cfg.m_viewWindowStack);
    m_ui.actionViewThreads->setChecked(m_cfg.m_viewWindowThreads);
    m_ui.actionViewBreakpoints->setChecked(m_cfg.m_viewWindowBreakpoints);
    m_ui.actionViewWatch->setChecked(m_cfg.m_viewWindowWatch);
    m_ui.actionViewAutoVariables->setChecked(m_cfg.m_viewWindowAutoVariables);
    m_ui.actionViewTargetOutput->setChecked(m_cfg.m_viewWindowTargetOutput);
    m_ui.actionViewGedeOutput->setChecked(m_cfg.m_viewWindowGedeOutput);
    m_ui.actionViewGdbOutput->setChecked(m_cfg.m_viewWindowGdbOutput);
    m_ui.actionViewFileBrowser->setChecked(m_cfg.m_viewWindowFileBrowser);
    m_ui.actionViewFunctionFilter->setChecked(m_cfg.m_viewFuncFilter);
    m_ui.actionViewClassFilter->setChecked(m_cfg.m_viewClassFilter);
    


    showWidgets();

    showEvent(NULL);
    
}


void MainWindow::onUserGdbCommandEntered()
{
    // Get the command to execute
    QString cmd = m_ui.gdbPrompt->displayText();
    m_ui.gdbPrompt->clear();

    // Add it to the log
    m_ui.logView->appendPlainText(cmd);

    // Execute it
    Core &core = Core::getInstance();
    core.gdbManualCommand(cmd);

}

void MainWindow::onViewStack()
{
    m_cfg.m_viewWindowStack = m_cfg.m_viewWindowStack == true ? 0 : 1;

    showWidgets();
}

void MainWindow::onViewBreakpoints()
{
    m_cfg.m_viewWindowBreakpoints = m_cfg.m_viewWindowBreakpoints == true ? 0 : 1;

    showWidgets();
}

void MainWindow::onViewThreads()
{
    m_cfg.m_viewWindowThreads = m_cfg.m_viewWindowThreads == true ? 0 : 1;

    showWidgets();
}

void MainWindow::onViewWatch()
{
      m_cfg.m_viewWindowWatch = m_cfg.m_viewWindowWatch ? false : true;

    showWidgets();
}

void MainWindow::onViewAutoVariables()
{
      m_cfg.m_viewWindowAutoVariables = m_cfg.m_viewWindowAutoVariables ? false : true;
     
    showWidgets();
}

void MainWindow::onViewTargetOutput()
{
      m_cfg.m_viewWindowTargetOutput = m_cfg.m_viewWindowTargetOutput ? false : true;
     
    showWidgets();
}


void MainWindow::onViewGedeOutput()
{
    m_cfg.m_viewWindowGedeOutput = m_cfg.m_viewWindowGedeOutput ? false : true;
     
    showWidgets();
}

void MainWindow::onViewGdbOutput()
{
      m_cfg.m_viewWindowGdbOutput = m_cfg.m_viewWindowGdbOutput ? false : true;
     
    showWidgets();
}

void MainWindow::onViewFileBrowser()
{
    m_cfg.m_viewWindowFileBrowser = m_cfg.m_viewWindowFileBrowser ? false : true;
    
    showWidgets();
}


void MainWindow::onViewClassFilter()
{
    m_cfg.m_viewClassFilter = m_ui.actionViewClassFilter->isChecked() ? true : false;
    
    showWidgets();
}

void MainWindow::onViewFuncFilter()
{
    m_cfg.m_viewFuncFilter = m_ui.actionViewFunctionFilter->isChecked() ? true : false;
    
    showWidgets();
}



void MainWindow::loadConfig()
{
    m_cfg.load();


    setConfig();
    
    
    m_cfg.save();

    m_ui.actionViewStack->setChecked(m_cfg.m_viewWindowStack);
    m_ui.actionViewThreads->setChecked(m_cfg.m_viewWindowThreads);
    m_ui.actionViewBreakpoints->setChecked(m_cfg.m_viewWindowBreakpoints);
    m_ui.actionViewWatch->setChecked(m_cfg.m_viewWindowWatch);
    m_ui.actionViewAutoVariables->setChecked(m_cfg.m_viewWindowAutoVariables);
    m_ui.actionViewTargetOutput->setChecked(m_cfg.m_viewWindowTargetOutput);
    m_ui.actionViewGedeOutput->setChecked(m_cfg.m_viewWindowGedeOutput);
    m_ui.actionViewGdbOutput->setChecked(m_cfg.m_viewWindowGdbOutput);
    m_ui.actionViewFileBrowser->setChecked(m_cfg.m_viewWindowFileBrowser);
    m_ui.actionViewFunctionFilter->setChecked(m_cfg.m_viewFuncFilter);
    m_ui.actionViewClassFilter->setChecked(m_cfg.m_viewClassFilter);

    showWidgets();
    

}

void MainWindow::showEvent(QShowEvent* e)
{
    Q_UNUSED(e);

    restoreGeometry(m_cfg.m_gui_mainwindowGeometry);
    restoreState(m_cfg.m_gui_mainwindowState);
    m_ui.splitter_1->restoreState(m_cfg.m_gui_splitter1State);
    m_ui.splitter_2->restoreState(m_cfg.m_gui_splitter2State);
    m_ui.splitter_3->restoreState(m_cfg.m_gui_splitter3State);
    m_ui.splitter_4->restoreState(m_cfg.m_gui_splitter4State);

}

void MainWindow::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e);

    m_cfg.m_gui_mainwindowState = saveState();
    m_cfg.m_gui_mainwindowGeometry = saveGeometry();
    m_cfg.m_gui_splitter1State = m_ui.splitter_1->saveState();
    m_cfg.m_gui_splitter2State = m_ui.splitter_2->saveState();
    m_cfg.m_gui_splitter3State = m_ui.splitter_3->saveState();
    m_cfg.m_gui_splitter4State = m_ui.splitter_4->saveState();

    m_cfg.save();

}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        QWidget *widget = QApplication::focusWidget();

        // 'Delete' key pressed in the var widget?
        if(widget == m_ui.varWidget)
        {
            m_watchVarCtl.onKeyPress(keyEvent);
            
        }

        if(keyEvent->key() == Qt::Key_Escape)
        {
            hideSearchBox();
        }
        //qDebug() << "key " << keyEvent->key() << " from " << obj << "focus " << widget;

    }
    return QObject::eventFilter(obj, event);
}


/**
 * @brief Execution has stopped.
 * @param path     Path of the source file last executed. 
 * @param lineNo   The line which is about to execute (1=first).
 */
void MainWindow::ICore_onStopped(ICore::StopReason reason, QString path, int lineNo)
{
    if(m_cfg.m_focusOnStop)
        activateWindow();
    
    if(reason == ICore::EXITED_NORMALLY || reason == ICore::EXITED)
    {
        QString title = "Program exited";
        QString text;
        if(reason == ICore::EXITED_NORMALLY)
            text = "Program exited normally";
        else
            text = "Program exited";
        QMessageBox::information (this, title, text); 
    }
    
    updateCurrentLine(path, lineNo);
    

    fillInStack();
}



/**
 * @brief Finds a child to a treewidget node.
 */
QTreeWidgetItem *findTreeWidgetChildByName(QTreeWidget *treeWidget, QTreeWidgetItem *parent, QString name)
{
    QTreeWidgetItem *foundItem = NULL;
    if(parent == NULL)
        parent = treeWidget->invisibleRootItem();

    for(int i = 0;i < parent->childCount() && foundItem == NULL;i++)
    {
        QTreeWidgetItem *childItem = parent->child(i);
        if(childItem->text(0) == name)
            foundItem = childItem;
    }
    return foundItem;
}

    
/**
 * @brief Adds a path of directories to a tree widget.
 * @return returns the root directory of the newly created directories.
 */
QTreeWidgetItem *MainWindow::addTreeWidgetPath(QTreeWidget *treeWidget, QTreeWidgetItem *parent, QString path)
{
    QString firstName;
    QString restPath;
    QTreeWidgetItem *newItem = NULL;


    // Divide the path into a folder and name part.
    firstName = path;
    int divPos = path.indexOf('/');
    if(divPos != -1)
    {
        firstName = path.left(divPos);        
        restPath = path.mid(divPos+1);
    }

    // Handle "../" paths
    if(firstName == ".." && parent != NULL)
    {
        return addTreeWidgetPath(treeWidget, parent->parent(), restPath);
    }
    
    // Empty name and only a path?
    if(firstName.isEmpty())
    {
        if(restPath.isEmpty())
            return NULL;
        else
            return addTreeWidgetPath(treeWidget, parent, restPath);
    }
        
//    debugMsg("inserting: '%s', '%s'\n", stringToCStr(firstName), stringToCStr(restPath));


    // Check if the item already exist?
    newItem = findTreeWidgetChildByName(treeWidget, parent, firstName);

    
    

    // Add the item
    if(newItem == NULL)
    {
        newItem = new QTreeWidgetItem;
        newItem->setText(0, firstName); 
        newItem->setIcon(0, m_folderIcon);
    }
    if(parent == NULL)
    {
        treeWidget->insertTopLevelItem(0, newItem);
    }
    else
    {
        parent->addChild(newItem);
        if(parent->text(0) != "usr" && parent->text(0) != "opt")
            parent->setExpanded(true);
    }

    if(restPath.isEmpty())
        return newItem;
    else
        return addTreeWidgetPath(treeWidget, newItem, restPath);
}


/**
 * @brief Try to shrink a tree by removing dirs in the tree. Eg: "/usr/include/bits" => "/usr...bits".
 */ 
void MainWindow::wrapSourceTree(QTreeWidget *treeWidget)
{
    for(int u = 0;u < treeWidget->topLevelItemCount();u++)
    {
        QTreeWidgetItem* rootItem = treeWidget->topLevelItem (u);
        QTreeWidgetItem* childItem = rootItem->child(0);
        QString newName =  "/" + rootItem->text(0);
        if(!childItem)
            continue;
        
        do
        {
            if(childItem->childCount() > 0 && rootItem->childCount() == 1)
            {
                newName += "/" + childItem->text(0);
                QList<QTreeWidgetItem *> subChildren = childItem->takeChildren();
                childItem = rootItem->takeChild(0);
                delete childItem;
                rootItem->addChildren(subChildren);

                childItem = subChildren.first();
            }
        }
        while(childItem->childCount() > 0 && rootItem->childCount() == 1);

        if(rootItem->text(0) != "usr")
            rootItem->setExpanded(true);

        rootItem->setText(0, newName);

    }
}


/**
 * @brief Fills in the source file treeview.
 */
void MainWindow::insertSourceFiles()
{
    QTreeWidget *treeWidget = m_ui.treeWidget_file;
    Core &core = Core::getInstance();

    m_tagManager.abort();

    treeWidget->clear();
    
    // Get source files
    QVector <SourceFile*> sourceFiles = core.getSourceFiles();
    m_sourceFiles.clear();
    for(int i = 0;i < sourceFiles.size();i++)
    {
        SourceFile* source = sourceFiles[i];

        // Ignore directory?
        bool ignore = false;
        for(int j = 0;j < m_cfg.m_sourceIgnoreDirs.size();j++)
        {
            QString ignoreDir = m_cfg.m_sourceIgnoreDirs[j];
            if(!ignoreDir.isEmpty())
            {
                if(source->m_fullName.startsWith(ignoreDir))
                    ignore = true;
            }
        }

        if(!ignore)
        {
            
            // File exist?
            if(!QFileInfo(source->m_fullName).exists())
            {
                debugMsg("File '%s' does not exist", qPrintable(source->m_fullName));
            }
            else    
            {
                FileInfo info;
                info.m_name = source->m_name;
                info.m_fullName = source->m_fullName;
                
                m_sourceFiles.push_back(info);
            }
        }
    }

    

    // Queue all scans
    QStringList queueList;
    for(int i = 0;i < m_sourceFiles.size();i++)
    {
        FileInfo &info = m_sourceFiles[i];
        queueList += info.m_fullName;
    }
    m_tagManager.queueScan(queueList);

    
    for(int i = 0;i < m_sourceFiles.size();i++)
    {
        const FileInfo &info = m_sourceFiles[i];
        QTreeWidgetItem *parentNode  = NULL;

        // Get parent path
        QString folderPath;
        QString filename;
        dividePath(info.m_fullName, &filename, &folderPath);
        folderPath = simplifyPath(folderPath);
        
        if(!folderPath.isEmpty())
            parentNode = addTreeWidgetPath(treeWidget, NULL, folderPath);
            

        // Check if the item already exist?
        QTreeWidgetItem *item = findTreeWidgetChildByName(treeWidget, parentNode, filename);
        
        if(item == NULL)
        {
            item = new QTreeWidgetItem;
            item->setText(0, filename);
            item->setData(0, Qt::UserRole, info.m_fullName);
            item->setIcon(0, m_fileIcon);
            
            if(parentNode == NULL)
                treeWidget->insertTopLevelItem(0, item);
            else
            {
                parentNode->addChild(item);
                parentNode->setExpanded(true);
            }
        }
    }

    wrapSourceTree(treeWidget);

    treeWidget->sortItems(0, Qt::AscendingOrder);

}



void MainWindow::ICore_onLocalVarChanged(QStringList varNames)
{
    m_autoVarCtl.ICore_onLocalVarChanged(varNames);
}


void MainWindow::ICore_onWatchVarDeleted(VarWatch &watch)
{
    m_watchVarCtl.ICore_onWatchVarDeleted(watch);
    m_autoVarCtl.ICore_onWatchVarDeleted(watch);

}

void MainWindow::ICore_onWatchVarChanged(VarWatch &watch)
{
    m_watchVarCtl.ICore_onWatchVarChanged(watch);
    m_autoVarCtl.ICore_onWatchVarChanged(watch);
    
}


void MainWindow::ICore_onWatchVarChildAdded(VarWatch &watch)
{
    m_watchVarCtl.ICore_onWatchVarChildAdded(watch);
    m_autoVarCtl.ICore_onWatchVarChildAdded(watch);
}


void MainWindow::ICore_onSourceFileChanged(QString filename)
{
    CodeViewTab* codeViewTab = findTab(filename);
    if(codeViewTab)
    {
        // Get the tags in the file
        QList<Tag> tagList;
        m_tagManager.scan(filename, &tagList);

        // Open the file
        if(codeViewTab->open(filename,tagList))
        {
        }
    }
}


void MainWindow::ICore_onSourceFileListChanged()
{
    insertSourceFiles();
}

/**
 * @brief User doubleclicked on the border
 * @param lineNo    The line pressed (1=first row).
 */
void MainWindow::ICodeView_onRowDoubleClick(int lineNo)
{
    Core &core = Core::getInstance();

    CodeViewTab* currentCodeViewTab = currentTab();
    assert(currentCodeViewTab != NULL);
    if(!currentCodeViewTab)
        return;
        
    BreakPoint* bkpt = core.findBreakPoint(currentCodeViewTab->getFilePath(), lineNo);
    if(bkpt)
        core.gdbRemoveBreakpoint(bkpt);
    else
        core.gdbSetBreakpoint(currentCodeViewTab->getFilePath(), lineNo);
}


    

void MainWindow::onNewWarnMsg(QString msg)
{
    QString text = "WARN | " + msg; 
    text.replace(" ", "&nbsp;");
    text = "<font color=\"Purple\">" + text + "</font>";
    m_ui.gedeOutputWidget->append(text);
}

void MainWindow::onNewErrorMsg(QString msg)
{
    QString text = "ERROR| " + msg; 
    text.replace(" ", "&nbsp;");
    text = "<font color=\"Red\">" + text + "</font>";

    m_ui.gedeOutputWidget->append(text);
}

void MainWindow::onNewCritMsg(QString msg)
{
    QString text = "ERROR| " + msg; 
    text.replace(" ", "&nbsp;");
    text = "<font color=\"Red\">" + text + "</font>";

    m_ui.gedeOutputWidget->append(text);

    QMessageBox::critical(NULL, QString("Gede - Error"), QString(msg));
}

void MainWindow::onNewInfoMsg(QString msg)
{
    QString text = "     | " + msg; 
    text.replace(" ", "&nbsp;");
    text = "<font color=\"Black\">" + text + "</font>";

    m_ui.gedeOutputWidget->append(text);
}

void MainWindow::ILogger_onWarnMsg(QString text)
{
    emit newWarnMsg(text);
}


void MainWindow::ILogger_onErrorMsg(QString text)
{
    emit newErrorMsg(text);
}

void MainWindow::ILogger_onCriticalMsg(QString text)
{
    emit newCritMsg(text);
}

void MainWindow::ILogger_onInfoMsg(QString text)
{
    emit newInfoMsg(text);
}


void MainWindow::ICore_onConsoleStream(QString text)
{
    m_ui.logView->appendPlainText(text);
}

void MainWindow::ICore_onMessage(QString message)
{
    m_ui.logView->appendPlainText(message);

}
    

void MainWindow::fillInStack()
{
    Core &core = Core::getInstance();
    
    core.getStackFrames();

}


void
MainWindow::onThreadWidgetSelectionChanged( )
{
    // Get the new selected thread
    QTreeWidget *threadWidget = m_ui.treeWidget_threads;
    QList <QTreeWidgetItem *> selectedItems = threadWidget->selectedItems();
    if(selectedItems.size() > 0)
    {
        QTreeWidgetItem * currentItem;
        currentItem = selectedItems[0];
        int selectedThreadId = currentItem->data(0, Qt::UserRole).toInt();

        // Select the thread
        Core &core = Core::getInstance();
        core.selectThread(selectedThreadId);
    }
}

void MainWindow::onStackWidgetSelectionChanged()
{
    Core &core = Core::getInstance();
        
    int selectedFrame = -1;
    // Get the new selected frame
    QTreeWidget *stackWidget = m_ui.treeWidget_stack;
    QList <QTreeWidgetItem *> selectedItems = stackWidget->selectedItems();
    if(selectedItems.size() > 0)
    {
        QTreeWidgetItem * currentItem;
        currentItem = selectedItems[0];
        selectedFrame = currentItem->data(0, Qt::UserRole).toInt();

        core.selectFrame(selectedFrame);
    }
}







void MainWindow::onFolderViewItemActivated ( QTreeWidgetItem * item, int column )
{
    Q_UNUSED(column);

    if(item->childCount() == 0)
    {
        QString filename  = item->data(0, Qt::UserRole).toString();

        open(filename);
    }
}

CodeViewTab* MainWindow::currentTab()
{
    return (CodeViewTab*)m_ui.editorTabWidget->currentWidget();
}


/**
 * @brief Called when user right clicks on tab and chooses 'Close Tabs To The Left'.
 */
void MainWindow::onCodeViewTab_closeTabsToLeft()
{
    // Get the selected tab
    QAction *action = static_cast<QAction *>(sender ());
    int tabIdx  = action->data().toInt();

    for(int i = 0;i < tabIdx;i++)
    {
        m_ui.editorTabWidget->removeTab(0);
    }
}


/**
 * @brief Called when user right clicks on tab and chooses 'Close Other Tabs'.
 */
void MainWindow::onCodeViewTab_closeOtherTabs()
{
    // Get the selected tab
    QAction *action = static_cast<QAction *>(sender ());
    int tabIdx  = action->data().toInt();

    for(int i = 0;i < tabIdx;i++)
    {
        m_ui.editorTabWidget->removeTab(0);
    }
    while(m_ui.editorTabWidget->count() >= 2)
    {
        m_ui.editorTabWidget->removeTab(1);
    }
}


/**
 * @brief Called when the user right clicks on the opened file tab widget.
 */
void MainWindow::onCodeViewTab_launchContextMenu(const QPoint& pos)
{
    QAction *action;
    QString title;

    // Get tab
    int tabIdx = m_ui.editorTabWidget->tabAt(pos);
    if(tabIdx == -1)
        return;

    m_popupMenu.clear();

    action = m_popupMenu.addSeparator();

    // Add 'Close Other Tabs'
    action = m_popupMenu.addAction("Close Other Tabs");
    action->setData(tabIdx);
    connect(action, SIGNAL(triggered()), this, SLOT(onCodeViewTab_closeOtherTabs()));

        
    // Add 'Close Tabs To The Left'
    action = m_popupMenu.addAction("Close Tabs To The Left");
    action->setData(tabIdx);
    connect(action, SIGNAL(triggered()), this, SLOT(onCodeViewTab_closeTabsToLeft()));

    
    QPoint popupPos = m_ui.editorTabWidget->mapToGlobal(pos);
    m_popupMenu.popup(popupPos);
}

void MainWindow::onCodeViewTab_currentChanged( int tabIdx)
{
    Q_UNUSED(tabIdx);
}


void MainWindow::onCodeViewTab_tabCloseRequested ( int tabIdx)
{
    CodeViewTab *codeViewTab = (CodeViewTab *)m_ui.editorTabWidget->widget(tabIdx);
    m_ui.editorTabWidget->removeTab(tabIdx);
    delete codeViewTab;
}


/**
 * @brief Opens a source file in the sourcecode viewer and highlights a specific line.
 */
CodeViewTab* MainWindow::open(Location loc)
{
    return open(loc.m_filename, loc.m_lineNo);
}


/**
 * @brief Opens a source file in the sourcecode viewer and highlights a specific line.
 */
CodeViewTab* MainWindow::open(QString filename, int lineNo)
{
    CodeViewTab* currentCodeViewTab = open(filename);
    if(currentCodeViewTab)
    {
        debugMsg("Ensuring that line %d is visible", lineNo);
        currentCodeViewTab->ensureLineIsVisible(lineNo);    
    }
    return currentCodeViewTab;
}


/**
* @brief Finds a tab that has a specific sourcefile opened.
*/
CodeViewTab* MainWindow::findTab(QString filename)
{
    for(int tabIdx = 0;tabIdx <  m_ui.editorTabWidget->count();tabIdx++)
    {
        CodeViewTab* tab = (CodeViewTab* )m_ui.editorTabWidget->widget(tabIdx);
        if(tab->getFilePath() == filename)
        {
            //debugMsg("Found already opened '%s'", qPrintable(filename));
            return tab;
        }
    }
    return NULL;
}

/**
 * @brief Opens a source file in the sourcecode viewer.
 */
CodeViewTab* MainWindow::open(QString filename)
{
    debugMsg("%s(%s)", __func__, qPrintable(filename));
  
    if(filename.isEmpty())
        return NULL;

    // Already open?
    int foundCodeViewTabIdx = -1;
    for(int tabIdx = 0;tabIdx <  m_ui.editorTabWidget->count();tabIdx++)
    {
        CodeViewTab* tab = (CodeViewTab* )m_ui.editorTabWidget->widget(tabIdx);
        if(tab->getFilePath() == filename)
        {
            debugMsg("Found already opened '%s'", qPrintable(filename));
            foundCodeViewTabIdx = tabIdx;
            
        }
    }
    if(foundCodeViewTabIdx == -1)
        debugMsg("Did not find '%s'", qPrintable(filename));
    
    if(foundCodeViewTabIdx != -1 && foundCodeViewTabIdx == m_ui.editorTabWidget->currentIndex())
         return (CodeViewTab* )m_ui.editorTabWidget->currentWidget();
         
    // Remove search widget
    m_ui.widget_search->hide();
    onIncSearch_textChanged("");

    CodeViewTab* codeViewTab = NULL;
    if(foundCodeViewTabIdx != -1)
    {
        m_ui.editorTabWidget->setCurrentIndex(foundCodeViewTabIdx);
        codeViewTab = (CodeViewTab* )m_ui.editorTabWidget->widget(foundCodeViewTabIdx);
    }
    else
    {
        // Get the tags in the file
        QList<Tag> tagList;
        m_tagManager.scan(filename, &tagList);

        // Close if we have to many opened
        if(m_ui.editorTabWidget->count() >= m_cfg.m_maxTabs)
        {
            // Find the oldest tab
            int oldestTabIdx = -1;
            QTime tnow = QTime::currentTime();
            QTime oldestTabTime = tnow;
            for(int tabIdx = 0;tabIdx < m_ui.editorTabWidget->count();tabIdx++)
            {
                CodeViewTab* testTab = (CodeViewTab* )m_ui.editorTabWidget->widget(tabIdx);
                if(oldestTabIdx == -1 || testTab->getLastAccessTime() < oldestTabTime)
                {
                    oldestTabTime = testTab->getLastAccessTime();
                    oldestTabIdx = tabIdx;
                }
            }

            // Close the oldest tab
            CodeViewTab* oldestestTab = (CodeViewTab* )m_ui.editorTabWidget->widget(oldestTabIdx);
            m_ui.editorTabWidget->removeTab(oldestTabIdx);
            delete oldestestTab;
        }
            
        // Create the tab
        codeViewTab = new CodeViewTab(this);
        codeViewTab->setInterface(this);
        codeViewTab->setConfig(&m_cfg);

        if(codeViewTab->open(filename,tagList))
        {
            delete codeViewTab;
            return NULL;
        }

        // Add the new codeview tab
        m_ui.editorTabWidget->addTab(codeViewTab, getFilenamePart(filename));
        m_ui.editorTabWidget->setCurrentIndex(m_ui.editorTabWidget->count()-1);
    }

    codeViewTab->updateLastAccessStamp();
        
    // Set window title
    QString windowTitle;
    QString filenamePart, folderPathPart;
    dividePath(filename, &filenamePart, &folderPathPart);
    windowTitle = QString::asprintf("%s - %s",  stringToCStr(filenamePart), stringToCStr(folderPathPart));
    setWindowTitle(windowTitle);

    m_locator.setCurrentFile(filename);

    ICore_onBreakpointsChanged();

    return codeViewTab;
}


void MainWindow::onCurrentLineChanged(int lineno)
{
    for(int tabIdx = 0;tabIdx <  m_ui.editorTabWidget->count();tabIdx++)
    {
        CodeViewTab* codeViewTab = (CodeViewTab* )m_ui.editorTabWidget->widget(tabIdx);
        if(codeViewTab->getFilePath() == m_currentFile)
            codeViewTab->setCurrentLine(lineno);
    }

}


void MainWindow::onCurrentLineDisabled()
{
    for(int tabIdx = 0;tabIdx <  m_ui.editorTabWidget->count();tabIdx++)
    {
        CodeViewTab* codeViewTab = (CodeViewTab* )m_ui.editorTabWidget->widget(tabIdx);
    
        codeViewTab->disableCurrentLine();
    }

}

void MainWindow::updateCurrentLine(QString filename, int lineno)
{
    CodeViewTab* currentCodeViewTab = NULL;
    
    m_currentFile = filename;
    m_currentLine = lineno;

    if(filename.isEmpty())
        warnMsg("No filename information available");
    else
    {
        currentCodeViewTab = open(filename);
    }

    
    // Update the current line view
    if(currentCodeViewTab != NULL)
    {
        onCurrentLineChanged(m_currentLine);
        
        // Scroll to the current line
        if(currentCodeViewTab->getFilePath() == m_currentFile)
            currentCodeViewTab->ensureLineIsVisible(m_currentLine);
    }
    else
    {
        onCurrentLineDisabled();
    }

}

 
void MainWindow::onQuit()
{
    QApplication::instance()->quit();
}

void MainWindow::onStop()
{
    Core &core = Core::getInstance();
    core.stop();
}

void MainWindow::onNext()
{
    Core &core = Core::getInstance();
    core.gdbNext();
    
}



/**
 * @brief Called when user presses "Help->About". Shows the about box.
 */
void MainWindow::onAbout()
{
    AboutDialog dlg(this, &m_cfg);
    dlg.exec();
}


/**
 * @brief Called when user presses "Search->Search".
 */
void MainWindow::onSearch()
{
    m_ui.widget_search->show();
    m_ui.checkBox_search->setCheckState(Qt::Checked);

    m_ui.lineEdit_search->setFocus();
    m_ui.lineEdit_search->selectAll();
}

/**
 * @brief Called when user presses "Search->Go to main()".
 */
void MainWindow::onGoToMain()
{
    QVector<Location> locList = m_locator.locateFunction("main");

    if(locList.size() > 0)
    {
        open(locList[0]);
    }
}

/**
 * @brief Called when user presses "Search->Go to line".
 */
void MainWindow::onGoToLine()
{
    // Get currently opened file
    QString currentFilename;
    CodeViewTab* currentCodeViewTab = currentTab();
    assert(currentCodeViewTab != NULL);
    if(currentCodeViewTab)
        currentFilename = currentCodeViewTab->getFilePath();
    
    // Show dialog
    GoToDialog dlg(this, &m_locator, &m_cfg, currentFilename);
    if(dlg.exec() != QDialog::Accepted)
        return;

    dlg.saveSettings(&m_cfg);
    
    // Which file and line was selected?
    QString filename;
    int lineno;
    dlg.getSelection(&filename, &lineno);


    // Open file    
    if(!filename.isEmpty() && lineno > 0)
    {
        open(filename, lineno);
    }
    else
        warnMsg("Location not found!");
}

void MainWindow::onRestart()
{
    Core &core = Core::getInstance();

    m_ui.targetOutputView->clearAll();

    // Was the program modified since last time?
    QDateTime modTime = QFileInfo(m_cfg.getProgramPath()).lastModified();
    if(modTime > core.getTimeStarted())
    {
        // Ask the user what to do
        QMessageBox msgBox;
        msgBox.setWindowTitle("Program has been modified");
        msgBox.setText("The program has been modified since last run.");
        msgBox.setInformativeText("Do you want to reload it?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        if(ret == QMessageBox::Yes)
        {
            core.gdbReload(m_cfg);
        }
        else
            core.gdbRun();
    }
    else
        core.gdbRun();
}


void MainWindow::onContinue()
{
    Core &core = Core::getInstance();
    core.gdbContinue();

    onCurrentLineDisabled();
}


void MainWindow::onStepIn()
{
    Core &core = Core::getInstance();
    core.gdbStepIn();
    
}

void MainWindow::onStepOut()
{
    Core &core = Core::getInstance();
    core.gdbStepOut();
    
}


void MainWindow::ICore_onThreadListChanged()
{
    Core &core = Core::getInstance();

    QTreeWidget *threadWidget = m_ui.treeWidget_threads;
    threadWidget->clear();

    QList<ThreadInfo> list = core.getThreadList();
    
    for(int idx = 0;idx < list.size();idx++)
    {
        // Get name
        QString name = list[idx].m_name;
        QString desc = list[idx].m_details;
        

        // Add the item
        QStringList names;
        names.push_back(name);
        names.push_back(desc);
        QTreeWidgetItem *item = new QTreeWidgetItem(names);
        item->setData(0, Qt::UserRole, list[idx].m_id);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        threadWidget->insertTopLevelItem(0, item);

    }
}


void MainWindow::ICore_onCurrentThreadChanged(int threadId)
{
    QTreeWidget *threadWidget = m_ui.treeWidget_threads;
    QTreeWidgetItem *rootItem = threadWidget->invisibleRootItem();
    threadWidget->clearSelection();
    QTreeWidgetItem *selectItem = NULL;
    for(int i = 0;i < rootItem->childCount();i++)
    {
        QTreeWidgetItem *item = rootItem->child(i);
        assert(item != NULL);
        if(item)
        {
            int id = item->data(0, Qt::UserRole).toInt();
            if(id == threadId)
            {
                selectItem = item;
            }
        }
    }
    if(selectItem)
        threadWidget->setCurrentItem(selectItem);
}




void MainWindow::ICore_onBreakpointsChanged()
{
    Core &core = Core::getInstance();
    QList<BreakPoint*>  bklist = core.getBreakPoints();
    

    // Update the settings
    m_cfg.m_breakpoints.clear();
    for(int u = 0;u < bklist.size();u++)
    {
        BreakPoint* bkpt = bklist[u];
        SettingsBreakpoint bkptCfg;
        bkptCfg.m_filename = bkpt->m_fullname;
        bkptCfg.m_lineNo = bkpt->m_lineNo;
        m_cfg.m_breakpoints.push_back(bkptCfg);
    }
    m_cfg.save();
    

    // Update the breakpoint list widget
    m_ui.treeWidget_breakpoints->clear();
    for(int i = 0;i <  bklist.size();i++)
    {
        BreakPoint* bk = bklist[i];

        QStringList nameList;
        QString name;
        nameList.append(getFilenamePart(bk->m_fullname));
        name = QString::asprintf("%d", bk->m_lineNo);
        nameList.append(name);
        nameList.append(bk->m_funcName);
        nameList.append(longLongToHexString(bk->m_addr));
        
        

        QTreeWidgetItem *item = new QTreeWidgetItem(nameList);
        item->setData(0, Qt::UserRole, i);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        // Add the item to the widget
        m_ui.treeWidget_breakpoints->insertTopLevelItem(0, item);

    }

    // Update the fileview
    for(int tabIdx = 0;tabIdx <  m_ui.editorTabWidget->count();tabIdx++)
    {
        CodeViewTab* codeViewTab = (CodeViewTab* )m_ui.editorTabWidget->widget(tabIdx);

        QVector<int> numList;
        for(int i = 0;i <  bklist.size();i++)
        {
            BreakPoint* bk = bklist[i];

            if(bk->m_fullname == codeViewTab->getFilePath())
                numList.push_back(bk->m_lineNo);
        }

        codeViewTab->setBreakpoints(numList);
    }
}



void MainWindow::ICore_onStackFrameChange(QList<StackFrameEntry> stackFrameList)
{
    m_stackFrameList = stackFrameList;
    QTreeWidget *stackWidget = m_ui.treeWidget_stack;
    
    stackWidget->clear();


    
    for(int idx = 0;idx < stackFrameList.size();idx++)
    {
        // Get name
        StackFrameEntry &entry = stackFrameList[stackFrameList.size()-idx-1];
        

        // Create the item
        QStringList names;
        names.push_back(entry.m_functionName);
        
        QTreeWidgetItem *item = new QTreeWidgetItem(names);

        
        item->setData(0, Qt::UserRole, idx);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        // Add the item to the widget
        stackWidget->insertTopLevelItem(0, item);


    }
    
}



/**
 * @brief The current frame has changed.
 * @param frameIdx    The frame  (0 being the newest frame) 
*/
void MainWindow::ICore_onCurrentFrameChanged(int frameIdx)
{
    QTreeWidget *stackWidget = m_ui.treeWidget_stack;

    // Update the sourceview (with the current row).
    if(frameIdx >= 0 && frameIdx < m_stackFrameList.size())
    {
        StackFrameEntry &entry = m_stackFrameList[m_stackFrameList.size()-frameIdx-1];

        QString currentFile = entry.m_sourcePath;
        updateCurrentLine(currentFile, entry.m_line);
    }

    // Update the selection of the current thread
    QTreeWidgetItem *rootItem = stackWidget->invisibleRootItem();
    stackWidget->clearSelection();
    QTreeWidgetItem *selectItem = NULL;
    for(int i = 0;i < rootItem->childCount();i++)
    {
        QTreeWidgetItem *item = rootItem->child(i);
        int id = item->data(0, Qt::UserRole).toInt();
        if(id == frameIdx)
        {
            selectItem = item;
        }
    }
    if(selectItem)
        stackWidget->setCurrentItem(selectItem);
    
}

void MainWindow::ICore_onFrameVarReset()
{

}

void MainWindow::ICore_onFrameVarChanged(QString name, QString value)
{
    Q_UNUSED(name);
    Q_UNUSED(value);

}

void MainWindow::onBreakpointsWidgetItemDoubleClicked(QTreeWidgetItem * item,int column)
{
    Q_UNUSED(column);

    Core &core = Core::getInstance();
    QList<BreakPoint*>  bklist = core.getBreakPoints();
    int idx = item->data(0, Qt::UserRole).toInt();
    BreakPoint* bk = bklist[idx];

    CodeViewTab* currentCodeViewTab = open(bk->m_fullname);
    if(currentCodeViewTab)
        currentCodeViewTab->ensureLineIsVisible(bk->m_lineNo);
    
}
    


/**
 * @brief User has right clicked in the codeview on a include file.
 */
void MainWindow::ICodeView_onContextMenuIncFile(QPoint pos, int lineNo, QString incFile)
{
    QAction *action;
    QString title;

    Q_UNUSED(lineNo);
    
    m_popupMenu.clear();

    // Add 'Open <include file>'
    action = m_popupMenu.addAction("Open " + incFile);
    action->setData(incFile);
    connect(action, SIGNAL(triggered()), this, SLOT(onCodeViewContextMenuOpenFile()));

        
    // Add 'Show current PC location'
    action = m_popupMenu.addSeparator();
    title = "Show current PC location";
    action = m_popupMenu.addAction(title);
    connect(action, SIGNAL(triggered()), this, SLOT(onCodeViewContextMenuShowCurrentLocation()));


    
    m_popupMenu.popup(pos);

}


/**
 * @brief User has right clicked in the codeview.
 * @param lineNo    The row (1=first row).
 *
 */
void MainWindow::ICodeView_onContextMenu(QPoint pos, int lineNo, QStringList text)
{
    QAction *action;
    int totalItemCount = 0;


    m_tagManager.waitAll();

    // Create actions for each tag
    QList<QAction*> defActionList;
    bool onlyFuncs = true;
    for(int k = 0;k < text.size();k++)
    {
        // Get the tag to look for
        QString wantedTag = text[k];
        if(wantedTag.lastIndexOf('.') != -1)
            wantedTag = wantedTag.mid(wantedTag.lastIndexOf('.')+1);

        
        // Loop through all the source files
        for(int i = 0;i < m_sourceFiles.size();i++)
        {
            FileInfo& fileInfo = m_sourceFiles[i];

            QList<Tag> tagList;
            m_tagManager.getTags(fileInfo.m_fullName, &tagList);

            // Loop through all the tags
            for(int j = 0;j < tagList.size();j++)
            {
                Tag &tagInfo = tagList[j];
                QString tagName = tagInfo.m_name;

                // Tag match?
                if(tagName == wantedTag)
                {

                    if(totalItemCount++ < 20)
                    {
                        // Get filename and lineNo
                        QStringList defList;
                        defList.push_back(fileInfo.m_fullName);
                        QString lineNoStr;
                        lineNoStr = QString::asprintf("%d", tagInfo.getLineNo());
                        defList.push_back(lineNoStr);

                        if(!tagInfo.isFunc())
                            onlyFuncs = false;
                            
                        // Add to popupmenu
                        QString menuEntryText;
                        menuEntryText = QString::asprintf("Show definition of '%s' L%d", stringToCStr(tagInfo.getLongName()), tagInfo.getLineNo());
                        menuEntryText.replace("&", "&&");
                        QAction *action = new QAction(menuEntryText, &m_popupMenu);
                        action->setData(defList);
                        defActionList.push_back(action);
                    }
                }
            }
        }
    }


    m_popupMenu.clear();

    // Add 'Add to watch list'
    if(!onlyFuncs || totalItemCount == 0)
    {
        for(int i = text.size()-1;i >= 0;i--)
        {
            action = m_popupMenu.addAction("Add '" + text[i] + "' to watch list");
            action->setData(text[i]);
            connect(action, SIGNAL(triggered()), this, SLOT(onCodeViewContextMenuAddWatch()));

        }
    }


    // Add 'toggle breakpoint'
    QString title;
    m_popupMenu.addSeparator();
    title = QString::asprintf("Toggle breakpoint at L%d", lineNo);
    action = m_popupMenu.addAction(title);
    action->setData(lineNo);
    connect(action, SIGNAL(triggered()), this, SLOT(onCodeViewContextMenuToggleBreakpoint()));

    action = m_popupMenu.addSeparator();

    // Add to the menu
    for(int i = 0;i < defActionList.size();i++)
    {
        QAction *action = defActionList[i];
                        
        m_popupMenu.addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(onCodeViewContextMenuShowDefinition()));
    }

    
    // Add 'Show current PC location'
    action = m_popupMenu.addSeparator();
    title = "Show current PC location";
    action = m_popupMenu.addAction(title);
    connect(action, SIGNAL(triggered()), this, SLOT(onCodeViewContextMenuShowCurrentLocation()));

    // Add 'Jump to this location'
    action = m_popupMenu.addSeparator();
    title = "Jump to this location";
    action = m_popupMenu.addAction(title);
    action->setData(lineNo);
    connect(action, SIGNAL(triggered()), this, SLOT(onCodeViewContextMenuJumpToLocation()));

    
    m_popupMenu.popup(pos);
}


void MainWindow::onCodeViewContextMenuJumpToLocation()
{
    QAction *action = static_cast<QAction *>(sender ());
    int lineNo = action->data().toInt();
    Core &core = Core::getInstance();

    CodeViewTab* currentCodeViewTab = currentTab();
    if(!currentCodeViewTab)
        return;
        
    QString filename = currentCodeViewTab->getFilePath();

    core.jump(filename, lineNo);
}


void MainWindow::onCodeViewContextMenuToggleBreakpoint()
{
    QAction *action = static_cast<QAction *>(sender ());
    int lineNo = action->data().toInt();
    Core &core = Core::getInstance();

    CodeViewTab* currentCodeViewTab = currentTab();
    if(!currentCodeViewTab)
        return;
        
    BreakPoint* bkpt = core.findBreakPoint(currentCodeViewTab->getFilePath(), lineNo);
    if(bkpt)
        core.gdbRemoveBreakpoint(bkpt);
    else
        core.gdbSetBreakpoint(currentCodeViewTab->getFilePath(), lineNo);

}


void MainWindow::onCodeViewContextMenuShowCurrentLocation()
{
    // Open file
    CodeViewTab* currentCodeViewTab = open(m_currentFile);
    if(currentCodeViewTab)
        currentCodeViewTab->ensureLineIsVisible(m_currentLine);    
}


void MainWindow::onCodeViewContextMenuShowDefinition()
{
    
    // Get the selected function name
    QAction *action = static_cast<QAction *>(sender ());
    QStringList list = action->data().toStringList();

    // Get filepath and lineNo
    if(list.size() != 2)
        return;
    QString foundFilepath = list[0];
    int lineNo = list[1].toInt();

    // Open file
    CodeViewTab* codeViewTab = open(foundFilepath);

    // Scroll to the function
    if(codeViewTab)
        codeViewTab->ensureLineIsVisible(lineNo);
    

}

void MainWindow::onCodeViewContextMenuOpenFile()
{
    QString foundFilename;
    
    // Get the selected variable name
    QAction *action = static_cast<QAction *>(sender ());
    QString filename = action->data().toString();

    QString filenameWop = filename;
    int divPos = filenameWop.lastIndexOf('/');
    if(divPos != -1)
        filenameWop = filenameWop.mid(divPos+1);

    
    // First try the same dir as the currently open file
    CodeViewTab* currentCodeViewTab = currentTab();
    assert(currentCodeViewTab != NULL);
    QString folderPath;
    dividePath(currentCodeViewTab->getFilePath(), NULL, &folderPath);
    if(QFileInfo(folderPath + "/" + filename).exists())
        foundFilename = folderPath + "/" + filename;
    else
    {

        // Look in all the project files
        for(int j = 0;foundFilename == "" && j < m_sourceFiles.size();j++)
        {
            FileInfo &info = m_sourceFiles[j];
            if(info.m_fullName.endsWith("/" + filenameWop))
                foundFilename = info.m_fullName;
        }

        // otherwise look in all the dirs
        if(foundFilename == "")
        {
            // Get a list of all dirs to look in
            QStringList dirs;
            for(int j = 0;foundFilename == "" && j < m_sourceFiles.size();j++)
            {
                FileInfo &info = m_sourceFiles[j];
                dividePath(info.m_fullName, NULL, &folderPath);
                dirs.push_back(folderPath);
            }
            dirs.removeDuplicates();


            // Look in the dirs
            for(int j = 0;foundFilename == "" && j < dirs.size();j++)
            {
                QString testDir = dirs[j];
                if(QFileInfo(testDir + "/" + filenameWop).exists())
                    foundFilename = testDir + "/" + filenameWop;
            }
            
        }

    }

    // open the file
    if(!foundFilename.isEmpty())
        open(foundFilename);
    else
        critMsg("Unable to find '%s'", stringToCStr(filename));
        
}


void MainWindow::onCodeViewContextMenuAddWatch()
{
    // Get the selected variable name
    QAction *action = static_cast<QAction *>(sender ());
    QString varName = action->data().toString();

    m_watchVarCtl.addNewWatch(varName);
    
    
}



/**
 * @brief Update the GUI with the settings in the config
 */
void MainWindow::setConfig()
{
    qApp->setStyle(m_cfg.m_guiStyleName);

    for(int tabIdx = 0;tabIdx <  m_ui.editorTabWidget->count();tabIdx++)
    {
        CodeViewTab* codeViewTab = (CodeViewTab* )m_ui.editorTabWidget->widget(tabIdx);
        codeViewTab->setConfig(&m_cfg);
    }
    
    m_gdbOutputFont = QFont(m_cfg.m_gdbOutputFontFamily, m_cfg.m_gdbOutputFontSize);
    m_ui.logView->setFont(m_gdbOutputFont);

    m_gedeOutputFont = QFont(m_cfg.m_gedeOutputFontFamily, m_cfg.m_gedeOutputFontSize);
    m_ui.gedeOutputWidget->setFont(m_gedeOutputFont);

    m_outputFont = QFont(m_cfg.m_outputFontFamily, m_cfg.m_outputFontSize);
    m_ui.targetOutputView->setMonoFont(m_outputFont);
    
    m_autoVarCtl.setConfig(&m_cfg);

    m_tagManager.setConfig(m_cfg);
}


void MainWindow::onSettings()
{
    QString oldStyleName = m_cfg.m_guiStyleName;

    SettingsDialog dlg(this, &m_cfg);
    if(dlg.exec() == QDialog::Accepted)
    {
        dlg.getConfig(&m_cfg);

        setConfig();
        
        
        m_cfg.save();

        if(m_cfg.m_guiStyleName.isEmpty() && !oldStyleName.isEmpty())
        {
            //
            QString msgText;
            msgText = QString::asprintf("Style will be changed after gede has been restarted.");
            QString title = "Style changed";
            QMessageBox::warning(this, title, msgText);
   
        }
            
    }
   
}

void MainWindow::ICore_onSignalReceived(QString signalName)
{
    if(signalName != "SIGINT")
    {
        //
        QString msgText;
        msgText = QString::asprintf("Program received signal %s.", stringToCStr(signalName));
        QString title = "Signal received";
        QMessageBox::warning(this, title, msgText);
    }
    
    onCurrentLineDisabled();
        
    fillInStack();

}

void MainWindow::ICore_onTargetOutput(QString message)
{

    m_ui.targetOutputView->appendLog(message);
    
}



void MainWindow::ICore_onStateChanged(TargetState state)
{
    bool isRunning = true;
    if (state == TARGET_STOPPED || state == TARGET_FINISHED)
        isRunning = false;
    bool isStopped = state == TARGET_STOPPED ? true : false;
    
    m_ui.actionNext->setEnabled(isStopped);
    m_ui.actionStep_In->setEnabled(isStopped);
    m_ui.actionStep_Out->setEnabled(isStopped);
    m_ui.actionStop->setEnabled(isRunning);
    m_ui.actionContinue->setEnabled(isStopped);
    m_ui.actionRestart->setEnabled(!isRunning);

    m_ui.varWidget->setEnabled(!isRunning);

    
    if(state == TARGET_STARTING || state == TARGET_RUNNING)
    {
        m_ui.treeWidget_stack->clear();
    }
    m_autoVarCtl.ICore_onStateChanged(state);
}


/**
* @brief Sets the status line in the mainwindow
*/
void MainWindow::setStatusLine(Settings &cfg)
{
    MainWindow &w = *this;
    QString statusText;
    if(cfg.m_connectionMode == MODE_LOCAL)
    {
        QString argumentText;
        for(int j = 0;j < cfg.m_argumentList.size();j++)
        {
            argumentText += "\"" + cfg.m_argumentList[j] + "\"";
            if(j+1 != cfg.m_argumentList.size())
                argumentText += ",";
        }
        if(argumentText.length() > 50)
        {
            argumentText = argumentText.left(50);
            argumentText += "...";
        }
        statusText = QString::asprintf("[%s] [%s]", stringToCStr(cfg.getProgramPath()), stringToCStr(argumentText));
    }
    else if(cfg.m_connectionMode == MODE_COREDUMP)
    {
        statusText = QString::asprintf("[%s] [%s]", stringToCStr(cfg.getProgramPath()), stringToCStr(cfg.m_coreDumpFile));
    }
    else if(cfg.m_connectionMode == MODE_PID)
    {
        statusText = QString::asprintf("[%s] [PID:%d]", stringToCStr(cfg.getProgramPath()), cfg.m_runningPid);
    }
    else
        statusText = QString::asprintf("[%s] [%s:%d]", stringToCStr(cfg.getProgramPath()), stringToCStr(cfg.m_tcpHost), (int)cfg.m_tcpPort);
    w.m_statusLineWidget.setText(statusText);
}


void MainWindow::onBreakpointsRemoveAll()
{
    Core &core = Core::getInstance();
    core.gdbRemoveAllBreakpoints();
}

void MainWindow::onBreakpointsRemoveSelected()
{
    // Get selected widget items
    QTreeWidget *bkptWidget = m_ui.treeWidget_breakpoints;
    QList<QTreeWidgetItem *> selectedItems = bkptWidget->selectedItems();

    // Get a list of breakpoints
    Core &core = Core::getInstance();
    QList<BreakPoint*>  bklist = core.getBreakPoints();
    QList <int>idList;
    QList<BreakPoint*>  toRemove;
    for(int u = 0;u < selectedItems.size();u++)
    {
        // Get the breakpoint
        QTreeWidgetItem *item = selectedItems[u];
        assert(item != NULL);
        int idx = item->data(0, Qt::UserRole).toInt();
        if(0 <= idx && idx < bklist.size())
        {
            BreakPoint* bkpt = bklist[idx];
            toRemove.append(bkpt);
        }
    }


    for(int u = 0;u < toRemove.size();u++)
    {
        BreakPoint* bkpt = toRemove[u];
        // Request that it is removed
        core.gdbRemoveBreakpoint(bkpt);
    }

}



void MainWindow::onBreakpointsGoTo()
{

    // Get selected widget items
    QTreeWidget *bkptWidget = m_ui.treeWidget_breakpoints;
    QList<QTreeWidgetItem *> selectedItems = bkptWidget->selectedItems();

    // Get a list of breakpoints
    Core &core = Core::getInstance();
    QList<BreakPoint*>  bklist = core.getBreakPoints();
    QList <int>idList;
    if(!selectedItems.empty())
    {
        // Get the breakpoint
        QTreeWidgetItem *item = selectedItems[0];
        int idx = item->data(0, Qt::UserRole).toInt();
        if(0 <= idx && idx < bklist.size())
        {
            BreakPoint* bk = bklist[idx];

            // Show the breakpoint
            CodeViewTab* currentCodeViewTab = open(bk->m_fullname);
            if(currentCodeViewTab)
                currentCodeViewTab->ensureLineIsVisible(bk->m_lineNo);
        }
    }
}

void MainWindow::onBreakpointsWidgetContextMenu(const QPoint& pos)
{
    QString title;
    QAction *action;

    // Get position
    QTreeWidget *bkptWidget = m_ui.treeWidget_breakpoints;
    QPoint popupPos = bkptWidget->mapToGlobal(pos);
    
    m_popupMenu.clear();

    // Add 'Go to'
    title = "Go to";
    action = m_popupMenu.addAction(title);
    connect(action, SIGNAL(triggered()), this, SLOT(onBreakpointsGoTo()));
    
    // Add 'Remove all breakpoints'
    title = "Remove all";
    action = m_popupMenu.addAction(title);
    connect(action, SIGNAL(triggered()), this, SLOT(onBreakpointsRemoveAll()));

    // Add 'Remove selected breakpoints'
    title = "Remove selected";
    action = m_popupMenu.addAction(title);
    connect(action, SIGNAL(triggered()), this, SLOT(onBreakpointsRemoveSelected()));
    
    m_popupMenu.popup(popupPos);
}

void MainWindow::hideSearchBox()
{
    m_ui.widget_search->hide();

    // Get active tab
    CodeViewTab* currentTab = (CodeViewTab* )m_ui.editorTabWidget->currentWidget();
    if(!currentTab)
        return;

    currentTab->clearIncSearch();
    
}

        
void MainWindow::onSearchCheckBoxStateChanged(int state)
{
    debugMsg("%s(state:%d)", __func__, state);

    if(state == Qt::Checked)
        m_ui.widget_search->show();
    else
    {
        hideSearchBox();
    }
}

void MainWindow::onSearchNext()
{
    // Get active tab
    CodeViewTab* currentTab = (CodeViewTab* )m_ui.editorTabWidget->currentWidget();
    if(!currentTab)
        return;

    int lineNo = currentTab->incSearchNext();
    if(lineNo > 0)
        currentTab->ensureLineIsVisible(lineNo);

    m_ui.lineEdit_search->setFocus();
}

void MainWindow::onSearchPrev()
{
    // Get active tab
    CodeViewTab* currentTab = (CodeViewTab* )m_ui.editorTabWidget->currentWidget();
    if(!currentTab)
        return;

    int lineNo = currentTab->incSearchPrev();
    if(lineNo > 0)
        currentTab->ensureLineIsVisible(lineNo);
    m_ui.lineEdit_search->setFocus();
}



void MainWindow::onFuncFilterClear()
{
    m_ui.lineEdit_funcFilter->setText("");
    
}


void MainWindow::onFuncFilter_textChanged(const QString &text)
{
    // Create the search list
    m_funcFilterText.clear();
    QStringList list = text.split(";");
    for(int i = 0;i < list.size();i++)
    {
        QString item = list[i].trimmed();
        if(!item.isEmpty())
        {
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
            QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(item));
#else
            QRegExp rx(item);
            rx.setPatternSyntax(QRegExp::Wildcard);
#endif
            m_funcFilterText.append(rx);
        }
    }
    
    fillInFuncList();
}


void MainWindow::onFuncFilterCheckBoxStateChanged(int state)
{
    if(state == Qt::Unchecked)
    {
        m_cfg.m_viewFuncFilter = false;
        showWidgets();
    }
}

void MainWindow::onClassFilterCheckBoxStateChanged(int state)
{
    if(state == Qt::Unchecked)
    {
        m_cfg.m_viewClassFilter = false;
        showWidgets();
    }
}


void MainWindow::onClassFilterClear()
{
    m_ui.lineEdit_classFilter->setText("");
    
}


void MainWindow::onClassFilter_textChanged(const QString &text)
{
    // Create the search list
    m_classFilterText.clear();
    QStringList list = text.split(";");
    for(int i = 0;i < list.size();i++)
    {
        QString item = list[i].trimmed();
        if(!item.isEmpty())
        {
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
            QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(item));
#else
            QRegExp rx(item);
            rx.setPatternSyntax(QRegExp::Wildcard);
#endif
            m_classFilterText.append(rx);
        }
    }
    
    fillInClassList();
}

void MainWindow::onIncSearch_textChanged(const QString &text)
{
    debugMsg("%s('%s')", __func__, qPrintable(text));

    // Get active tab
    CodeViewTab* currentTab = (CodeViewTab* )m_ui.editorTabWidget->currentWidget();
    if(!currentTab)
        return;

    int lineNo = currentTab->incSearchStart(text);
    if(lineNo > 0)
        currentTab->ensureLineIsVisible(lineNo);

}

/**
 * @brief Called when the tag manager is done with finding all the tags
 */
void MainWindow::onAllTagScansDone()
{
    // Get all tags
    m_tagList.clear();
    for(int k = 0;k < m_sourceFiles.size();k++)
    {
        FileInfo &info = m_sourceFiles[k];

        // Find the tag
        QList<Tag> thisTagList;
        m_tagManager.getTags(info.m_fullName, &thisTagList);
        m_tagList += thisTagList;
    }

    fillInClassList();
    fillInFuncList();
}


void MainWindow::fillInClassList()
{
    // Get all classes
    QStringList classList;
    for(int i = 0;i < m_tagList.size();i++)
    {
        const Tag &tag = m_tagList[i];
        QString className = tag.getClassName();
        if(!className.isEmpty())
            classList += tag.getClassName();
    }
    classList.removeDuplicates();
    classList.sort();
    
    // Insert the classes in the class widget
    QTreeWidget *classWidget = m_ui.treeWidget_classes;
    classWidget->clear();
    int totalClassFuncCount = 0;
    for(int ci = 0;ci < classList.size();ci++)
    {
        QString className = classList[ci];

        bool isMatch = true;
        
        // Does the filter match the name?
        for(int j = 0;j < m_classFilterText.size() && isMatch == true;j++)
        {
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
            QRegularExpression &rx = m_classFilterText[j];
            if(!rx.match(className).hasMatch())
                isMatch = false;
#else
            QRegExp &rx = m_classFilterText[j];
            if(rx.indexIn(className) == -1)
                isMatch = false;
#endif
        }

        if(isMatch)
        {
            // Insert the class
            QTreeWidgetItem *classItem = new QTreeWidgetItem;
            classItem->setText(0, className);
            QBrush blueBrush (Qt::blue);
            classItem->setForeground( 0, blueBrush);
            classWidget->addTopLevelItem(classItem);


            // Add all functions to the class in the class widget
            for(int i = 0;i < m_tagList.size();i++)
            {
                const Tag &tag = m_tagList[i];
                if(tag.isFunc() && tag.getClassName() == className)
                {
                    QTreeWidgetItem *item = new QTreeWidgetItem;
                    item->setText(0, tag.getName() + tag.getSignature());
                    item->setData(0, Qt::UserRole, tag.getLineNo());
                    item->setText(1, getFilenamePart(tag.getFilePath()));
                    item->setData(1, Qt::UserRole, tag.getFilePath());
                    item->setText(2, QString::number(tag.getLineNo()));
                    classItem->addChild(item);

                }
            }
            totalClassFuncCount += m_tagList.size();
        }
    
    }

    // Expand class treewidget?
    if(totalClassFuncCount < CLASS_LIST_AUTO_EXPAND_COUNT)
    {
        for(int j = classWidget->topLevelItemCount()-1;j >= 0;--j)
        {
            QTreeWidgetItem * classItem = classWidget->topLevelItem(j);
            classItem->setExpanded(true);
        }
    }
    
}

void MainWindow::fillInFuncList()
{
    QTreeWidget *funcWidget = m_ui.treeWidget_functions;
    funcWidget->clear();

    // Add all functions to the treewidget
    for(int i = 0;i < m_tagList.size();i++)
    {
        const Tag &tag = m_tagList[i];
        if(tag.isFunc())
        {
            bool isMatch = true;

            // Does the filter match the name?
            QString name = tag.getLongName();
            for(int j = 0;j < m_funcFilterText.size() && isMatch == true;j++)
            {
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
                QRegularExpression &rx = m_funcFilterText[j];
                if(!rx.match(name).hasMatch())
                    isMatch = false;
#else
                QRegExp &rx = m_funcFilterText[j];
                if(rx.indexIn(name) == -1)
                    isMatch = false;
#endif
            }

            // Add the item if it matches
            if(isMatch)
            {
                QTreeWidgetItem *item = new QTreeWidgetItem;
                if(tag.getClassName().isEmpty())
                    item->setText(0, " " + name);
                else
                    item->setText(0, name);
                item->setData(0, Qt::UserRole, tag.getLineNo());
                item->setText(1, getFilenamePart(tag.getFilePath()));
                item->setData(1, Qt::UserRole, tag.getFilePath());
                item->setText(2, QString::number(tag.getLineNo()));
                    
                funcWidget->addTopLevelItem(item);
            }
        }
    }
    funcWidget->sortItems(0, Qt::AscendingOrder);

}

void MainWindow::onFuncWidgetItemSelected(QTreeWidgetItem * item, int column)
{
    Q_UNUSED(column);

    // Get the linenumber and file where the function is defined in            
    QString filePath = item->data(1, Qt::UserRole).toString();
    int lineNo = item->data(0, Qt::UserRole).toInt();

    open(filePath, lineNo);
}


void MainWindow::onClassWidgetItemSelected(QTreeWidgetItem * item, int column)
{
    Q_UNUSED(column);

    debugMsg("%s(item:%p column:%d)", __func__, item, column);

    // Get the linenumber and file where the function is defined in            
    QString filePath = item->data(1, Qt::UserRole).toString();
    int lineNo = item->data(0, Qt::UserRole).toInt();

    open(filePath, lineNo);
}

