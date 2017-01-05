/*
 * Copyright (C) 2014-2016 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "core.h"

#include "ini.h"
#include "util.h"
#include "log.h"
#include "gdbmiparser.h"

#include <QByteArray>
#include <QDebug>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h> // posix_openpt()
#include <fcntl.h> //  O_RDWR
#include <ctype.h>

VarWatch::VarWatch()
    : m_inScope(true)
    ,m_hasChildren(false)
{
}

VarWatch::VarWatch(QString watchId_, QString name_)
  : watchId(watchId_)
    ,name(name_)
    ,m_inScope(true)
    ,m_hasChildren(false)
{

}


bool VarWatch::hasChildren()
{
    return m_hasChildren;
}

void VarWatch::setValue(QString value)
{
    m_var.valueFromGdbString(value);
}


CoreVar::CoreVar()
 : m_address(0)
   ,m_type(TYPE_UNKNOWN)
{

}

    
CoreVar::CoreVar(QString name)
    : m_name(name)
    ,m_address(0)
      ,m_type(TYPE_UNKNOWN)
{

}

CoreVar::~CoreVar()
{
    clear();
}

CoreVar* CoreVar::addChild(QString name)
{
    CoreVar* child = new CoreVar(name);
    m_children.push_back(child);
    return child; 
}
    
long long CoreVar::getAddress()
{
    return m_address;
}

void CoreVar::clear()
{
    for(int j = 0;j < m_children.size();j++)
    {
        CoreVar* child = m_children[j];
        delete child;
    }
    m_children.clear();
}

void CoreVar::valueFromGdbString(QString data)
{
    QList<Token*> tokenList = GdbMiParser::tokenizeVarString(data);
    QList<Token*> orgList = tokenList;

    GdbMiParser::parseVariableData(this, &tokenList);

    for(int i = 0;i < orgList.size();i++)
    {
        Token *tok = orgList[i];
        delete tok;
    }

}

void CoreVar::setData(QString data)
{
    m_data = data;


    // A parent?
    if(data == "...")
    {
        m_data = "{...}";
        m_type = TYPE_UNKNOWN;
    }
    // String?
    else if(data.startsWith('"'))
    {
        if(data.endsWith('"'))
            data = data.mid(1, data.length()-2);
        m_data = data;
        m_type = TYPE_STRING;
    }
    // Character?
    else if(data.startsWith('\''))
    {
        if(data.endsWith('\''))
            data = data.mid(1, data.length()-2);
        if(data.startsWith("\\0"))
            m_data = (int)data.mid(2).toInt();
        else
            m_data = (int)data[0].toAscii();
        
        m_type = TYPE_CHAR;
    }
    // Gdb Error message?
    else if(data.endsWith(">"))
    {
        m_data = data;
        m_type = TYPE_ERROR_MSG;
    }
    // Float?
    else if(data.contains("."))
    {
        m_data = data;
        m_type = TYPE_FLOAT;
    }
    // Integer?
    else if(data.length() > 0)
    {
        if(data.startsWith("0x"))
            m_data = (qulonglong)data.toULongLong(0,0);
        else
            m_data = data.toLongLong(0,0);
        m_type = TYPE_INT;
    }
    else
        m_type = TYPE_UNKNOWN;

}


QString CoreVar::getData(DispFormat fmt) const
{
    QString valueText;

    if(m_type == TYPE_CHAR || m_type == TYPE_INT)
    {
        if((fmt == FMT_NATIVE && m_type == TYPE_CHAR) || fmt == FMT_CHAR)
        {
            QChar c = m_data.toInt();
            char clat = c.toLatin1();
            if(isprint(clat))
                valueText.sprintf("%d '%c'", (int)m_data.toInt(), clat);
            else
                valueText.sprintf("%d ' '", (int)m_data.toInt());
        }
        else if(fmt == FMT_BIN)
        {
            QString subText;
            QString reverseText;
            qlonglong val = m_data.toULongLong();
            do
            {
                subText.sprintf("%d", (int)(val & 0x1));
                reverseText = subText + reverseText;
                val = val>>1;
            }
            while(val > 0 || reverseText.length()%8 != 0);
            for(int i = 0;i < reverseText.length();i++)
            {
                valueText += reverseText[i];
                if(i%4 == 3 && i+1 != reverseText.length())
                    valueText += "_";
            }

            valueText = "0b" + valueText;
        }
        else if(fmt == FMT_HEX)
        {
            QString text;
            text.sprintf("%llx", m_data.toLongLong());

            // Prefix the string with suitable number of zeroes
            while(text.length()%4 != 0 && text.length() > 4)
                text = "0" + text;
            if(text.length()%2 != 0)
                text = "0" + text;
                
            for(int i = 0;i < text.length();i++)
            {
                valueText = valueText + text[i];
                if(i%4 == 3 && i+1 != text.length())
                    valueText += "_";
            }
            valueText = "0x" + valueText;
        }
        else// if(fmt == FMT_DEC)
        {
            valueText = m_data.toString();
        }
    }
    else if(m_type == TYPE_STRING)
    {
          valueText = '"' + m_data.toString() + '"';
    }
    else
        valueText = m_data.toString();

    return valueText;
}

    




Core::Core()
 : m_inf(NULL)
    ,m_selectedThreadId(0)
    ,m_targetState(ICore::TARGET_STOPPED)
    ,m_lastTargetState(ICore::TARGET_FINISHED)
    ,m_pid(0)
    ,m_currentFrameIdx(-1)
    ,m_varWatchLastId(10)
    ,m_isRemote(false)
    ,m_ptsFd(0)
    ,m_scanSources(false)
{
    
    Com& com = Com::getInstance();
    com.setListener(this);

    m_ptsFd = posix_openpt(O_RDWR | O_NOCTTY);
   
    if(grantpt(m_ptsFd))
        errorMsg("Failed to grantpt");
    if(unlockpt(m_ptsFd))
        errorMsg("Failed to unlock pt");
    infoMsg("Using: %s", ptsname(m_ptsFd));
    
    m_ptsListener = new QSocketNotifier(m_ptsFd, QSocketNotifier::Read);
    connect(m_ptsListener, SIGNAL(activated(int)), this, SLOT(onGdbOutput(int)));

}

Core::~Core()
{
    // Clear the local var array
    for(int j = 0;j < m_localVars.size();j++)
    {
        CoreVar *val = m_localVars[j];
        delete val;
    }
    m_localVars.clear();

    for(int i = 0;i < m_watchList.size();i++)
    {
        VarWatch* watch = m_watchList[i];
        delete watch;
    }

    delete m_ptsListener;
    
    Com& com = Com::getInstance();
    com.setListener(NULL);

    close(m_ptsFd);

    for(int m = 0;m < m_sourceFiles.size();m++)
    {
        SourceFile *sourceFile = m_sourceFiles[m];
        delete sourceFile;
    }

}


int Core::initLocal(Settings *cfg, QString gdbPath, QString programPath, QStringList argumentList)
{
    Com& com = Com::getInstance();
    Tree resultData;
    int rc = 0;

    m_isRemote = false;

    if(com.init(gdbPath, cfg->m_enableDebugLog))
    {
        errorMsg("Failed to start gdb ('%s')", stringToCStr(gdbPath));
        return -1;
    }
    
    QString ptsDevPath = ptsname(m_ptsFd);
    
    com.commandF(&resultData, "-inferior-tty-set %s", stringToCStr(ptsDevPath));

    if(com.commandF(&resultData, "-file-exec-and-symbols %s", stringToCStr(programPath)) == GDB_ERROR)
    {
        errorMsg("Failed to load '%s'", stringToCStr(programPath));
    }

    QString commandStr;
    if(argumentList.size() > 0)
    {
        commandStr = "-exec-arguments ";
        for(int i = 0;i < argumentList.size();i++)
            commandStr += " " + argumentList[i];
        com.command(NULL, commandStr);
    }
    
    if(gdbSetBreakpointAtFunc(cfg->m_initialBreakpoint))
    {
        rc = 1;
        errorMsg("Failed to set breakpoint at %s", stringToCStr(cfg->m_initialBreakpoint));
    }

    gdbGetFiles();

    // Run the initializing commands
    for(int i = 0;i < cfg->m_initCommands.size();i++)
    {
        QString cmd = cfg->m_initCommands[i];

        // Remove comments
        if(cmd.indexOf('#') != -1)
            cmd = cmd.left(cmd.indexOf('#'));
        cmd = cmd.trimmed();

        if(!cmd.isEmpty())
            com.commandF(NULL, "%s", stringToCStr(cmd));

    }


    if(rc == 0)
        gdbRun();

    
    return 0;
}

int Core::initRemote(Settings *cfg, QString gdbPath, QString programPath, QString tcpHost, int tcpPort)
{
    Com& com = Com::getInstance();
    Tree resultData;

    m_isRemote = true;
    
    if(com.init(gdbPath, cfg->m_enableDebugLog))
    {
        errorMsg("Failed to start gdb");
        return -1;
    }

    com.commandF(&resultData, "-target-select extended-remote %s:%d", stringToCStr(tcpHost), tcpPort); 

    if(!programPath.isEmpty())
    {
        com.commandF(&resultData, "-file-symbol-file %s", stringToCStr(programPath));

    }

    // Run the initializing commands
    for(int i = 0;i < cfg->m_initCommands.size();i++)
    {
        QString cmd = cfg->m_initCommands[i];

        // Remove comments
        if(cmd.indexOf('#') != -1)
            cmd = cmd.left(cmd.indexOf('#'));
        cmd = cmd.trimmed();

        if(!cmd.isEmpty())
            com.commandF(NULL, "%s", stringToCStr(cmd));

    }

    if(!programPath.isEmpty())
    {
      com.commandF(&resultData, "-file-exec-file %s", stringToCStr(programPath));

        if(cfg->m_download)
            com.commandF(&resultData, "-target-download");
    }
    

    gdbSetBreakpointAtFunc(cfg->m_initialBreakpoint);
    
    gdbGetFiles();

    gdbRun();

    return 0;
}


void Core::onGdbOutput(int socketFd)
{
    Q_UNUSED(socketFd);
    char buff[128];
    buff[0] = '\0';
    int n =  read(m_ptsFd, buff, sizeof(buff)-1);
    if(n > 0)
    {
        buff[n] = '\0';
    }
    m_inf->ICore_onTargetOutput(buff);
}


/**
 * @brief Reads a memory area.
 * @return 0 on success.
 */
