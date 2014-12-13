#include "core.h"

#include <QByteArray>
#include <QDebug>
#include <unistd.h>
#include "util.h"
#include "log.h"
#include <assert.h>
#include <signal.h>



Core::Core()
 : m_inf(NULL)
    ,m_selectedThreadId(0)
    ,m_targetState(TARGET_STOPPED)
    ,m_pid(0)
    ,m_currentFrameIdx(-1)
    ,m_varWatchLastId(10)
{
    
    Com& com = Com::getInstance();
    com.setListener(this);

}

Core::~Core()
{
    
}


int Core::init(QStringList argumentList)
{
    Com& com = Com::getInstance();
    Tree resultData;
    
    com.init();
    com.commandF(&resultData, "-file-exec-and-symbols %s", stringToCStr(argumentList[0]));


    QString commandStr;
    if(argumentList.size() > 1)
    {
        commandStr = "-exec-arguments ";
        for(int i = 1;i < argumentList.size();i++)
            commandStr += argumentList[i];
        com.command(NULL, commandStr);
    }
    
    gdbInsertBreakPoint("main");
    gdbRun();

    //gdbNext();

    gdbGetFiles();

    //com.command("run");
    
    return 0;
}


void Core::gdbGetFiles()
{
    Com& com = Com::getInstance();
    Tree resultData;

    com.command(&resultData, "-file-list-exec-source-files");


    for(int i = 0;i < m_sourceFiles.size();i++)
    {
        SourceFile *sourceFile = m_sourceFiles[i];
        delete sourceFile;
    }
    m_sourceFiles.clear();


    for(int i = 0;i < resultData.getRootChildCount();i++)
    {
        TreeNode *rootNode = resultData.getChildAt(i);
        QString rootName = rootNode->getName();

        if(rootName == "files")
        {
            QStringList childList = resultData.getChildList("files");
            for(int i = 0;i < childList.size();i++)
            {
                QString treePath = "files/" + childList[i];
                QString name = resultData.getString(treePath + "/file");
                QString fullname = resultData.getString(treePath + "/fullname");

                SourceFile *sourceFile = new SourceFile; 

                sourceFile->name =name;
                sourceFile->fullName = fullname;

                m_sourceFiles.append(sourceFile);
            }
        }
    }
}


void Core::gdbInsertBreakPoint(QString func)
{
    Com& com = Com::getInstance();
    Tree resultData;

    com.commandF(&resultData, "-break-insert %s", stringToCStr(func));
}

void Core::gdbRun()
{
    Com& com = Com::getInstance();
    Tree resultData;

    if(m_targetState == TARGET_RUNNING)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is currently running");
        return;
    }

    com.commandF(&resultData, "-exec-run");

}


/**
 * @brief  Resumes the execution until a breakpoint is encountered, or until the program exits.
 */
void Core::gdbContinue()
{
    Com& com = Com::getInstance();
    Tree resultData;

    if(m_targetState == TARGET_RUNNING)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is currently running");
        return;
    }

    com.commandF(&resultData, "-exec-continue");

}

void Core::stop()
{

    if(m_targetState != TARGET_RUNNING)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is not running");
        return;
    }
    kill(m_pid, SIGINT);
}

void Core::gdbNext()
{
    Com& com = Com::getInstance();
    Tree resultData;

    if(m_targetState != TARGET_STOPPED)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is not stopped");
        return;
    }

    com.commandF(&resultData, "-exec-next");

}


void Core::getStackFrames()
{
    Com& com = Com::getInstance();
    Tree resultData;
    com.command(&resultData, "-stack-list-frames");

}



void Core::gdbStepIn()
{
    Com& com = Com::getInstance();
    Tree resultData;

    if(m_targetState != TARGET_STOPPED)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is not stopped");
        return;
    }

        
    com.commandF(&resultData, "-exec-step");
    com.commandF(&resultData, "-var-update --all-values *");

}
    
Core& Core::getInstance()
{
    static Core core;
    return core;
}


int Core::gdbAddVarWatch(QString varName, QString *varType, QString *value, int *watchId_)
{
    Com& com = Com::getInstance();
    Tree resultData;
    int watchId = m_varWatchLastId++;
    GdbResult res;
    int rc = 0;
    
    assert(varName.isEmpty() == false);
    
    res = com.commandF(&resultData, "-var-create w%d * %s", watchId, stringToCStr(varName));
    if(res == ERROR)
    {
        rc = -1;
    }
    else
    {

    //
    QString varName2 = resultData.getString("name");
    QString varValue2 = resultData.getString("value");
    QString varType2 = resultData.getString("type");


    VarWatch w;
    w.name = varName;
    w.id = watchId;
    m_watchList[watchId] = w;
    
    *varType = varType2;
    *value = varValue2;
    }

    *watchId_ = watchId;

    return rc;
}


