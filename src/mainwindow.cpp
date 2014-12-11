#include "mainwindow.h"
#include <QDirIterator>
#include "util.h"
#include "log.h"
#include "core.h"
#include <assert.h>
#include "aboutdialog.h"
#include "settingsdialog.h"
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
      : QMainWindow(parent)
{

    m_ui.setupUi(this);

    m_ui.codeView->setInterface(this);

    m_fileIcon.addFile(QString::fromUtf8(":/images/res/file.png"), QSize(), QIcon::Normal, QIcon::Off);
    m_folderIcon.addFile(QString::fromUtf8(":/images/res/folder.png"), QSize(), QIcon::Normal, QIcon::Off);

    //
    m_ui.varWidget->setColumnCount(3);
    m_ui.varWidget->setColumnWidth(0, 80);
    QStringList names;
    names += "Name";
    names += "Value";
    names += "Type";
    m_ui.varWidget->setHeaderLabels(names);
    connect(m_ui.varWidget, SIGNAL(itemChanged(QTreeWidgetItem * ,int)), this, SLOT(onVarWidgetCurrentItemChanged(QTreeWidgetItem * ,int)));


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
    m_ui.autoWidget->setColumnCount(2);
    m_ui.autoWidget->setColumnWidth(0, 80);
    names.clear();
    names += "Name";
    names += "Value";
    m_ui.autoWidget->setHeaderLabels(names);



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
    connect(m_ui.actionRun, SIGNAL(triggered()), SLOT(onRun()));
    connect(m_ui.actionContinue, SIGNAL(triggered()), SLOT(onContinue()));


    connect(m_ui.actionSettings, SIGNAL(triggered()), SLOT(onSettings()));


    fillInVars();

    open("mainwindow.h");


    Core &core = Core::getInstance();
    core.setListener(this);


    //
    QFont font = m_ui.logView->font();
    font.setPointSize(9);
    m_ui.logView->setFont(font);


    installEventFilter(this);

    loadConfig();
    
}

void MainWindow::loadConfig()
{
    m_ini.appendLoad(CONFIG_FILENAME);

    m_ui.codeView->setConfig(&m_ini);


    m_ini.save(CONFIG_FILENAME);

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
            QList<QTreeWidgetItem *> items = m_ui.varWidget->selectedItems();

            for(int i =0;i < items.size();i++)
            {
                QTreeWidgetItem *item = items[i];
            
                // Delete the item
                Core &core = Core::getInstance();
                QTreeWidgetItem *rootItem = m_ui.varWidget->invisibleRootItem();
                int key = item->data(0, Qt::UserRole).toInt();
                if(key > 0)
                {
                    rootItem->removeChild(item);
                    core.gdbRemoveVarWatch(key);
                }
            }
        }
        
        //qDebug() << "key " << keyEvent->key() << " from " << obj << "focus " << widget;

    }
    return QObject::eventFilter(obj, event);
}


/**
 * @brief Execution has stopped.
 * @param lineno   The line which is about to execute (1=first).
 */
void MainWindow::ICore_onStopped(ICore::StopReason reason, QString path, int lineno)
{
    m_currentFile = path;
    m_currentLine = lineno;
    if(!path.isEmpty())
    {
        open(path);
    }
    else
        m_ui.codeView->disableCurrentLine();

    fillInStack();
}


void MainWindow::ICore_onLocalVarReset()
{
    QTreeWidget *varWidget = m_ui.autoWidget;

    varWidget->clear();
}


/**
 * @brief Adds a path of directories to the tree widget.
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
    }

    if(restPath.isEmpty())
        return newItem;
    else
        return addTreeWidgetPath(treeWidget, newItem, restPath);
}


    
void MainWindow::insertSourceFiles()
{
    QTreeWidget *treeWidget = m_ui.treeWidget_file;
    Core &core = Core::getInstance();

    QVector <SourceFile*> sourceFiles = core.getSourceFiles();

    for(int i = 0;i < sourceFiles.size();i++)
    {
        SourceFile* source = sourceFiles[i];


        QTreeWidgetItem *item = new QTreeWidgetItem;
        QTreeWidgetItem *parentNode  = NULL;

        // Get parent path
        QString folderPath;
        QString filename;
        dividePath(source->name, &filename, &folderPath);

        if(!folderPath.isEmpty())
            parentNode = addTreeWidgetPath(treeWidget, NULL, folderPath);
            
        item->setText(0, filename);
        item->setData(0, Qt::UserRole, source->fullName);
        item->setIcon(0, m_fileIcon);
        
        if(parentNode == NULL)
            treeWidget->insertTopLevelItem(0, item);
        else
        {
            parentNode->addChild(item);
        }
    }


/*
  QTreeWidgetItem *root = new QTreeWidgetItem;
   root->setText(0, "/"); 
root->setIcon(0, m_folderIcon);
   treeWidget->insertTopLevelItem(0, root);

   QTreeWidgetItem *child = new QTreeWidgetItem;
   child->setText(0, "dir");
    root->addChild(child);
*/
    

  /*  
 

//
    QDirIterator it("./", QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QFileInfo info = it.fileInfo();
QString sf = info.suffix();

        if(sf == "cpp" || sf == "h")
        {
            QString filename = info.fileName();
QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(filename));

item->setData(0, Qt::UserRole, filename);


        root->addChild(item);

        }
        
        it.next();
    }
*/


}