int Core::gdbGetMemory(uint64_t addr, size_t count, QByteArray *data)
{
    Com& com = Com::getInstance();
    Tree resultData;

    int rc = 0;
    QString cmdStr;
    cmdStr.sprintf("-data-read-memory-bytes 0x%lx %u" , addr, (unsigned int)count);
    
    rc = com.command(&resultData, cmdStr);


    QString dataStr = resultData.getString("/memory/1/contents");
    if(!dataStr.isEmpty())
    {
        data->clear();

        QByteArray dataByteArray = dataStr.toLocal8Bit();
        const char *dataCStr = dataByteArray.constData();
        int dataCStrLen = strlen(dataCStr);
        for(int i = 0;i+1 < dataCStrLen;i+=2)
        {
            unsigned char dataByte = hexStringToU8(dataCStr+i);
            
            data->push_back(dataByte);
        }
    }

    return rc;
}


/**
* @brief Asks GDB for a list of source files.
* @return true if any files was added or removed.
*/
bool Core::gdbGetFiles()
{
    Com& com = Com::getInstance();
    Tree resultData;
    QMap<QString, bool> fileLookup;
    bool modified = false;
    
    com.command(&resultData, "-file-list-exec-source-files");


    // Clear the old list
    for(int m = 0;m < m_sourceFiles.size();m++)
    {
        SourceFile *sourceFile = m_sourceFiles[m];
        fileLookup[sourceFile->fullName] = false;
        delete sourceFile;
    }
    m_sourceFiles.clear();


    // Create the new list
    for(int k = 0;k < resultData.getRootChildCount();k++)
    {
        TreeNode *rootNode = resultData.getChildAt(k);
        QString rootName = rootNode->getName();

        if(rootName == "files")
        {
            QStringList childList = resultData.getChildList("files");
            for(int j = 0;j < childList.size();j++)
            {
                QString treePath = "files/" + childList[j];
                QString name = resultData.getString(treePath + "/file");
                QString fullname = resultData.getString(treePath + "/fullname");

                if(fullname.isEmpty())
                    continue;

                SourceFile *sourceFile = NULL;
                if(!name.contains("<built-in>"))
                {
                    // Already added this file?
                    bool alreadyAdded = false;
                    if(fileLookup.contains(fullname))
                    {
                        if(fileLookup[fullname] == true)
                            alreadyAdded = true;
                    }
                    else
                        modified = true;
                        
                    if(!alreadyAdded)
                    {
                        fileLookup[fullname] = true;
                        
                        sourceFile = new SourceFile; 

                        sourceFile->name = name;
                        sourceFile->fullName = fullname;

                        m_sourceFiles.append(sourceFile);
                    }
                }
            }
        }
    }

    // Any file removed?
    QMap<QString, bool> ::const_iterator iterFl = fileLookup.constBegin();
    while (iterFl != fileLookup.constEnd()) {
        if(iterFl.value() == false)
            modified = false;
        ++iterFl;
    }
    
    return modified;
}