QString Core::gdbGetVarWatchName(int watchId)
{
    return m_watchList[watchId].name;
}

void Core::gdbRemoveVarWatch(int watchId)
{
    Com& com = Com::getInstance();
    Tree resultData;
    
    assert(watchId != -1);
    
    
    QMap <int, VarWatch>::iterator pos = m_watchList.find(watchId);
    if(pos == m_watchList.end())
    {
        assert(0);
    }
    else
    {
        m_watchList.erase(pos);
    
        com.commandF(&resultData, "-var-delete w%d", watchId);
    }
}


void Core::onNotifyAsyncOut(Tree &tree, AsyncClass ac)
{
    debugMsg("NotifyAsyncOut> %s", Com::asyncClassToString(ac));

     if(ac == ComListener::AC_BREAKPOINT_MODIFIED)
    {

        for(int i = 0;i < tree.getRootChildCount();i++)
        {
            TreeNode *rootNode = tree.getChildAt(i);
            QString rootName = rootNode->getName();

            if(rootName == "bkpt")
            {
                dispatchBreakpointTree(tree);
            }
        }
    }
    // A new thread has been created
    else if(ac == ComListener::AC_THREAD_CREATED)
    {
        gdbGetThreadList();
        
    }
    
    tree.dump();
}

ICore::StopReason Core::parseReasonString(QString reasonString)
{
    if( reasonString == "breakpoint-hit")
        return ICore::BREAKPOINT_HIT;
    if(reasonString == "end-stepping-range")
        return ICore::END_STEPPING_RANGE;
    if(reasonString == "signal-received" || reasonString == "exited-signalled")
        return ICore::SIGNAL_RECEIVED;
    if(reasonString == "exited-normally")
        return ICore::EXITED_NORMALLY;
    if(reasonString == "exited")
        return ICore::EXITED;
    
    errorMsg("Received unknown reason (\"%s\").", stringToCStr(reasonString));
    assert(0);

    return ICore::UNKNOWN;
}
    
void Core::onExecAsyncOut(Tree &tree, AsyncClass ac)
{
    Com& com = Com::getInstance();

    debugMsg("ExecAsyncOut> %s", Com::asyncClassToString(ac));
    
    tree.dump();

    // The program has stopped
    if(ac == ComListener::AC_STOPPED)
    {
        m_targetState = TARGET_STOPPED;

        if(m_pid == 0)
            com.command(NULL, "-list-thread-groups");
         

        com.commandF(NULL, "-var-update --all-values *");
        com.commandF(NULL, "-stack-list-locals 1");


        QString p = tree.getString("frame/fullname");
        int lineno = tree.getInt("frame/line");

        // Get the reason
        ICore::StopReason  reason = parseReasonString(tree.getString("reason"));

        if(reason == ICore::EXITED_NORMALLY)
            m_targetState = TARGET_FINISHED;
        
        if(m_inf)
        {
            if(reason == ICore::SIGNAL_RECEIVED)
            {
                QString signalName = tree.getString("signal-name");
                if(signalName == "SIGSEGV")
                {
                    m_targetState = TARGET_FINISHED;
                }
                m_inf->ICore_onSignalReceived(signalName);  
            }
            else
                m_inf->ICore_onStopped(reason, p, lineno);

            m_inf->ICore_onFrameVarReset();

            QStringList childList = tree.getChildList("frame/args");
            for(int i = 0;i < childList.size();i++)
            {
                QString treePath = "frame/args/" + childList[i];
                QString varName = tree.getString(treePath + "/name");
                QString varValue = tree.getString(treePath + "/value");
                if(m_inf)
                    m_inf->ICore_onFrameVarChanged(varName, varValue);
            }

            int frameIdx = tree.getInt("frame/level");
            m_currentFrameIdx = frameIdx;
            m_inf->ICore_onCurrentFrameChanged(frameIdx);

        }
    }
    else if(ac == ComListener::AC_RUNNING)
    {
        m_targetState = TARGET_RUNNING;

        debugMsg("is running\n");
    }

    // Get the current thread
    QString threadIdStr = tree.getString("thread-id");
    if(threadIdStr.isEmpty() == false)
    {
        int threadId = threadIdStr.toInt();
        if(m_inf)
            m_inf->ICore_onCurrentThreadChanged(threadId);
    }

}


