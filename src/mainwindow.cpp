#include "mainwindow.h"
#include "util.h"
#include "log.h"
#include "core.h"
#include <assert.h>
#include "aboutdialog.h"
#include "settingsdialog.h"
#include "tagscanner.h"
#include "codeview.h"


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
    m_ui.treeWidget_breakpoints->setColumnCount(2);
    m_ui.treeWidget_breakpoints->setColumnWidth(0, 80);
    names.clear();
    names += "Filename";
    names += "Func";
    names += "Line";
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


    //
    treeWidget = m_ui.treeWidget_threads;
    names.clear();
    names += "Name";
    treeWidget->setHeaderLabels(names);
    treeWidget->setColumnCount(1);
    treeWidget->setColumnWidth(0, 200);

    connect(m_ui.treeWidget_threads, SIGNAL(itemSelectionChanged()), this,
                SLOT(onThreadWidgetSelectionChanged()));

    //
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
    m_ui.splitter->setSizes(slist);

     
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


    connect(m_ui.actionSettings, SIGNAL(triggered()), SLOT(onSettings()));

    
    m_tagScanner.init();




    Core &core = Core::getInstance();
    core.setListener(this);


    //
    QFont font = m_ui.logView->font();
    font.setPointSize(9);
    m_ui.logView->setFont(font);


    installEventFilter(this);

    loadConfig();

    connect(m_ui.editorTabWidget, SIGNAL(tabCloseRequested(int)), SLOT(onCodeViewTab_tabCloseRequested(int)));
    connect(m_ui.editorTabWidget, SIGNAL(currentChanged(int)), SLOT(onCodeViewTab_currentChanged(int)));
    
    
}


void MainWindow::loadConfig()
{
    m_cfg.load(CONFIG_FILENAME);


    for(int tabIdx = 0;tabIdx <  m_ui.editorTabWidget->count();tabIdx++)
    {
        CodeViewTab* codeViewTab = (CodeViewTab* )m_ui.editorTabWidget->widget(tabIdx);
        codeViewTab->m_ui.codeView->setConfig(&m_cfg);
    }
    

    m_cfg.save(CONFIG_FILENAME);

}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        QWidget *widget = QApplication::focusWidget();

        // 'Delete' key pressed in the var widget 
        if(widget == m_ui.varWidget && keyEvent->key() == Qt::Key_Delete)
        {
            m_watchVarCtl.deleteSelected();
            
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

    if(reason == ICore::EXITED_NORMALLY)
    {
        QString title = "Program exited";
        QString text = "Program exited normally";
        QMessageBox::information (this, title, text); 
    }
    
    updateCurrentLine(path, lineNo);
    

    fillInStack();
}


