/*
 * Copyright (C) 2014-2016 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

//#define ENABLE_DEBUGMSG

#include "core.h"

#include <QByteArray>
#include <QDebug>
#include <QFileInfo>

#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h> // posix_openpt()
#include <fcntl.h> //  O_RDWR
#include <ctype.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

#include "ini.h"
#include "util.h"
#include "log.h"
#include "gdbmiparser.h"


VarWatch::VarWatch()
    : m_inScope(true)
    ,m_hasChildren(false)
{
}

VarWatch::VarWatch(QString watchId_, QString name_)
  : m_watchId(watchId_)
    ,m_name(name_)
    ,m_inScope(true)
    ,m_var(name_)
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
   ,m_addressValid(false)
{

}

    
CoreVar::CoreVar(QString name)
    : m_name(name)
    ,m_address(0)
      ,m_type(TYPE_UNKNOWN)
    ,m_addressValid(false)
{

}

CoreVar::~CoreVar()
{
    clear();
}

    
quint64 CoreVar::getPointerAddress()
{
    return m_address;
}

void CoreVar::clear()
{
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

void CoreVar::setData(Type type, QVariant data)
{
    m_type = type;
    m_data = data;
}


QString CoreVar::getData(DispFormat fmt) const
{
    QString valueText;

    if(fmt == FMT_NATIVE)
    {
        if(m_type == TYPE_HEX_INT)
            fmt = FMT_HEX;
        if(m_type == TYPE_CHAR)
            fmt = FMT_CHAR;
    }

    if(m_type == TYPE_ENUM)
        return m_data.toString();
    else if(m_type == TYPE_CHAR || m_type == TYPE_HEX_INT || m_type == TYPE_DEC_INT)
    {
        if(fmt == FMT_CHAR)
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

    
int Core::openPseudoTerminal()
{
    int ptsFd = posix_openpt(O_RDWR | O_NOCTTY);
   
    if(grantpt(ptsFd))
        critMsg("Failed to grantpt");
    if(unlockpt(ptsFd))
        critMsg("Failed to unlock pt");

    // Set window size
    struct winsize term_winsize;
    term_winsize.ws_col = 80;
    term_winsize.ws_row = 20;
    term_winsize.ws_xpixel = 80 * 8;
    term_winsize.ws_ypixel = 20 * 8;
    if(ioctl(ptsFd, TIOCSWINSZ, &term_winsize) < 0)
    {
        errorMsg("ioctl TIOCSWINSZ failed (%s)", strerror(errno));
    }

/*
    // Set controlling
    if (ioctl(ptsFd, TIOCSCTTY, (char *)0) < 0)
    {
        errorMsg("ioctl TIOCSCTTY failed (%s)", strerror(errno));
    }
*/
    return ptsFd;
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
    ,m_ptsListener(NULL)
    ,m_memDepth(32)
{
    
    GdbCom& com = GdbCom::getInstance();
    com.setListener(this);

    m_ptsFd = openPseudoTerminal();





    infoMsg("Using: %s", ptsname(m_ptsFd));
    

}

Core::~Core()
{
    m_localVars.clear();

    for(int i = 0;i < m_watchList.size();i++)
    {
        VarWatch* watch = m_watchList[i];
        delete watch;
    }

    delete m_ptsListener;
    m_ptsListener = NULL;
    GdbCom& com = GdbCom::getInstance();
    com.setListener(NULL);

    close(m_ptsFd);

    for(int m = 0;m < m_sourceFiles.size();m++)
    {
        SourceFile *sourceFile = m_sourceFiles[m];
        delete sourceFile;
    }

}


/**
 * @brief Returns the memory depth.
 * @return 32 or 64.
 */
int Core::getMemoryDepth()
{
    return m_memDepth;
}


/**
 * @brief Detects memory depth of target by communicating with Gdb.
 */
void Core::detectMemoryDepth()
{
    Tree resultData;
    GdbCom& com = GdbCom::getInstance();
    if(com.commandF(&resultData, "-data-evaluate-expression \"sizeof(void *)\""))
    {
        warnMsg("Failed to detect memory depth");    
    }
    else
    {
        int byteDepth = resultData.getInt("value");
        m_memDepth = byteDepth*8;
    }

}