/**
 * @brief Sets a breakpoint at a function
 */
int Core::gdbSetBreakpointAtFunc(QString func)
{
    Com& com = Com::getInstance();
    Tree resultData;
    int rc = 0;
    int res;
    
    res = com.commandF(&resultData, "-break-insert -f %s", stringToCStr(func));
    if(res == GDB_ERROR)
    {
        rc = -1;
    }
    
    return rc;
}


/**
 * @brief Asks gdb to run the program.
 */
void Core::gdbRun()
{
    Com& com = Com::getInstance();
    Tree resultData;
    ICore::TargetState oldState;

    if(m_targetState == ICore::TARGET_STARTING || m_targetState == ICore::TARGET_RUNNING)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is currently running");
        return;
    }

    m_pid = 0;
    oldState = m_targetState;
    m_targetState = ICore::TARGET_STARTING;
    GdbResult rc = com.commandF(&resultData, "-exec-run");
    if(rc == GDB_ERROR)
        m_targetState = oldState;


}


/**
 * @brief  Resumes the execution until a breakpoint is encountered, or until the program exits.
 */
void Core::gdbContinue()
{
    Com& com = Com::getInstance();
    Tree resultData;

    if(m_targetState == ICore::TARGET_STARTING || m_targetState == ICore::TARGET_RUNNING)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is currently running");
        return;
    }

    com.commandF(&resultData, "-exec-continue");

}