void MainWindow::ICore_onLocalVarReset()
{
    QTreeWidget *autoWidget = m_ui.autoWidget;

    autoWidget->clear();
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
    QTreeWidgetItem * lookParent;
    if(parent == NULL)
        lookParent = treeWidget->invisibleRootItem();
    else
        lookParent = parent;
    for(int i = 0;i < lookParent->childCount() && newItem == NULL;i++)
    {
        QTreeWidgetItem *item = lookParent->child(i);


        if(item->text(0) == firstName)
            newItem = item;
    }
    

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

    // Get source files
    QVector <SourceFile*> sourceFiles = core.getSourceFiles();
    m_sourceFiles.clear();
    for(int i = 0;i < sourceFiles.size();i++)
    {
        SourceFile* source = sourceFiles[i];

        FileInfo info;
        info.name = source->name;
        info.fullName = source->fullName;
        
        m_sourceFiles.push_back(info);
    }

    

    for(int i = 0;i < m_sourceFiles.size();i++)
    {
        FileInfo &info = m_sourceFiles[i];

        m_tagScanner.scan(info.fullName, &info.m_tagList);
        

        QTreeWidgetItem *item = new QTreeWidgetItem;
        QTreeWidgetItem *parentNode  = NULL;

        // Get parent path
        QString folderPath;
        QString filename;
        dividePath(info.fullName, &filename, &folderPath);
        folderPath = simplifyPath(folderPath);
        
        if(!folderPath.isEmpty())
            parentNode = addTreeWidgetPath(treeWidget, NULL, folderPath);
            
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

    wrapSourceTree(treeWidget);

    treeWidget->sortItems(0, Qt::AscendingOrder);

}






    
    
void MainWindow::ICore_onLocalVarChanged(QString name, CoreVarValue varValue)
{
    m_autoVarCtl.ICore_onLocalVarChanged(name, varValue);
}



void MainWindow::ICore_onWatchVarChanged(QString watchId, QString name, QString valueString, bool hasChildren)
{
    ICore_onWatchVarChildAdded( watchId,  name,  valueString, "", hasChildren);
    
}


            
void MainWindow::ICore_onWatchVarChildAdded(QString watchId, QString name, QString valueString, QString varType, bool hasChildren)
{
    m_watchVarCtl.ICore_onWatchVarChildAdded(watchId, name, valueString, varType, hasChildren);
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
        
    BreakPoint* bkpt = core.findBreakPoint(currentCodeViewTab->m_filepath, lineNo);
    if(bkpt)
        core.gdbRemoveBreakpoint(bkpt);
    else
        core.gdbSetBreakpoint(currentCodeViewTab->m_filepath, lineNo);
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
    QTreeWidget *threadWidget = m_ui.treeWidget_stack;
    QList <QTreeWidgetItem *> selectedItems = threadWidget->selectedItems();
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


CodeViewTab* MainWindow::createTab(QString filepath)
{
    CodeViewTab *newTab = new CodeViewTab(this);
    connect(newTab->m_ui.comboBox_funcList, SIGNAL(activated(int)), SLOT(onFuncListItemActivated(int)));

    newTab->m_filepath = filepath;
    newTab->m_ui.codeView->setInterface(this);

    newTab->m_ui.codeView->setConfig(&m_cfg);
    
    m_ui.editorTabWidget->addTab(newTab, getFilenamePart(filepath));

    m_ui.editorTabWidget->setCurrentIndex(m_ui.editorTabWidget->count()-1);
            

    return newTab;
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
        if(codeViewTab->m_filepath == filename)
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
    QString text;

    codeViewTab = createTab(filename);
    
    // Read file content
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errorMsg("Failed to open '%s'", stringToCStr(filename));
        return NULL;
    }
    while (!file.atEnd())
    {
         QByteArray line = file.readLine();
         text += line;
    }

    codeViewTab->m_ui.scrollArea_codeView->setWidgetResizable(true);

    // Set window title
    QString windowTitle;
    QString filenamePart, folderPathPart;
    dividePath(filename, &filenamePart, &folderPathPart);
    windowTitle.sprintf("%s - %s",  stringToCStr(filenamePart), stringToCStr(folderPathPart));
    setWindowTitle(windowTitle);


    codeViewTab->m_ui.codeView->setPlainText(text);

    // Fill in the functions
    codeViewTab->m_ui.comboBox_funcList->clear();
    QList<Tag> tagList;
    m_tagScanner.scan(filename, &tagList);
    for(int tagIdx = 0;tagIdx < tagList.size();tagIdx++)
    {
        Tag &tag = tagList[tagIdx];
        if(tag.type == Tag::TAG_FUNC)
        {
            codeViewTab->m_ui.comboBox_funcList->addItem(tag.getLongName(), QVariant(tag.getLineNo()));
        }
        
    }

    }


    ICore_onBreakpointsChanged();

    return codeViewTab;
}