void MainWindow::ICore_onLocalVarChanged(QString name, QString value)
{
    QTreeWidget *varWidget = m_ui.autoWidget;
    QTreeWidgetItem *item;
    QStringList names;
    
    
    names.clear();
    names += name;
    names += value;
    item = new QTreeWidgetItem(names);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    varWidget->insertTopLevelItem(0, item);
}



void MainWindow::ICore_onWatchVarChanged(int watchId, QString name, QString value)
{
    QTreeWidget *varWidget = m_ui.varWidget;
    QStringList names;

    Q_UNUSED(name);

    for(int i = 0;i < varWidget->topLevelItemCount();i++)
    {
        QTreeWidgetItem * item =  varWidget->topLevelItem(i);
        int itemKey = item->data(0, Qt::UserRole).toInt();
        //debugMsg("%s=%s", stringToCStr(name), stringToCStr(itemKey));
        if(watchId == itemKey)
        {
            // Update the variable value
            item->setText(1, value);
        }
    }

}

void MainWindow::ICodeView_onRowDoubleClick(int rowIdx)
{
    Core &core = Core::getInstance();
    int lineno = rowIdx+1;

    BreakPoint* bkpt = core.findBreakPoint(m_filename, lineno);
    if(bkpt)
        core.gdbRemoveBreakpoint(bkpt);
    else
        core.gdbSetBreakpoint(m_filename, lineno);
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

     
void MainWindow::fillInVars()
{
    QTreeWidget *varWidget = m_ui.varWidget;
    QTreeWidgetItem *item;
    QStringList names;
    
    varWidget->clear();



    names.clear();
    names += "...";
    item = new QTreeWidgetItem(names);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    varWidget->insertTopLevelItem(0, item);
    /*
    names.clear();
    names += "hej";
    names += "kalle";
    item = new QTreeWidgetItem(names);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    varWidget->insertTopLevelItem(0, item);
*/

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



void 
MainWindow::onVarWidgetCurrentItemChanged( QTreeWidgetItem * current, int column )
{
    QTreeWidget *varWidget = m_ui.varWidget;
    Core &core = Core::getInstance();
    int oldKey = current->data(0, Qt::UserRole).toInt();
    QString oldName  = oldKey == -1 ? "" : core.gdbGetVarWatchName(oldKey);
    QString newName = current->text(0);

    if(column != 0)
        return;

    if(oldKey != -1 && oldName == newName)
        return;
    
     debugMsg("oldName:'%s' newName:'%s' ", stringToCStr(oldName), stringToCStr(newName));

    if(newName == "...")
        newName = "";
    if(oldName == "...")
        oldName = "";
        
    // Nothing to do?
    if(oldName == "" && newName == "")
    {
        current->setText(0, "...");
        current->setText(1, "");
        current->setText(2, "");
    }
    // Remove a variable?
    else if(newName.isEmpty())
    {
        QTreeWidgetItem *item = varWidget->invisibleRootItem();
        item->removeChild(current);

        core.gdbRemoveVarWatch(oldKey);
        
    }
    // Add a new variable?
    else if(oldName == "")
    {
        // debugMsg("%s", stringToCStr(current->text(0)));
        QString value;
        int watchId;
        QString varType;
        if(core.gdbAddVarWatch(newName, &varType, &value, &watchId) == 0)
        {
            current->setData(0, Qt::UserRole, watchId);
            current->setText(1, value);
            current->setText(2, varType);
            
            // Create a new dummy item
            QTreeWidgetItem *item;
            QStringList names;
            names += "...";
            item = new QTreeWidgetItem(names);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            varWidget->addTopLevelItem(item);
        }
        else
        {
            current->setText(0, "...");
            current->setText(1, "");
            current->setText(2, "");
        }
    
    }
    // Change a existing variable
    else
    {
        //debugMsg("'%s' -> %s", stringToCStr(current->text(0)), stringToCStr(current->text(0)));

        // Remove old watch
        core.gdbRemoveVarWatch(oldKey);

        QString value;
        int watchId;
        QString varType;
        if(core.gdbAddVarWatch(newName, &varType, &value, &watchId) == 0)
        {
            current->setData(0, Qt::UserRole, watchId);
            current->setText(1, value);
            current->setText(2, varType);
        }
        else
        {
            QTreeWidgetItem *rootItem = varWidget->invisibleRootItem();
            rootItem->removeChild(current);
        }
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


void MainWindow::open(QString filename)
{
    //qDebug() << filename;

    setWindowTitle(getFilenamePart(filename));

    QString text;

    // Read file content
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
         return;
    while (!file.atEnd())
    {
         QByteArray line = file.readLine();
         text += line;
     }

    m_filename = filename;
    m_ui.codeView->setPlainText(text);

    // Update the current line view
    if(m_currentFile == filename)
    {
        m_ui.codeView->setCurrentLine(m_currentLine);

        // Scroll to the current line
        ensureLineIsVisible(m_currentLine);
    }
    else
        m_ui.codeView->disableCurrentLine();


    ICore_onBreakpointsChanged();
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
    Core &core = Core::getInstance();
    core.gdbContinue();

}

void MainWindow::onStepIn()
{
    Core &core = Core::getInstance();
    core.gdbStepIn();
    
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
    QVector<int> numList;


    // Update the breakpoint list widget
    m_ui.treeWidget_breakpoints->clear();
    for(int i = 0;i <  bklist.size();i++)
    {
        BreakPoint* bk = bklist[i];

        QStringList nameList;
        QString name;
        nameList.append(getFilenamePart(bk->fullname));
        nameList.append(bk->m_funcName);
        name.sprintf("%d", bk->lineno);
        nameList.append(name);
        nameList.append(longLongToHexString(bk->m_addr));
        
        

        QTreeWidgetItem *item = new QTreeWidgetItem(nameList);
        item->setData(0, Qt::UserRole, i);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        // Add the item to the widget
        m_ui.treeWidget_breakpoints->insertTopLevelItem(0, item);

    }

    // Update the fileview
    for(int i = 0;i <  bklist.size();i++)
    {
        BreakPoint* bk = bklist[i];

        if(bk->fullname == m_filename)
            numList.push_back(bk->lineno);
    }
    m_ui.codeView->setBreakpoints(numList);
    m_ui.codeView->update();
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

        // Update the sourceview (with the current row).
        if(idx+1 == stackFrameList.size())
        {
            m_currentFile = entry.m_sourcePath;
            m_currentLine = entry.m_line;
            if(!m_currentFile.isEmpty())
            {
                open(m_currentFile);
            }
            else
                m_ui.codeView->disableCurrentLine();
        }

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

    open(bk->fullname);
    
    ensureLineIsVisible(bk->lineno);
    
}
    

void MainWindow::ensureLineIsVisible(int lineIdx)
{
    m_ui.scrollArea_codeView->ensureVisible(0, m_ui.codeView->getRowHeight()*lineIdx-1);

}

void MainWindow::ICodeView_onContextMenu(QPoint pos, QStringList text)
{
    m_popupMenu.clear();
    for(int i = 0;i < text.size();i++)
    {
        QAction *action;
        action = m_popupMenu.addAction("Add '" + text[i] + "'");
        action->setData(text[i]);
        connect(action, SIGNAL(triggered()), this, SLOT(onCodeViewContextMenuItemPressed()));

    }
    
    m_popupMenu.popup(pos);
}

void MainWindow::onCodeViewContextMenuItemPressed()
{
    // Get the selected variable name
    QAction *action = static_cast<QAction *>(sender ());
    QString varName = action->data().toString();

    
    // Add the new variable to the watch list
    QTreeWidgetItem* rootItem = m_ui.varWidget->invisibleRootItem();
    QTreeWidgetItem* lastItem = rootItem->child(rootItem->childCount()-1);
    lastItem->setText(0, varName);
    
}

void MainWindow::onSettings()
{
    SettingsDialog dlg(this, &m_ini);
    if(dlg.exec() == QDialog::Accepted)
    {
        dlg.getConfig(&m_ini);

        m_ui.codeView->setConfig(&m_ini);

        m_ini.save(CONFIG_FILENAME);
    }
   
}

void MainWindow::ICore_onSignalReceived(QString signalName)
{
    if(signalName != "SIGINT")
    {
        //
        QMessageBox msgBox;
        QString msgText;
        msgText.sprintf("Program received signal %s", stringToCStr(signalName));
        msgBox.setText(msgText);
        msgBox.exec();
    }
    
    m_ui.codeView->disableCurrentLine();

    fillInStack();

}



    