void Core::gdbRemoveBreakpoint(BreakPoint* bkpt)
{
    Com& com = Com::getInstance();
    Tree resultData;
    
    assert(bkpt != NULL);
    
    com.commandF(&resultData, "-break-delete %d", bkpt->m_number);    

    m_breakpoints.removeOne(bkpt);

    if(m_inf)
        m_inf->ICore_onBreakpointsChanged();
    delete bkpt;
    
}

void Core::gdbGetThreadList()
{
    Com& com = Com::getInstance();
    Tree resultData;

    if(m_targetState == TARGET_RUNNING)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is currently running");
        return;
    }

    
    com.commandF(&resultData, "-thread-info");    


}


BreakPoint* Core::findBreakPoint(QString fullPath, int lineno)
{
    for(int i = 0;i < m_breakpoints.size();i++)
    {
        BreakPoint *bkpt = m_breakpoints[i];

    
        if(bkpt->lineno == lineno && fullPath == bkpt->fullname)
        {
            return bkpt;
        }
    }

    return NULL;
}


BreakPoint* Core::findBreakPointByNumber(int number)
{
    for(int i = 0;i < m_breakpoints.size();i++)
    {
        BreakPoint *bkpt = m_breakpoints[i];

    
        if(bkpt->m_number == number)
        {
            return bkpt;
        }
    }
    return NULL;
}


void Core::dispatchBreakpointTree(Tree &tree)
{
    int lineno = tree.getInt("bkpt/line");
    int number = tree.getInt("bkpt/number");
                

    BreakPoint *bkpt = findBreakPointByNumber(number);
    if(bkpt == NULL)
    {
        bkpt = new BreakPoint(number);
        m_breakpoints.push_back(bkpt);
    }
    bkpt->lineno = lineno;
    bkpt->fullname = tree.getString("bkpt/fullname");
    bkpt->m_funcName = tree.getString("bkpt/func");
    bkpt->m_addr = tree.getLongLong("bkpt/addr");

    if(m_inf)
        m_inf->ICore_onBreakpointsChanged();


    
}
        
void Core::onResult(Tree &tree)
{
         
    debugMsg("Result>");


    for(int i = 0;i < tree.getRootChildCount();i++)
    {
        TreeNode *rootNode = tree.getChildAt(i);
        QString rootName = rootNode->getName();
        if(rootName == "changelist")
         {
            debugMsg("Changelist");
            for(int j = 0;j < rootNode->getChildCount();j++)
            {
                QString path;
                path.sprintf("changelist/%d/name", j+1);
                int watchId = tree.getString(path).mid(1).toInt();
                path.sprintf("changelist/%d/value", j+1);
                QString varValue = tree.getString(path);

                if(!m_watchList.contains(watchId))
                {
                    assert(!m_watchList.contains(watchId));
                }
                else
                {
                    VarWatch &w = m_watchList[watchId];
        
                    if(m_inf)
                        m_inf->ICore_onWatchVarChanged(watchId, w.name, varValue);
                }
            }
            
        }
        else if(rootName == "bkpt")
        {
            dispatchBreakpointTree(tree);
                
        }
        else if(rootName == "threads")
        {
            m_threadList.clear();
            
            // Parse the result
            QStringList childList = tree.getChildList("threads");
            for(int i = 0;i < childList.size();i++)
            {
                QString treePath = "threads/" + childList[i];
                QString threadId = tree.getString(treePath + "/id");
                QString targetId = tree.getString(treePath + "/target-id");
                QString funcName = tree.getString(treePath + "/frame/func");

                
                ThreadInfo tinfo;
                tinfo.id = atoi(stringToCStr(threadId));
                tinfo.m_name = targetId;
                tinfo.m_func = funcName;
                m_threadList[tinfo.id] = tinfo;
            }

            if(m_inf)
                m_inf->ICore_onThreadListChanged();
            
        }
        else if(rootName == "current-thread-id")
        {
            // Get the current thread
            QString threadIdStr = tree.getString("current-thread-id");
            if(threadIdStr.isEmpty() == false)
            {
                int threadId = threadIdStr.toInt();
                if(m_inf)
                    m_inf->ICore_onCurrentThreadChanged(threadId);
            }
        }
        else if(rootName == "frame")
        {
            QString p = tree.getString("frame/fullname");
            int lineno = tree.getInt("frame/line");
            int frameIdx = tree.getInt("frame/level");
            ICore::StopReason  reason = ICore::UNKNOWN;
             
            m_currentFrameIdx = frameIdx;

            if(m_inf)
            {

                m_inf->ICore_onStopped(reason, p, lineno);

                m_inf->ICore_onFrameVarReset();

                QStringList childList = tree.getChildList("frame/args");
                for(int i = 0;i < childList.size();i++)
                {
                    QString treePath = "frame/args/" + childList[i];
                    QString varName = tree.getString(treePath + "/name");
                    QString varValue = tree.getString(treePath + "/value");
                    if(m_inf)
                        m_inf->ICore_onFrameVarChanged(varName, varValue);
                }
            }
        }
        // A stack frame dump?
        else if(rootName == "stack")
        {
            int cnt = tree.getChildCount("stack");
            QList<StackFrameEntry> stackFrameList;
            for(int j = 0;j < cnt;j++)
            {
                QString path;
                path.sprintf("stack/#%d/func", j+1);
                

                StackFrameEntry entry;
                path.sprintf("stack/#%d/func", j+1);
                entry.m_functionName = tree.getString(path);
                path.sprintf("stack/#%d/line", j+1);
                entry.m_line = tree.getInt(path);
                path.sprintf("stack/#%d/fullname", j+1);
                entry.m_sourcePath = tree.getString(path);
                stackFrameList.push_front(entry);
            }
            if(m_inf)
            {
                m_inf->ICore_onStackFrameChange(stackFrameList);
                m_inf->ICore_onCurrentFrameChanged(m_currentFrameIdx);
            }
        }
        // Local variables?
        else if(rootName == "locals")
        {
            if(m_inf)
            {
                m_inf->ICore_onLocalVarReset();
                

                int cnt = tree.getChildCount("locals");
                for(int j = 0;j < cnt;j++)
                {
                    QString path;
                    path.sprintf("locals/%d/name", j+1);
                    QString varName = tree.getString(path);
                    path.sprintf("locals/%d/value", j+1);
                    QString varData = tree.getString(path);

                    m_inf->ICore_onLocalVarChanged(varName, varData);
                    
                }
            }
        }
        else if(rootName == "msg")
        {
            QString message = tree.getString("msg");
            if(m_inf)
                m_inf->ICore_onMessage(message);
                
        }
        else if(rootName == "groups")
        {
            if(m_pid == 0)
                m_pid = tree.getInt("groups/1/pid");
        }
        
     }
    
    tree.dump();
}