/**
 * @brief Connects to a running program (with a specific PID).
 */
int Core::initPid(Settings *cfg, QString gdbPath, QString programPath, int pid)
{
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;
    int rc = 0;

    m_isRemote = false;

    if(com.init(gdbPath, cfg->m_enableDebugLog))
    {
        critMsg("Failed to start gdb ('%s')", stringToCStr(gdbPath));
        return -1;
    }
    
    QString ptsDevPath = ptsname(m_ptsFd);
    
    if(com.commandF(&resultData, "-inferior-tty-set %s", stringToCStr(ptsDevPath)))
    {
        rc = 1;
        critMsg("Failed to set inferior tty");
    }

    if(com.commandF(&resultData, "-file-exec-and-symbols %s", stringToCStr(programPath)) == GDB_ERROR)
    {
        critMsg("Failed to load '%s'", stringToCStr(programPath));
    }


    // Get memory depth (32 or 64)
    detectMemoryDepth();

    if(com.commandF(NULL, "-target-attach %d", pid))
    {
        critMsg("Failed to attach to %d", pid);
        return 1;
    }
    
    if(gdbSetBreakpointAtFunc(cfg->m_initialBreakpoint))
    {
        rc = 1;
        warnMsg("Failed to set breakpoint at %s", stringToCStr(cfg->m_initialBreakpoint));
    }

    runInitCommands(cfg);

    gdbGetFiles();

    return rc;
}


/**
 * @brief Execute the init commands (supplied by the user).
 */
int Core::runInitCommands(Settings *cfg)
{
    GdbCom& com = GdbCom::getInstance();

    // Loop through the initializing commands
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

    return 0;
}

int Core::initLocal(Settings *cfg, QString gdbPath, QString programPath, QStringList argumentList)
{
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;
    int rc = 0;

    m_isRemote = false;

    if(com.init(gdbPath, cfg->m_enableDebugLog))
    {
        critMsg("Failed to start gdb ('%s')", stringToCStr(gdbPath));
        return -1;
    }
    
    QString ptsDevPath = ptsname(m_ptsFd);
    
    if(com.commandF(&resultData, "-inferior-tty-set %s", stringToCStr(ptsDevPath)))
    {
        rc = 1;
        critMsg("Failed to set inferior tty");
    }

    if(com.commandF(&resultData, "-file-exec-and-symbols %s", stringToCStr(programPath)) == GDB_ERROR)
    {
        critMsg("Failed to load '%s'", stringToCStr(programPath));
    }


    // Get memory depth (32 or 64)
    detectMemoryDepth();


    QString commandStr;
    if(argumentList.size() > 0)
    {
        commandStr = "-exec-arguments ";
        for(int i = 0;i < argumentList.size();i++)
            commandStr += " " + argumentList[i];
        com.command(NULL, commandStr);
    }
    

    runInitCommands(cfg);

    if(gdbSetBreakpointAtFunc(cfg->m_initialBreakpoint))
    {
        rc = 1;
        warnMsg("Failed to set breakpoint at %s", stringToCStr(cfg->m_initialBreakpoint));
    }


    gdbGetFiles();

    return rc;
}



/**
 * @brief Init gdb with a core dumpfile.
 */
int Core::initCoreDump(Settings *cfg, QString gdbPath, QString programPath, QString coreDumpFile)
{
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;
    int rc = 0;

    m_isRemote = false;


    if(com.init(gdbPath, cfg->m_enableDebugLog))
    {
        critMsg("Failed to start gdb ('%s')", stringToCStr(gdbPath));
        return -1;
    }

    // Load the symbols
    if(!programPath.isEmpty())
    {
        com.commandF(&resultData, "-file-exec-and-symbols %s", stringToCStr(programPath));
    }

    // Load the coredump file
    com.commandF(&resultData, "-target-select core %s", stringToCStr(coreDumpFile)); 

    
    QString ptsDevPath = ptsname(m_ptsFd);
    
    if(com.commandF(&resultData, "-inferior-tty-set %s", stringToCStr(ptsDevPath)))
    {
        rc = 1;
        critMsg("Failed to set inferior tty");
    }


    // Get memory depth (32 or 64)
    detectMemoryDepth();

    runInitCommands(cfg);

    gdbGetFiles();

    m_targetState = ICore::TARGET_FINISHED;
    if(m_inf)
        m_inf->ICore_onStateChanged(ICore::TARGET_FINISHED);


    com.commandF(NULL, "-stack-list-variables --no-values");
    
    return rc;
}

    