/**
 * @brief Tries to stop the program.
 */ 
void Core::stop()
{
    Com& com = Com::getInstance();

    if(m_targetState != ICore::TARGET_RUNNING)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is not running");
        return;
    }


    if(m_isRemote)
    {
        Com& com = Com::getInstance();
        Tree resultData;

        // Send 'kill' to interrupt gdbserver
        debugMsg("sending INTR to %d", m_pid);
        if(m_pid == 0)
            m_pid = com.getPid();
            
        if(m_pid != 0)
            kill(m_pid, SIGINT);
        else
            errorMsg("Failed to stop since PID not known");


        com.command(NULL, "-exec-interrupt --all");
        com.command(NULL, "-exec-step-instruction");
        
    }
    else
    {
        debugMsg("sending INTR to %d\n", m_pid);
        if(m_pid != 0)
            kill(m_pid, SIGINT);
        else
            errorMsg("Failed to stop since PID not known");
    }
}


/**
 * @brief Execute the next row in the program.
 */
void Core::gdbNext()
{
    Com& com = Com::getInstance();
    Tree resultData;

    if(m_targetState != ICore::TARGET_STOPPED)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is not stopped");
        return;
    }

    com.commandF(&resultData, "-exec-next");

}


/**
 * @brief Request a list of stack frames.
 */
void Core::getStackFrames()
{
    Com& com = Com::getInstance();
    Tree resultData;
    com.command(&resultData, "-stack-list-frames");

}


/**
 * @brief Step in the current line.
 */
void Core::gdbStepIn()
{
    Com& com = Com::getInstance();
    Tree resultData;

    if(m_targetState != ICore::TARGET_STOPPED)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is not stopped");
        return;
    }

        
    com.commandF(&resultData, "-exec-step");
    com.commandF(&resultData, "-var-update --all-values *");

}


/**
 * @brief Step out of the current function.
 */
