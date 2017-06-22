//#define ENABLE_DEBUGMSG

/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "mainwindow.h"
#include "util.h"
#include "log.h"
#include "core.h"
#include <assert.h>
#include "aboutdialog.h"
#include "settingsdialog.h"
#include "tagscanner.h"
#include "codeview.h"

#include "memorydialog.h"
#include <QDirIterator>
#include <QMessageBox>
#include <QScrollBar>


MainWindow::MainWindow(QWidget *parent)
      : QMainWindow(parent)
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






    //
    QTreeWidget *treeWidget = m_ui.treeWidget_file;
    names.clear();
    names += "Name";
    treeWidget->setHeaderLabels(names);
    treeWidget->setColumnCount(1);
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
    connect(m_ui.actionStep_In, SIGNAL(triggered()), SLOT(onStepIn()));
    connect(m_ui.actionStep_Out, SIGNAL(triggered()), SLOT(onStepOut()));
    connect(m_ui.actionRun, SIGNAL(triggered()), SLOT(onRun()));
    connect(m_ui.actionContinue, SIGNAL(triggered()), SLOT(onContinue()));

    connect(m_ui.actionViewStack, SIGNAL(triggered()), SLOT(onViewStack()));
    connect(m_ui.actionViewBreakpoints, SIGNAL(triggered()), SLOT(onViewBreakpoints()));
    connect(m_ui.actionViewThreads, SIGNAL(triggered()), SLOT(onViewThreads()));
    connect(m_ui.actionViewWatch, SIGNAL(triggered()), SLOT(onViewWatch()));
    connect(m_ui.actionViewAutoVariables, SIGNAL(triggered()), SLOT(onViewAutoVariables()));
    connect(m_ui.actionViewTargetOutput, SIGNAL(triggered()), SLOT(onViewTargetOutput()));
    connect(m_ui.actionViewGdbOutput, SIGNAL(triggered()), SLOT(onViewGdbOutput()));
    connect(m_ui.actionViewFileBrowser, SIGNAL(triggered()), SLOT(onViewFileBrowser()));

    connect(m_ui.actionDefaultViewSetup, SIGNAL(triggered()), SLOT(onDefaultViewSetup()));

    connect(m_ui.actionSettings, SIGNAL(triggered()), SLOT(onSettings()));

    

    Core &core = Core::getInstance();
    core.setListener(this);


    

    installEventFilter(this);

    loadConfig();

    connect(m_ui.editorTabWidget, SIGNAL(tabCloseRequested(int)), SLOT(onCodeViewTab_tabCloseRequested(int)));
    connect(m_ui.editorTabWidget, SIGNAL(currentChanged(int)), SLOT(onCodeViewTab_currentChanged(int)));
    
    statusBar()->addPermanentWidget(&m_statusLineWidget);


    m_gui_default_mainwindowState = saveState();
    m_gui_default_mainwindowGeometry = saveGeometry();
    m_gui_default_splitter1State = m_ui.splitter_1->saveState();
    m_gui_default_splitter2State = m_ui.splitter_2->saveState();
    m_gui_default_splitter3State = m_ui.splitter_3->saveState();
    m_gui_default_splitter4State = m_ui.splitter_4->saveState();

}



MainWindow::~MainWindow()
{
 
}