void Core::onStatusAsyncOut(Tree &tree, AsyncClass ac)
{
    infoMsg("StatusAsyncOut> %s", Com::asyncClassToString(ac));
    tree.dump();
}

void Core::onConsoleStreamOutput(QString str)
{
    QStringList list = str.split('\n');
    for(int i = 0;i < list.size();i++)
    {
        QString text = list[i];
        if(text.isEmpty() && i+1 == list.size())
            continue;
            
        infoMsg("GDB | Console-stream | %s", stringToCStr(text));

        if(m_inf)
            m_inf->ICore_onConsoleStream(text);
    }
}


void Core::onTargetStreamOutput(QString str)
{
    QStringList list = str.split('\n');
    for(int i = 0;i < list.size();i++)
        infoMsg("GDB | Target-stream | %s", stringToCStr(list[i]));
}


void Core::onLogStreamOutput(QString str)
{
    QStringList list = str.split('\n');
    for(int i = 0;i < list.size();i++)
        infoMsg("GDB | Log-stream | %s", stringToCStr(list[i]));
}

void Core::gdbSetBreakpoint(QString filename, int lineno)
{
    Com& com = Com::getInstance();
    Tree resultData;
    
    assert(filename != "");
    
    com.commandF(&resultData, "-break-insert %s:%d", stringToCStr(filename), lineno);

}

QList<ThreadInfo> Core::getThreadList()
{
    return m_threadList.values();
}

void Core::selectThread(int threadId)
{
    if(m_selectedThreadId == threadId)
        return;

    Com& com = Com::getInstance();
    Tree resultData;
    
    
    com.commandF(&resultData, "-thread-select %d", threadId);

        
    m_selectedThreadId = threadId;
}


/**
 * @brief Selects a specific frame
 * @param selectedFrameIdx    The frame to select as active (0=newest frame).
 */
void Core::selectFrame(int selectedFrameIdx)
{
    
    Com& com = Com::getInstance();
    Tree resultData;

    if(m_targetState == TARGET_RUNNING)
    {
        return;
    }
    if(m_currentFrameIdx != selectedFrameIdx)
    {
        com.commandF(NULL, "-stack-select-frame %d", selectedFrameIdx);


        com.commandF(&resultData, "-stack-info-frame");

        com.commandF(NULL, "-stack-list-locals 1");
    }

}



    