void Core::gdbStepOut()
{
    Com& com = Com::getInstance();
    Tree resultData;

    if(m_targetState != ICore::TARGET_STOPPED)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is not stopped");
        return;
    }

        
    com.commandF(&resultData, "-exec-finish");
    com.commandF(&resultData, "-var-update --all-values *");

}

Core& Core::getInstance()
{
    static Core core;
    return core;
}


/**
 * @brief Returns info for an existing watch.
 */
VarWatch* Core::getVarWatchInfo(QString watchId)
{
    assert(watchId != "");
    assert(watchId[0] == 'w');
    for(int i = 0;i < m_watchList.size();i++)
    {
        VarWatch* watch = m_watchList[i];
        if(watch->getWatchId() == watchId)
            return watch;
    }
    return NULL;
}


/**
 * @brief Returns all children of a watch.
 */
QList <VarWatch*> Core::getWatchChildren(VarWatch &parentWatch)
{
    QList <VarWatch*> list;
    for(int i = 0;i < m_watchList.size();i++)
    {
        VarWatch* otherWatch = m_watchList[i];
        if(parentWatch.getWatchId() == otherWatch->m_parentWatchId)
            list.push_back(otherWatch);
    }    
    return list;
}


/**
 * @brief Adds a watch for a variable.
 * @param varName   The name of the variable to watch
 * @param watchPtr  Pointer to a watch handle for the newly created watch.
 * @return 0 on success.
 */
int Core::gdbAddVarWatch(QString varName, VarWatch** watchPtr)
{
    Com& com = Com::getInstance();
    Tree resultData;
    QString watchId;
    GdbResult res;
    int rc = 0;

    *watchPtr = NULL;
    
    watchId.sprintf("w%d", m_varWatchLastId++);

    assert(varName.isEmpty() == false);
    
    res = com.commandF(&resultData, "-var-create %s @ %s", stringToCStr(watchId), stringToCStr(varName));
    if(res == GDB_ERROR)
    {
        rc = -1;
    }
    else
    {

    //
    QString varName2 = resultData.getString("name");
    QString varValue2 = resultData.getString("value");
    QString varType2 = resultData.getString("type");
    int numChild = resultData.getInt("numchild", 0);


    // debugMsg("%s = %s = %s\n", stringToCStr(varName2),stringToCStr(varValue2), stringToCStr(varType2));

    VarWatch *w = new VarWatch(watchId,varName);
    w->m_varType = varType2;
    w->setValue(varValue2);
    w->m_hasChildren = numChild > 0 ? true : false;
    m_watchList.append(w);
    
        *watchPtr = w;
        
    }


    return rc;
}


/**
 * @brief Expands all the children of a watched variable.
 * @return 0 on success.
 */
int Core::gdbExpandVarWatchChildren(QString watchId)
{
    int res;
    Tree resultData;
    Com& com = Com::getInstance();

    assert(getVarWatchInfo(watchId) != NULL);

    
    // Request its children
    res = com.commandF(&resultData, "-var-list-children --all-values %s", stringToCStr(watchId));

    if(res != 0)
    {
        return -1;
    }

        
    // Enumerate the children
    QStringList childList = resultData.getChildList("children");
    for(int i = 0;i < childList.size();i++)
    {
        // Get name and value
        QString treePath;
        treePath.sprintf("children/#%d",i+1);
        QString childWatchId = resultData.getString(treePath + "/name");
        QString childExp = resultData.getString(treePath + "/exp");
        QString childValue = resultData.getString(treePath + "/value");
        QString childType = resultData.getString(treePath + "/type");
        int numChild = resultData.getInt(treePath + "/numchild", 0);
        bool hasChildren = false;
        if(numChild > 0)
            hasChildren = true;

        VarWatch *watch = getVarWatchInfo(childWatchId);
        if(watch == NULL)
        {
            watch = new VarWatch(childWatchId,childExp);
            watch->m_inScope = true;
            watch->setValue(childValue);
            watch->m_varType = childType;
            watch->m_hasChildren = hasChildren;
            watch->m_parentWatchId = watchId;
            m_watchList.append(watch);
        }

        m_inf->ICore_onWatchVarChildAdded(*watch);

    }

    return 0;
}


/**
 * @brief Returns the variable name that is being watched.
 */