void MainWindow::showWidgets()
{

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
    

    
    
    m_ui.varWidget->setVisible(m_cfg.m_viewWindowWatch);
    m_ui.autoWidget->setVisible(m_cfg.m_viewWindowAutoVariables);
    m_ui.treeWidget_file->setVisible(m_cfg.m_viewWindowFileBrowser);


    currentSelection = m_ui.tabWidget_2->currentWidget();
    m_ui.tabWidget_2->clear();
    if(m_cfg.m_viewWindowTargetOutput)
        m_ui.tabWidget_2->insertTab(0, m_ui.targetOutputView, "Target Output");
    if(m_cfg.m_viewWindowGdbOutput)
        m_ui.tabWidget_2->insertTab(0, m_ui.logView, "GDB Output");
    selectionIdx = m_ui.tabWidget_2->indexOf(currentSelection);
    if(selectionIdx != -1)
        m_ui.tabWidget_2->setCurrentIndex(selectionIdx);
    m_ui.tabWidget_2->setVisible(m_ui.tabWidget_2->count() == 0 ? false : true);


}



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
    m_ui.actionViewGdbOutput->setChecked(m_cfg.m_viewWindowGdbOutput);
    m_ui.actionViewFileBrowser->setChecked(m_cfg.m_viewWindowFileBrowser);

    showWidgets();

    showEvent(NULL);
    
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
    m_ui.actionViewGdbOutput->setChecked(m_cfg.m_viewWindowGdbOutput);
    m_ui.actionViewFileBrowser->setChecked(m_cfg.m_viewWindowFileBrowser);

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

        // 'Delete' key pressed in the var widget 
        if(widget == m_ui.varWidget)
        {
            m_watchVarCtl.onKeyPress(keyEvent);
            
        }
        
        //qDebug() << "key " << keyEvent->key() << " from " << obj << "focus " << widget;

    }
    return QObject::eventFilter(obj, event);
}


/**
 * @brief Execution has stopped.
 * @param lineNo   The line which is about to execute (1=first).
 */