int Core::initRemote(Settings *cfg, QString gdbPath, QString programPath, QString tcpHost, int tcpPort)
{
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;

    m_isRemote = true;
    
    if(com.init(gdbPath, cfg->m_enableDebugLog))
    {
        critMsg("Failed to start gdb ('%s')", stringToCStr(gdbPath));
        return -1;
    }

    com.commandF(&resultData, "-target-select extended-remote %s:%d", stringToCStr(tcpHost), tcpPort); 

    if(!programPath.isEmpty())
    {
        com.commandF(&resultData, "-file-symbol-file %s", stringToCStr(programPath));

    }

    runInitCommands(cfg);

    if(!programPath.isEmpty())
    {
      com.commandF(&resultData, "-file-exec-file %s", stringToCStr(programPath));

        if(cfg->m_download)
            com.commandF(&resultData, "-target-download");
    }
    
    // Get memory depth (32 or 64)
    detectMemoryDepth();

    if(gdbSetBreakpointAtFunc(cfg->m_initialBreakpoint))
    {
        warnMsg("Failed to set breakpoint at %s", stringToCStr(cfg->m_initialBreakpoint));
    }

    gdbGetFiles();

    
    return 0;
}

/**
 * @brief Writes to stdin of the program being debugged.
 */
void Core::writeTargetStdin(QString text)
{
    QByteArray rawData = text.toLocal8Bit();
    int bytesWritten = 0;
    int n;
    do
    {
        n = write(m_ptsFd, rawData.constData()+bytesWritten, rawData.size()-bytesWritten);
        if(n > 0)
            bytesWritten += n;
        else if(n < 0)
            errorMsg("Failed to write data to target stdin");
    }while(n > 0 && bytesWritten != rawData.size());
    fsync(m_ptsFd);
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

        QString str = QString::fromUtf8(buff);
        m_inf->ICore_onTargetOutput(str);

    }
    else
    {
        delete m_ptsListener;
        m_ptsListener = NULL;
    }

}


/**
 * @brief Reads a memory area.
 * @return 0 on success.
 */