QString Core::gdbGetVarWatchName(QString watchId)
{
    VarWatch* watch = getVarWatchInfo(watchId);
    if(watch)
        return watch->getName();
    int divPos = watchId.lastIndexOf(".");
    assert(divPos != -1);
    if(divPos == -1)
        return "";
    else
        return watchId.mid(divPos+1);
}

/**
 * @brief Removes a watch.
 */
void Core::gdbRemoveVarWatch(QString watchId)
{
    Com& com = Com::getInstance();
    Tree resultData;
    
    assert(getVarWatchInfo(watchId) != NULL);

    // Remove from the list
    for(int i = 0;i < m_watchList.size();i++)
    {
        VarWatch* watch = m_watchList[i];
        if(watch->getWatchId() == watchId)
        {
            m_watchList.removeAt(i--);
            delete watch;
        }
    }

    // Remove children first
    QStringList removeList;
    for(int i = 0;i < m_watchList.size();i++)
    {
        VarWatch* childWatch = m_watchList[i];
        if(childWatch->m_parentWatchId == watchId)
            removeList.push_back(childWatch->getWatchId());
    }
    for(int i = 0;i < removeList.size();i++)
    {
        gdbRemoveVarWatch(removeList[i]);
    }

    com.commandF(&resultData, "-var-delete %s", stringToCStr(watchId));

     
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
    else if(ac == ComListener::AC_LIBRARY_LOADED)
    {
        m_scanSources = true;
    }
    //tree.dump();
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
    if(reasonString == "function-finished")
        return ICore::FUNCTION_FINISHED;
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
    
    //tree.dump();

    // The program has stopped
    if(ac == ComListener::AC_STOPPED)
    {
        m_targetState = ICore::TARGET_STOPPED;

        if(m_pid == 0)
            com.command(NULL, "-list-thread-groups");
         
        // Any new or destroyed thread?
        com.commandF(NULL, "-thread-info");

        com.commandF(NULL, "-var-update --all-values *");
        com.commandF(NULL, "-stack-list-locals 1");

        if(m_scanSources)
        {
            if(gdbGetFiles())
            {
                m_inf->ICore_onSourceFileListChanged();
            }
            m_scanSources = false;
        }    

        QString p = tree.getString("frame/fullname");
        int lineNo = tree.getInt("frame/line");

        // Get the reason
        QString reasonString = tree.getString("reason");
        ICore::StopReason  reason;
        if(reasonString.isEmpty())
            reason = ICore::UNKNOWN;
        else
            reason = parseReasonString(reasonString);

        if(reason == ICore::EXITED_NORMALLY || reason == ICore::EXITED)
        {
            m_targetState = ICore::TARGET_FINISHED;
        }
        
        if(m_inf)
        {
            if(reason == ICore::SIGNAL_RECEIVED)
            {
                QString signalName = tree.getString("signal-name");
                if(signalName == "SIGSEGV")
                {
                    m_targetState = ICore::TARGET_FINISHED;
                }
                m_inf->ICore_onSignalReceived(signalName);  
            }
            else
                m_inf->ICore_onStopped(reason, p, lineNo);

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
        m_targetState = ICore::TARGET_RUNNING;

        debugMsg("is running");
    }

    // Get the current thread
    QString threadIdStr = tree.getString("thread-id");
    if(threadIdStr.isEmpty() == false)
    {
        int threadId = threadIdStr.toInt(0,0);
        if(m_inf)
            m_inf->ICore_onCurrentThreadChanged(threadId);
    }

    // State changed?
    if(m_inf && m_lastTargetState != m_targetState)
    {
        m_inf->ICore_onStateChanged(m_targetState);
        m_lastTargetState = m_targetState;
    
    }
}


/**
 * @brief Remove a breakpoint.
 */
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

    if(m_targetState == ICore::TARGET_STARTING || m_targetState == ICore::TARGET_RUNNING)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is currently running");
        return;
    }

    
    com.commandF(&resultData, "-thread-info");    


}


/**
 * @brief Find a breakpoint based on path and linenumber.
 */
BreakPoint* Core::findBreakPoint(QString fullPath, int lineNo)
{
    for(int i = 0;i < m_breakpoints.size();i++)
    {
        BreakPoint *bkpt = m_breakpoints[i];

    
        if(bkpt->lineNo == lineNo && fullPath == bkpt->fullname)
        {
            return bkpt;
        }
    }

    return NULL;
}