void MainWindow::ICore_onStopped(ICore::StopReason reason, QString path, int lineNo)
{
    Q_UNUSED(reason);


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
                if(source->fullName.startsWith(ignoreDir))
                    ignore = true;
            }
        }

        if(!ignore)
        {
            FileInfo info;
            info.name = source->name;
            info.fullName = source->fullName;
            
            m_sourceFiles.push_back(info);
        }
    }

    

    for(int i = 0;i < m_sourceFiles.size();i++)
    {
        FileInfo &info = m_sourceFiles[i];

        m_tagManager.queueScan(info.fullName);
        

        QTreeWidgetItem *parentNode  = NULL;

        // Get parent path
        QString folderPath;
        QString filename;
        dividePath(info.fullName, &filename, &folderPath);
        folderPath = simplifyPath(folderPath);
        
        if(!folderPath.isEmpty())
            parentNode = addTreeWidgetPath(treeWidget, NULL, folderPath);
            

        // Check if the item already exist?
        QTreeWidgetItem *item = findTreeWidgetChildByName(treeWidget, parentNode, filename);
        
        if(item == NULL)
        {
            item = new QTreeWidgetItem;
            item->setText(0, filename);
            item->setData(0, Qt::UserRole, info.fullName);
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


CodeViewTab* MainWindow::open(QString filename)
{
    if(filename.isEmpty())
        return NULL;

    // Already open
    int foundCodeViewTabIdx = -1;
    for(int tabIdx = 0;tabIdx <  m_ui.editorTabWidget->count();tabIdx++)
    {
        CodeViewTab* codeViewTab = (CodeViewTab* )m_ui.editorTabWidget->widget(tabIdx);
        if(codeViewTab->getFilePath() == filename)
        {
            foundCodeViewTabIdx = tabIdx;
            
        }
    }

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
        
    
        // Create the tab
        codeViewTab = new CodeViewTab(this);
        codeViewTab->setInterface(this);
        codeViewTab->setConfig(&m_cfg);

        if(codeViewTab->open(filename,tagList))
            return NULL;

        // Add the new codeview tab
        m_ui.editorTabWidget->addTab(codeViewTab, getFilenamePart(filename));
        m_ui.editorTabWidget->setCurrentIndex(m_ui.editorTabWidget->count()-1);
    }
    
    // Set window title
    QString windowTitle;
    QString filenamePart, folderPathPart;
    dividePath(filename, &filenamePart, &folderPathPart);
    windowTitle.sprintf("%s - %s",  stringToCStr(filenamePart), stringToCStr(folderPathPart));
    setWindowTitle(windowTitle);


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

    if(!filename.isEmpty())
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


void MainWindow::onRun()
{
    Core &core = Core::getInstance();
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
        item->setData(0, Qt::UserRole, list[idx].id);
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
        bkptCfg.filename = bkpt->fullname;
        bkptCfg.lineNo = bkpt->lineNo;
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
        nameList.append(getFilenamePart(bk->fullname));
        name.sprintf("%d", bk->lineNo);
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

            if(bk->fullname == codeViewTab->getFilePath())
                numList.push_back(bk->lineNo);
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

    CodeViewTab* currentCodeViewTab = open(bk->fullname);
    if(currentCodeViewTab)
        currentCodeViewTab->ensureLineIsVisible(bk->lineNo);
    
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

    // Add 'open'
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
            m_tagManager.getTags(fileInfo.fullName, &tagList);

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
                        defList.push_back(fileInfo.fullName);
                        QString lineNoStr;
                        lineNoStr.sprintf("%d", tagInfo.getLineNo());
                        defList.push_back(lineNoStr);

                        if(tagInfo.type != Tag::TAG_FUNC)
                            onlyFuncs = false;
                            
                        // Add to popupmenu
                        QString menuEntryText;
                        menuEntryText.sprintf("Show definition of '%s' L%d", stringToCStr(tagInfo.getLongName()), tagInfo.getLineNo());
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
    title.sprintf("Toggle breakpoint at L%d", lineNo);
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

    
    m_popupMenu.popup(pos);
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
            if(info.fullName.endsWith("/" + filenameWop))
                foundFilename = info.fullName;
        }

        // otherwise look in all the dirs
        if(foundFilename == "")
        {
            // Get a list of all dirs to look in
            QStringList dirs;
            for(int j = 0;foundFilename == "" && j < m_sourceFiles.size();j++)
            {
                FileInfo &info = m_sourceFiles[j];
                dividePath(info.fullName, NULL, &folderPath);
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
        errorMsg("Unable to find '%s'", stringToCStr(filename));
        
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
    for(int tabIdx = 0;tabIdx <  m_ui.editorTabWidget->count();tabIdx++)
    {
        CodeViewTab* codeViewTab = (CodeViewTab* )m_ui.editorTabWidget->widget(tabIdx);
        codeViewTab->setConfig(&m_cfg);
    }
    
    m_gdbOutputFont = QFont(m_cfg.m_gdbOutputFontFamily, m_cfg.m_gdbOutputFontSize);
    m_ui.logView->setFont(m_gdbOutputFont);

    m_outputFont = QFont(m_cfg.m_outputFontFamily, m_cfg.m_outputFontSize);
    m_ui.targetOutputView->setFont(m_outputFont);
    
    m_autoVarCtl.setConfig(&m_cfg);

}


void MainWindow::onSettings()
{
    
    SettingsDialog dlg(this, &m_cfg);
    if(dlg.exec() == QDialog::Accepted)
    {
        dlg.getConfig(&m_cfg);

        setConfig();
        
        
        m_cfg.save();
    }
   
}

void MainWindow::ICore_onSignalReceived(QString signalName)
{
    if(signalName != "SIGINT")
    {
        //
        QString msgText;
        msgText.sprintf("Program received signal %s.", stringToCStr(signalName));
        QString title = "Signal received";
        QMessageBox::warning(this, title, msgText);
    }
    
    onCurrentLineDisabled();
        
    fillInStack();

}

void MainWindow::ICore_onTargetOutput(QString message)
{
    if(message.endsWith("\n"))
    {
        message = message.left(message.length()-1);
    }
    if(message.endsWith("\r"))
    {
        message = message.left(message.length()-1);
    }
    m_ui.targetOutputView->appendPlainText(message);    
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
    m_ui.actionRun->setEnabled(!isRunning);

    m_ui.varWidget->setEnabled(isStopped);

    
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
        statusText.sprintf("[%s] [%s]", stringToCStr(cfg.m_lastProgram), stringToCStr(argumentText));
    }
    else
        statusText.sprintf("[%s] [%s:%d]", stringToCStr(cfg.m_tcpProgram), stringToCStr(cfg.m_tcpHost), (int)cfg.m_tcpPort);
    w.m_statusLineWidget.setText(statusText);
}



    