int Core::gdbGetMemory(quint64 addr, size_t count, QByteArray *data)
{
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;

    int rc = 0;
    QString cmdStr;
    cmdStr.sprintf("-data-read-memory-bytes 0x%llx %u" , (long long)addr, (unsigned int)count);
    
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
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;
    QMap<QString, bool> fileLookup;
    bool modified = false;
    
    com.command(&resultData, "-file-list-exec-source-files");


    // Clear the old list
    for(int m = 0;m < m_sourceFiles.size();m++)
    {
        SourceFile *sourceFile = m_sourceFiles[m];
        fileLookup[sourceFile->m_fullName] = false;
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
            for(int j = 0;j < rootNode->getChildCount();j++)
            {
                TreeNode *childNode = rootNode->getChild(j);
                QString name = childNode->getChildDataString("file");
                QString fullname = childNode->getChildDataString("fullname");

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

                        sourceFile->m_name = name;
                        sourceFile->m_fullName = fullname;
                        sourceFile->m_modTime = QDateTime::currentDateTime();

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
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;
    int rc = 0;
    int res;

    ensureStopped();
    
    res = com.commandF(&resultData, "-break-insert -f %s", stringToCStr(func));
    if(res == GDB_ERROR)
    {
        rc = -1;
    }
    
    return rc;
}


/**
 * @brief Asks gdb to run the program from the beginning.
 */
void Core::gdbRun()
{
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;
    ICore::TargetState oldState;


    if(m_targetState == ICore::TARGET_STARTING || m_targetState == ICore::TARGET_RUNNING)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is currently running");
        return;
    }

    //
    if(m_ptsListener)
        delete m_ptsListener;
    m_ptsListener = new QSocketNotifier(m_ptsFd, QSocketNotifier::Read);
    connect(m_ptsListener, SIGNAL(activated(int)), this, SLOT(onGdbOutput(int)));


    m_pid = 0;
    oldState = m_targetState;
    m_targetState = ICore::TARGET_STARTING;
    GdbResult rc = com.commandF(&resultData, "-exec-run");
    if(rc == GDB_ERROR)
        m_targetState = oldState;



    if(!m_isRemote)
    {
        // Loop through all source files
        for(int i = 0;i < m_sourceFiles.size();i++)
        {
            SourceFile *sourceFile = m_sourceFiles[i];

            if(QFileInfo(sourceFile->m_fullName).exists())
            {
                // Has the file being modified?
                QDateTime modTime = QFileInfo(sourceFile->m_fullName).lastModified();
                if(sourceFile->m_modTime <  modTime)
                {
                    m_inf->ICore_onSourceFileChanged(sourceFile->m_fullName);
                }
            }
        }

        // Get all source files
        gdbGetFiles();
                                
    }
}


/**
 * @brief  Resumes the execution until a breakpoint is encountered, or until the program exits.
 */
void Core::gdbContinue()
{
    GdbCom& com = GdbCom::getInstance();
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
 * @brief Ensures that the program is running
 */
void Core::ensureStopped()
{
    if(m_targetState == ICore::TARGET_RUNNING)
        stop();
}


/**
 * @brief Tries to stop the program.
 */ 
void Core::stop()
{
    
    if(m_targetState != ICore::TARGET_RUNNING)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is not running");
        return;
    }


    if(m_isRemote)
    {
        GdbCom& com = GdbCom::getInstance();
        Tree resultData;

        // Send 'kill' to interrupt gdbserver
        int gdbPid = com.getPid();
        if(gdbPid != 0)
        {
            debugMsg("Sending SIGINT to %d", gdbPid);
            kill(gdbPid, SIGINT);
        }
        else
        {
            critMsg("Failed to stop since PID of Gdb not known");
        }


        com.command(NULL, "-exec-interrupt --all");
        com.command(NULL, "-exec-step-instruction");
        
    }
    else
    {
        if(m_pid != 0)
        {
            debugMsg("sending SIGINT to %d\n", m_pid);
            kill(m_pid, SIGINT);
        }
        else
            critMsg("Failed to stop since PID not known");
    }
}


/**
 * @brief Execute the next row in the program.
 */
void Core::gdbNext()
{
    GdbCom& com = GdbCom::getInstance();
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
 * @brief Jumps to a location (by changing program counter).
 * @return 0 on success.
 */
int Core::jump(QString filename, int lineNo)
{
    int res;
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;

    if(m_targetState != ICore::TARGET_STOPPED)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is not stopped");
        return -3;
    }

    res = com.commandF(&resultData, "-break-insert -t %s:%d", stringToCStr(filename), lineNo);
    if(res != GDB_DONE)
        return -1;
    resultData.removeAll();
        
    res = com.commandF(&resultData, "-exec-jump %s:%d", stringToCStr(filename), lineNo);
    if(res != GDB_DONE)
        return -2;

    return 0;
}



/**
 * @brief Request a list of stack frames.
 */
void Core::getStackFrames()
{
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;
    com.command(&resultData, "-stack-list-frames");

}


/**
 * @brief Step in the current line.
 */
void Core::gdbStepIn()
{
    GdbCom& com = GdbCom::getInstance();
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
    GdbCom& com = GdbCom::getInstance();
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
 * @brief Evaluate an data expression.
 * @return 0 on success.
 */
int Core::evaluateExpression(QString expr, QString *data)
{
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;
    GdbResult res;
    int rc = 0;
    
    assert(expr.isEmpty() == false);

    if(expr.isEmpty())
        return -1;
    
    res = com.commandF(&resultData, "-data-evaluate-expression %s", stringToCStr(expr));
    if(res != GDB_DONE)
    {
        rc = -1;
    }
    else
        *data = resultData.getString("value");

    return rc;
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
 * @param watch     Watch handle that will be used for the enw watch
 * @return 0 on success.
 */
int Core::priv_gdbVarWatchCreate(QString varName, QString watchId, VarWatch* watch)
{
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;
    GdbResult res;
    int rc = 0;

    

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

    watch->m_varType = varType2;
    watch->setValue(varValue2);
    watch->m_hasChildren = numChild > 0 ? true : false;
        
    }


    return rc;

}


/**
 * @brief Adds a watch for a variable.
 * @param varName   The name of the variable to watch
 * @param watchPtr  Pointer to a watch handle for the newly created watch.
 * @return 0 on success.
 */
int Core::gdbAddVarWatch(QString varName, VarWatch** watchPtr)
{
    int rc = 0;
    QString watchId;
    watchId.sprintf("w%d", m_varWatchLastId++);
    VarWatch *w = new VarWatch(watchId,varName);
    rc = priv_gdbVarWatchCreate(varName, watchId, w);
    if(rc)
    {
        delete w;
        w = NULL;
    }
    else
    {
        m_watchList.append(w);
    }
    
    *watchPtr = w;
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
    GdbCom& com = GdbCom::getInstance();

    assert(getVarWatchInfo(watchId) != NULL);

    
    // Request its children
    res = com.commandF(&resultData, "-var-list-children --simple-values %s", stringToCStr(watchId));

    if(res != 0)
    {
        return -1;
    }

        
    // Enumerate the children
    TreeNode* root = resultData.findChild("children");
    if(root)
    {
    for(int i = 0;i < root->getChildCount();i++)
    {
        // Get name and value
        TreeNode* child = root->getChild(i);
        QString childWatchId = child->getChildDataString("name");
        QString childExp = child->getChildDataString("exp");
        QString childValue = child->getChildDataString("value");
        QString childType = child->getChildDataString("type");
        int numChild = child->getChildDataInt("numchild", 0);
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
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;
    
    ensureStopped();

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
    debugMsg("NotifyAsyncOut> %s", GdbCom::asyncClassToString(ac));


    if(ac == GdbComListener::AC_BREAKPOINT_DELETED)
    {
        int id = tree.getInt("id");
        dispatchBreakpointDeleted(id);
    }
    else if(ac == GdbComListener::AC_BREAKPOINT_MODIFIED)
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
    else if(ac == GdbComListener::AC_THREAD_CREATED)
    {
        gdbGetThreadList();
        
    }
    else if(ac == GdbComListener::AC_LIBRARY_LOADED)
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
    GdbCom& com = GdbCom::getInstance();

    debugMsg("ExecAsyncOut> %s", GdbCom::asyncClassToString(ac));
    
    //tree.dump();

    // The program has stopped
    if(ac == GdbComListener::AC_STOPPED)
    {
        m_targetState = ICore::TARGET_STOPPED;

        if(m_pid == 0)
            com.command(NULL, "-list-thread-groups");
         
        // Any new or destroyed thread?
        com.commandF(NULL, "-thread-info");

        com.commandF(NULL, "-var-update --all-values *");
        com.commandF(NULL, "-stack-list-variables --no-values");

        if(m_scanSources)
        {
            if(gdbGetFiles())
            {
                m_inf->ICore_onSourceFileListChanged();
            }
            m_scanSources = false;
        }

        


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
            QString p = tree.getString("frame/fullname");
            int lineNo = tree.getInt("frame/line");

            if(reason == ICore::SIGNAL_RECEIVED)
            {
                QString signalName = tree.getString("signal-name");
                if(signalName == "SIGTRAP" && m_isRemote)
                {
                    m_inf->ICore_onStopped(reason, p, lineNo);
                }
                else
                {
                    if(signalName == "SIGSEGV")
                    {
                        m_targetState = ICore::TARGET_FINISHED;
                    }
                    m_inf->ICore_onSignalReceived(signalName);  
                }
            }
            else
                m_inf->ICore_onStopped(reason, p, lineNo);

            m_inf->ICore_onFrameVarReset();


            TreeNode *argsNode = tree.findChild("frame/args");
            if(argsNode)
            {
                for(int i = 0;i < argsNode->getChildCount();i++)
                {
                    TreeNode *child2 = argsNode->getChild(i);
                    QString varName = child2->getChildDataString("name");
                    QString varValue = child2->getChildDataString("value");
                    if(m_inf)
                        m_inf->ICore_onFrameVarChanged(varName, varValue);
                }
            }

            int frameIdx = tree.getInt("frame/level");
            m_currentFrameIdx = frameIdx;
            m_inf->ICore_onCurrentFrameChanged(frameIdx);

        }
    }
    else if(ac == GdbComListener::AC_RUNNING)
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

void Core::gdbRemoveAllBreakpoints()
{
    ensureStopped();
    
    // Get id for all breakpoints
    QList <int> idList;
    for(int i = 0;i < m_breakpoints.size();i++)
    {
        BreakPoint* bkpt = m_breakpoints[i];
        idList.append(bkpt->m_number);
        delete bkpt;
    }
    m_breakpoints.clear();

    // Remove all
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;
    for(int u = 0;u < idList.size();u++)
    {
        int id = idList[u];
        com.commandF(&resultData, "-break-delete %d", id);
    }
    
    if(m_inf)
        m_inf->ICore_onBreakpointsChanged();
    

}


/**
 * @brief Remove a breakpoint.
 */
void Core::gdbRemoveBreakpoint(BreakPoint* bkpt)
{
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;

    assert(bkpt != NULL);

    ensureStopped();
        
    com.commandF(&resultData, "-break-delete %d", bkpt->m_number);    

    m_breakpoints.removeOne(bkpt);

    if(m_inf)
        m_inf->ICore_onBreakpointsChanged();
    delete bkpt;
    
}

void Core::gdbGetThreadList()
{
    GdbCom& com = GdbCom::getInstance();
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

    
        if(bkpt->m_lineNo == lineNo && fullPath == bkpt->m_fullname)
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

void Core::dispatchBreakpointDeleted(int id)
{

    BreakPoint *bkpt = findBreakPointByNumber(id);
    if(bkpt == NULL)
    {
        warnMsg("Unknown breakpoint %d deleted", id);
    }
    else
        m_breakpoints.removeOne(bkpt);

    if(m_inf)
        m_inf->ICore_onBreakpointsChanged();

}

void Core::dispatchBreakpointTree(Tree &tree)
{
    TreeNode *rootNode = tree.findChild("bkpt");
    if(!rootNode)
        return;
    int lineNo = rootNode->getChildDataInt("line");
    int number = rootNode->getChildDataInt("number");
                

    BreakPoint *bkpt = findBreakPointByNumber(number);
    if(bkpt == NULL)
    {
        bkpt = new BreakPoint(number);
        m_breakpoints.push_back(bkpt);
    }
    bkpt->m_lineNo = lineNo;
    bkpt->m_fullname = rootNode->getChildDataString("fullname");

    // We did not receive 'fullname' from gdb.
    // Lets try original-location instead...
    if(bkpt->m_fullname.isEmpty())
    {
        QString orgLoc = rootNode->getChildDataString("original-location");
        int divPos = orgLoc.lastIndexOf(":");
        if(divPos == -1)
            warnMsg("Original-location in unknown format");
        else
        {
            bkpt->m_fullname = orgLoc.left(divPos);
        }
    }
    
    bkpt->m_funcName = rootNode->getChildDataString("func");
    bkpt->m_addr = rootNode->getChildDataLongLong("addr");

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
                TreeNode *child = rootNode->getChild(j);
                QString watchId = child->getChildDataString("name");
                VarWatch *watch = getVarWatchInfo(watchId);

                bool typeChanged = false;
                
                // Watch no longer exist?
                QString inscopeText = child->getChildDataString("in_scope");
                if(inscopeText == "invalid")
                {
                    QString varName = watch->getName();
                    
                    m_inf->ICore_onWatchVarDeleted(*watch);

                    gdbRemoveVarWatch(watchId);

                    watch = NULL;
                }
                else
                {
                // If the type has changed then all of the children must be removed.
                QString typeChangeText = child->getChildDataString("type_changed");
                if(typeChangeText == "true")
                    typeChanged = true;
                else if(watch != NULL && typeChanged)
                {
                    QString varName = watch->getName();

                    // Remove children
                    QList <VarWatch*> removeList = getWatchChildren(*watch);

                        
                    for(int cidx = 0;cidx < removeList.size();cidx++)
                    {
                        gdbRemoveVarWatch(removeList[cidx]->getWatchId());
                    }
                    watch->setValue("");
                    watch->m_varType = child->getChildDataString("new_type");
                    watch->m_hasChildren = child->getChildDataInt("new_num_children") > 0 ? true : false;
                    m_inf->ICore_onWatchVarChanged(*watch);

                }
                // value changed?
                else if(watch)
                {
                    
                watch->setValue(child->getChildDataString("value"));
                QString inScopeStr = child->getChildDataString("in_scope");
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
            
        }
        else if(rootName == "bkpt")
        {
            dispatchBreakpointTree(tree);
                
        }
        else if(rootName == "threads")
        {
            m_threadList.clear();
            
            // Parse the result
            for(int cIdx = 0;cIdx < rootNode->getChildCount();cIdx++)
            {
                TreeNode *child = rootNode->getChild(cIdx);
                QString threadId = child->getChildDataString("id");
                QString targetId = child->getChildDataString("target-id");
                QString funcName = child->getChildDataString("frame/func");
                QString lineNo  = child->getChildDataString("frame/line");
                QString details = child->getChildDataString("details");

                if(details.isEmpty())
                {
                    if(!funcName.isEmpty())
                    {
                        details = QString("Executing %1()").arg(funcName);
                        if(!lineNo.isEmpty())
                            details += QString(" @ L%1").arg(lineNo);
                    }
                }
                
                ThreadInfo tinfo;
                tinfo.m_id = atoi(stringToCStr(threadId));
                tinfo.m_name = targetId;
                tinfo.m_details = details;
                tinfo.m_func = funcName;
                m_threadList[tinfo.m_id] = tinfo;
            }

            if(m_inf)
                m_inf->ICore_onThreadListChanged();
            
        }
        else if(rootName == "current-thread-id")
        {
            // Get the current thread
            int threadId = rootNode->getDataInt(-1);
            if(threadId != -1 && m_inf)
                m_inf->ICore_onCurrentThreadChanged(threadId);
            
        }
        else if(rootName == "frame")
        {
            QString p = rootNode->getChildDataString("fullname");
            int lineNo = rootNode->getChildDataInt("line");
            int frameIdx = rootNode->getChildDataInt("level");
            ICore::StopReason  reason = ICore::UNKNOWN;
             
            m_currentFrameIdx = frameIdx;

            if(m_inf)
            {

                m_inf->ICore_onStopped(reason, p, lineNo);

                m_inf->ICore_onFrameVarReset();

                TreeNode *argsNode = rootNode->findChild("args");
                if(argsNode)
                {
                for(int i = 0;i < argsNode->getChildCount();i++)
                {
                    TreeNode *child = argsNode->getChild(i);
                    QString varName = child->getChildDataString("name");
                    QString varValue = child->getChildDataString("value");
                    if(m_inf)
                        m_inf->ICore_onFrameVarChanged(varName, varValue);
                }
                }
            }
        }
        // A stack frame dump?
        else if(rootName == "stack")
        {
            QList<StackFrameEntry> stackFrameList;
            for(int j = 0;j < rootNode->getChildCount();j++)
            {
                const TreeNode *child = rootNode->getChild(j);
                
                StackFrameEntry entry;
                entry.m_functionName = child->getChildDataString("func");
                entry.m_line = child->getChildDataInt("line");
                entry.m_sourcePath = child->getChildDataString("fullname");
                stackFrameList.push_front(entry);
            }
            if(m_inf)
            {
                m_inf->ICore_onStackFrameChange(stackFrameList);
                m_inf->ICore_onCurrentFrameChanged(m_currentFrameIdx);
            }
        }
        // Local variables?
        else if(rootName == "variables")
        {
            // Clear the local var array
            m_localVars.clear();
                
            //
            for(int j = 0;j < rootNode->getChildCount();j++)
            {
                TreeNode *child = rootNode->getChild(j);
                QString varName = child->getChildDataString("name");

                m_localVars.push_back(varName);
            }

            if(m_inf)
            {
                m_inf->ICore_onLocalVarChanged(m_localVars);
            }
        }
        else if(rootName == "msg")
        {
            QString message = rootNode->getData();
            if(m_inf)
                m_inf->ICore_onMessage(message);
                
        }
        else if(rootName == "groups")
        {
            if(m_pid == 0 && rootNode->getChildCount() > 0)
            {
                TreeNode *firstChild = rootNode->getChild(0);
                m_pid = firstChild->getChildDataInt("pid", 0);
            }
        }
        
     }
    
    //tree.dump();
}

void Core::onStatusAsyncOut(Tree &tree, AsyncClass ac)
{
    infoMsg("StatusAsyncOut> %s", GdbCom::asyncClassToString(ac));
    tree.dump();
}

void Core::onConsoleStreamOutput(QString str)
{
    str.replace("\r","");
    QStringList list = str.split('\n');
    for(int i = 0;i < list.size();i++)
    {
        QString text = list[i];
        if(text.isEmpty() && i+1 == list.size())
            continue;
            
        debugMsg("GDB | Console-stream | '%s'", stringToCStr(text));

        if(m_inf)
            m_inf->ICore_onConsoleStream(text);
    }
}


void Core::onTargetStreamOutput(QString str)
{
    str.replace("\r","");
    QStringList list = str.split('\n');
    for(int i = 0;i < list.size();i++)
        infoMsg("GDB | Target-stream | %s", stringToCStr(list[i]));
}


void Core::onLogStreamOutput(QString str)
{
    str.replace("\r","");
    if(str.endsWith('\n'))
        str = str.left(str.size()-1);
    QStringList list = str.split('\n');
    for(int i = 0;i < list.size();i++)
        infoMsg("GDB | Log-stream | %s", stringToCStr(list[i]));
}


/**
 * @brief Adds a breakpoint on a specified linenumber.
 */
int Core::gdbSetBreakpoint(QString filename, int lineNo)
{
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;
    int rc = 0;
    
    assert(filename != "");
    if(filename.isEmpty())
        return -1;

    ensureStopped();
    
    int res = com.commandF(&resultData, "-break-insert %s:%d", stringToCStr(filename), lineNo);
    if(res == GDB_ERROR)
    {
        rc = -1;
        warnMsg("Failed to set breakpoint at %s:%d", stringToCStr(filename), lineNo);
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

    GdbCom& com = GdbCom::getInstance();
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
    
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;

    if(m_targetState == ICore::TARGET_STARTING || m_targetState == ICore::TARGET_RUNNING)
    {
        return;
    }
    if(m_currentFrameIdx != selectedFrameIdx)
    {
        com.commandF(NULL, "-stack-select-frame %d", selectedFrameIdx);


        com.commandF(&resultData, "-stack-info-frame");

        com.commandF(NULL, "-stack-list-variables --no-values");
    }

}


/**
 * @brief Changes the content of a variable.
 */
int Core::changeWatchVariable(QString watchId, QString newValue)
{
    QString dataStr;
    GdbCom& com = GdbCom::getInstance();
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


/**
 * @brief Returns the address in memory where the variable is stored in.
 */
quint64 Core::getAddress(VarWatch &w)
{
    int rc = 0;
    quint64 addr = 0;
    GdbResult gdbRes = GDB_ERROR;
    GdbCom& com = GdbCom::getInstance();
    Tree resultData;

    // Get the expression of the watch
    com.commandF(&resultData, "-var-info-path-expression %s", stringToCStr(w.getWatchId()));
    QString expr = resultData.getString("path_expr");
    if(expr.isEmpty())
    {
        rc = -1;
        errorMsg("Failed to get address of %s", stringToCStr(w.getName()));
        return 0;
    }

    // Evalute the expression
    gdbRes = com.commandF(&resultData, "-data-evaluate-expression &(%s)", stringToCStr(expr));
    if(gdbRes == GDB_DONE)
    {
        // Get address response
        QString varValue = resultData.getString("value");
        varValue = varValue.split(' ')[0];

        // Convert to integer
        bool ok = false;
        addr = varValue.toULongLong(&ok,0);
        if(!ok)
            rc = -2;
    }
    else if(gdbRes == GDB_ERROR)
    {
        rc = -1;
        errorMsg("Failed to get address of %s", stringToCStr(w.getName()));
    }
    if(rc)
        return 0;
    return addr;
}



/**
* @brief Checks if the target is running (or if it is stopped or finnished).
* @return true if the target is running.
*/ 
bool Core::isRunning()
{
    if(m_targetState == ICore::TARGET_STARTING ||
        m_targetState == ICore::TARGET_RUNNING)
    {
        return true;
    }
    return false;
}



    