/**
 * @brief Finds a breakpoint by number.
 */
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
    int lineNo = tree.getInt("bkpt/line");
    int number = tree.getInt("bkpt/number");
                

    BreakPoint *bkpt = findBreakPointByNumber(number);
    if(bkpt == NULL)
    {
        bkpt = new BreakPoint(number);
        m_breakpoints.push_back(bkpt);
    }
    bkpt->lineNo = lineNo;
    bkpt->fullname = tree.getString("bkpt/fullname");

    // We did not receive 'fullname' from gdb.
    // Lets try original-location instead...
    if(bkpt->fullname.isEmpty())
    {
        QString orgLoc = tree.getString("bkpt/original-location");
        int divPos = orgLoc.lastIndexOf(":");
        if(divPos == -1)
            warnMsg("Original-location in unknown format");
        else
        {
            bkpt->fullname = orgLoc.left(divPos);
        }
    }
    
    bkpt->m_funcName = tree.getString("bkpt/func");
    bkpt->m_addr = tree.getLongLong("bkpt/addr");

    if(m_inf)
        m_inf->ICore_onBreakpointsChanged();


    
}



void Core::onResult(Tree &tree)
{
         
    debugMsg("Result>");


    for(int treeChildIdx = 0;treeChildIdx < tree.getRootChildCount();treeChildIdx++)
    {
        TreeNode *rootNode = tree.getChildAt(treeChildIdx);
        QString rootName = rootNode->getName();
        if(rootName == "changelist")
         {
            debugMsg("Changelist");
            for(int j = 0;j < rootNode->getChildCount();j++)
            {
                QString path;
                path.sprintf("changelist/%d/name", j+1);
                QString watchId = tree.getString(path);
                path.sprintf("changelist/%d/type_changed", j+1);
                bool typeChanged = false;
                if(tree.getString(path) == "true")
                    typeChanged = true;
                
                VarWatch *watch = getVarWatchInfo(watchId);

                // If the type has changed then all of the children must be removed.
                if(watch != NULL && typeChanged)
                {
                    QString varName = watch->getName();

                    // Remove children
                    QList <VarWatch*> removeList = getWatchChildren(*watch);

                        
                    for(int cidx = 0;cidx < removeList.size();cidx++)
                    {
                        gdbRemoveVarWatch(removeList[cidx]->getWatchId());
                    }
                    watch->setValue("");
                    path.sprintf("changelist/%d/new_type", j+1);
                    watch->m_varType = tree.getString(path);
                    path.sprintf("changelist/%d/new_num_children", j+1);
                    watch->m_hasChildren = tree.getInt(path) > 0 ? true : false;
                    m_inf->ICore_onWatchVarChanged(*watch);

                }
                // value changed?
                else if(watch)
                {
                    
                path.sprintf("changelist/%d/value", j+1);
                watch->setValue(tree.getString(path));
                path.sprintf("changelist/%d/in_scope", j+1);
                QString inScopeStr = tree.getString(path);
                if(inScopeStr == "true" || inScopeStr.isEmpty())
                    watch->m_inScope = true;
                else
                    watch->m_inScope = false;

                

                if (watch->getValue() == "{...}" && watch->hasChildren() == false)
                    watch->m_hasChildren = true;
                
//                printf("in_scope:%s -> %d\n", stringToCStr(inScopeStr), inScope);

                        
                    if(m_inf)
                    {
                        
                        m_inf->ICore_onWatchVarChanged(*watch);
                    }
                }
                else
                {
                    warnMsg("Received watch info for unknown watch %s", stringToCStr(watchId));
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
            for(int cIdx = 0;cIdx < childList.size();cIdx++)
            {
                QString treePath = "threads/" + childList[cIdx];
                QString threadId = tree.getString(treePath + "/id");
                QString targetId = tree.getString(treePath + "/target-id");
                QString funcName = tree.getString(treePath + "/frame/func");
                QString details = tree.getString(treePath + "/details");

                
                ThreadInfo tinfo;
                tinfo.id = atoi(stringToCStr(threadId));
                tinfo.m_name = targetId;
                tinfo.m_details = details;
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
                int threadId = threadIdStr.toInt(0,0);
                if(m_inf)
                    m_inf->ICore_onCurrentThreadChanged(threadId);
            }
        }
        else if(rootName == "frame")
        {
            QString p = tree.getString("frame/fullname");
            int lineNo = tree.getInt("frame/line");
            int frameIdx = tree.getInt("frame/level");
            ICore::StopReason  reason = ICore::UNKNOWN;
             
            m_currentFrameIdx = frameIdx;

            if(m_inf)
            {

                m_inf->ICore_onStopped(reason, p, lineNo);

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
            // Clear the local var array
            for(int j = 0;j < m_localVars.size();j++)
            {
                CoreVar *val = m_localVars[j];
                delete val;
            }
            m_localVars.clear();
                
            //
            int cnt = tree.getChildCount("locals");
            for(int j = 0;j < cnt;j++)
            {
                QString path;
                path.sprintf("locals/%d/name", j+1);
                QString varName = tree.getString(path);
                path.sprintf("locals/%d/value", j+1);
                QString varData = tree.getString(path);

                CoreVar *val = new CoreVar(varName);
                val->valueFromGdbString(varData);

                m_localVars.push_back(val);
            }

            if(m_inf)
            {
                m_inf->ICore_onLocalVarReset();
                
                for(int j = 0;j < m_localVars.size();j++)
                {
                    CoreVar *val = m_localVars[j];
                    m_inf->ICore_onLocalVarChanged(val);
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
            {
                m_pid = tree.getInt("groups/1/pid");
            }
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
            
        debugMsg("GDB | Console-stream | %s", stringToCStr(text));

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


/**
 * @brief Adds a breakpoint on a specified linenumber.
 */
int Core::gdbSetBreakpoint(QString filename, int lineNo)
{
    Com& com = Com::getInstance();
    Tree resultData;
    int rc = 0;
    
    assert(filename != "");
    
    int res = com.commandF(&resultData, "-break-insert %s:%d", stringToCStr(filename), lineNo);
    if(res == GDB_ERROR)
    {
        rc = -1;
        errorMsg("Failed to set breakpoint at %s:%d", stringToCStr(filename), lineNo);
    }
    
    return rc;
}


/**
 * @brief Returns a list of threads.
 */
QList<ThreadInfo> Core::getThreadList()
{
    return m_threadList.values();
}


/**
 * @brief Changes context to a specified thread.
 */
void Core::selectThread(int threadId)
{
    if(m_selectedThreadId == threadId)
        return;

    Com& com = Com::getInstance();
    Tree resultData;
    
    
    if(com.commandF(&resultData, "-thread-select %d", threadId) == GDB_DONE)
    {

        
        m_selectedThreadId = threadId;
    }
}


/**
 * @brief Selects a specific frame
 * @param selectedFrameIdx    The frame to select as active (0=newest frame).
 */
void Core::selectFrame(int selectedFrameIdx)
{
    
    Com& com = Com::getInstance();
    Tree resultData;

    if(m_targetState == ICore::TARGET_STARTING || m_targetState == ICore::TARGET_RUNNING)
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


/**
 * @brief Changes the content of a variable.
 */
int Core::changeWatchVariable(QString watchId, QString newValue)
{
    QString dataStr;
    Com& com = Com::getInstance();
    Tree resultData;
    int rc = 0;
    GdbResult gdbRes = GDB_ERROR;
    
    if(m_targetState == ICore::TARGET_STARTING || m_targetState == ICore::TARGET_RUNNING)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is currently running");
        return -1;
    }

    //
    dataStr = newValue;
    QString varName = watchId;
    VarWatch *watch = getVarWatchInfo(watchId);
    if(watch)
    {
        varName = watch->getName();
        QString varType = watch->getVarType();
        if(varType == "char" && newValue.length() == 1)
            dataStr = '\'' + newValue + '\'';
    }

    gdbRes = com.commandF(&resultData, "-var-assign %s %s", stringToCStr(watchId), stringToCStr(dataStr));    
    if(gdbRes == GDB_DONE)
    {

        com.commandF(&resultData, "-var-update --all-values *");
    }
    else if(gdbRes == GDB_ERROR)
    {
        rc = -1;
        errorMsg("Failed to change variable %s", stringToCStr(watchId));
    }
    return rc;
}