void MainWindow::updateCurrentLine(QString filename, int lineno)
{
    m_currentFile = filename;
    m_currentLine = lineno;

    if(!filename.isEmpty())
    {
        open(filename);
    }

    
    // Update the current line view
    if(!filename.isEmpty())
    {
        for(int tabIdx = 0;tabIdx <  m_ui.editorTabWidget->count();tabIdx++)
        {
            CodeViewTab* codeViewTab = (CodeViewTab* )m_ui.editorTabWidget->widget(tabIdx);
            if(codeViewTab->m_filepath == m_currentFile)
                codeViewTab->m_ui.codeView->setCurrentLine(m_currentLine);
        }
        
        // Scroll to the current line
        CodeViewTab* currentCodeViewTab = currentTab();
        if(currentCodeViewTab)
        {
            if(currentCodeViewTab->m_filepath == m_currentFile)
                currentCodeViewTab->ensureLineIsVisible(m_currentLine);
        }
            
    }
    else
    {
        for(int tabIdx = 0;tabIdx <  m_ui.editorTabWidget->count();tabIdx++)
        {
            CodeViewTab* codeViewTab = (CodeViewTab* )m_ui.editorTabWidget->widget(tabIdx);
        
            codeViewTab->m_ui.codeView->disableCurrentLine();
        }
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
    AboutDialog dlg(this);
    dlg.exec();
}


void MainWindow::onRun()
{
    Core &core = Core::getInstance();
    core.gdbRun();

}

void MainWindow::onContinue()
{
    CodeViewTab* codeViewTab = currentTab();
    Core &core = Core::getInstance();
    core.gdbContinue();

    if(codeViewTab)
        codeViewTab->m_ui.codeView->disableCurrentLine();

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
        

        // Add the item
        QStringList names;
        names.push_back(name);
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
    for(int i = 0;i < rootItem->childCount();i++)
    {
        QTreeWidgetItem *item = rootItem->child(i);
    
        int id = item->data(0, Qt::UserRole).toInt();
        if(id == threadId)
        {
            item->setSelected(true);    
        }
        else
            item->setSelected(false);
    }
    
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
    m_cfg.save(CONFIG_FILENAME);
    

    // Update the breakpoint list widget
    m_ui.treeWidget_breakpoints->clear();
    for(int i = 0;i <  bklist.size();i++)
    {
        BreakPoint* bk = bklist[i];

        QStringList nameList;
        QString name;
        nameList.append(getFilenamePart(bk->fullname));
        nameList.append(bk->m_funcName);
        name.sprintf("%d", bk->lineNo);
        nameList.append(name);
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

            if(bk->fullname == codeViewTab->m_filepath)
                numList.push_back(bk->lineNo);
        }

        codeViewTab->m_ui.codeView->setBreakpoints(numList);
        codeViewTab->m_ui.codeView->update();
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
    QTreeWidget *threadWidget = m_ui.treeWidget_stack;
    QTreeWidgetItem *rootItem = threadWidget->invisibleRootItem();

    // Update the sourceview (with the current row).
    if(frameIdx < m_stackFrameList.size())
    {
        StackFrameEntry &entry = m_stackFrameList[m_stackFrameList.size()-frameIdx-1];

        QString currentFile = entry.m_sourcePath;
        updateCurrentLine(currentFile, entry.m_line);
    }

    // Update the selection of the current thread
    for(int i = 0;i < rootItem->childCount();i++)
    {
        QTreeWidgetItem *item = rootItem->child(i);
    
        int id = item->data(0, Qt::UserRole).toInt();
        if(id == frameIdx)
        {
            item->setSelected(true);    
        }
        else
            item->setSelected(false);
    }
    
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

            // Loop through all the tags
            for(int j = 0;j < fileInfo.m_tagList.size();j++)
            {
                Tag &tagInfo = fileInfo.m_tagList[j];
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
    if(!onlyFuncs)
    {
        for(int i = 0;i < text.size();i++)
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
        
    BreakPoint* bkpt = core.findBreakPoint(currentCodeViewTab->m_filepath, lineNo);
    if(bkpt)
        core.gdbRemoveBreakpoint(bkpt);
    else
        core.gdbSetBreakpoint(currentCodeViewTab->m_filepath, lineNo);

}


void MainWindow::onCodeViewContextMenuShowCurrentLocation()
{
    // Open file
    CodeViewTab* currentCodeViewTab = open(m_currentFile);

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
    

    // Select the function in the function combobox
    if(codeViewTab)
    {
    int idx = codeViewTab->m_ui.comboBox_funcList->findData(QVariant(lineNo));
    if(idx >= 0)
        codeViewTab->m_ui.comboBox_funcList->setCurrentIndex(idx);
    }
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
    dividePath(currentCodeViewTab->m_filepath, NULL, &folderPath);
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

void MainWindow::onSettings()
{
    
    SettingsDialog dlg(this, &m_cfg);
    if(dlg.exec() == QDialog::Accepted)
    {
        dlg.getConfig(&m_cfg);

        for(int tabIdx = 0;tabIdx <  m_ui.editorTabWidget->count();tabIdx++)
        {
            CodeViewTab* codeViewTab = (CodeViewTab* )m_ui.editorTabWidget->widget(tabIdx);
            codeViewTab->m_ui.codeView->setConfig(&m_cfg);
        }
        
        m_cfg.save(CONFIG_FILENAME);
    }
   
}

void MainWindow::ICore_onSignalReceived(QString signalName)
{
    CodeViewTab* codeViewTab = currentTab();

    if(signalName != "SIGINT")
    {
        //
        QString msgText;
        msgText.sprintf("Program received signal %s.", stringToCStr(signalName));
        QString title = "Signal received";
        QMessageBox::warning(this, title, msgText);
    }
    
    if(codeViewTab)
        codeViewTab->m_ui.codeView->disableCurrentLine();

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
    m_ui.actionNext->setEnabled(state == TARGET_STOPPED ? true : false);
    m_ui.actionStep_In->setEnabled(state == TARGET_STOPPED ? true : false);
    m_ui.actionStep_Out->setEnabled(state == TARGET_STOPPED ? true : false);
    m_ui.actionStop->setEnabled(state == TARGET_STOPPED ? false : true);
    m_ui.actionContinue->setEnabled(state == TARGET_STOPPED ? true : false);
    m_ui.actionRun->setEnabled(state == TARGET_STOPPED ? true : false);
    
    if(state == TARGET_RUNNING)
    {
        m_ui.treeWidget_stack->clear();
        m_ui.autoWidget->clear();
    }
}


void MainWindow::onFuncListItemActivated(int index)
{
    CodeViewTab* codeViewTab = currentTab();
    assert(codeViewTab != NULL);
    if(!codeViewTab)
        return;
        
    int funcLineNo = codeViewTab->m_ui.comboBox_funcList->itemData(index).toInt();
    int lineIdx = funcLineNo-2;
    if(lineIdx < 0)
        lineIdx = 0;
    codeViewTab->m_ui.scrollArea_codeView->verticalScrollBar()->setValue(codeViewTab->m_ui.codeView->getRowHeight()*lineIdx);
}


    
