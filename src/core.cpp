#include "core.h"

#include "ini.h"
#include "util.h"
#include "log.h"

#include <QByteArray>
#include <QDebug>
#include <unistd.h>
#include <assert.h>
#include <signal.h>


/**
 * @brief Parses a variable assignment block.
 */
void parseVariableData(TreeNode *thisNode, QList<Token*> *tokenList)
{
    Token *token = tokenList->takeFirst();
    TreeNode *childNode = NULL;

    assert(token != NULL);

    if(token->getType() == Token::KEY_LEFT_BRACE)
    {
        
        do
        {
            // Get name
            QString name;
            Token *nameTok = tokenList->takeFirst();
            assert(nameTok != NULL);
            name = nameTok->getString();

            // Is it a "static varType" type?
            Token *extraNameTok = tokenList->first();
            if(extraNameTok->getType() == Token::VAR)
            {
                extraNameTok = tokenList->takeFirst();
                name += " " + extraNameTok->getString();
            }
        
            // Get equal sign
            Token *eqToken = tokenList->first();
            assert(eqToken != NULL);
            if(eqToken->getType() == Token::KEY_EQUAL)
            {
                eqToken = tokenList->takeFirst();

                // Create treenode
                childNode = new TreeNode;
                childNode->setName(nameTok->getString());
                thisNode->addChild(childNode);

                // Get variable data
                parseVariableData(childNode, tokenList);

                // End of the data
                token = tokenList->takeFirst();
            }
            else if(eqToken->getType() == Token::KEY_COMMA)
            {
                token = tokenList->isEmpty() ? NULL : tokenList->takeFirst();
            }
            else if(eqToken->getType() == Token::KEY_RIGHT_BRACE)
            {
                // Triggered by for example: "'{','<No data fields>', '}'"
                token = tokenList->isEmpty() ? NULL : tokenList->takeFirst();
            }
            else
            {
                errorMsg("Unknown token. Expected '=', Got:'%s'", stringToCStr(eqToken->getString()));

                // End of the data
                token = tokenList->isEmpty() ? NULL : tokenList->takeFirst();
            }
            
        }while(token != NULL && token->getType() == Token::KEY_COMMA);

        //
        if(token == NULL)
            errorMsg("Unexpected end of token");
        else if (token->getType() != Token::KEY_RIGHT_BRACE)
            errorMsg("Unknown token. Expected '}', Got:'%s'", stringToCStr(token->getString()));
    }
    else
    {
        QString valueStr = token->getString();

        // Was it only a address with data following the address? (Eg: '0x0001 "string"' )
        Token *nextTok = tokenList->first();
        if( nextTok->getType() == Token::VAR || nextTok->getType() == Token::C_STRING)
        {
            nextTok = tokenList->takeFirst();
            valueStr += " ";
            if(nextTok->getType() == Token::C_STRING)
                valueStr += "\"" + nextTok->getString() + "\"";
            else
                valueStr += nextTok->getString();
        }
        
        thisNode->setData(token->getString());
    }

    
}


QString CoreVarValue::toString()
{
    if(m_str.startsWith("{<"))
    {
        // Is it a message? Eg: "{<No data fields>}".
        return m_str.mid(1,m_str.length()-2);
    }
    
    return m_str;
}



/**
 * @brief Creates tokens from a single GDB output row.
 */
QList<Token*> Core::tokenizeVarString(QString str)
{
    enum { IDLE, BLOCK, BLOCK_COLON, STRING, VAR} state = IDLE;
    QList<Token*> list;
    Token *cur = NULL;
    QChar prevC = ' ';
    
    if(str.isEmpty())
        return list;

    for(int i = 0;i < str.size();i++)
    {
        QChar c = str[i];
        switch(state)
        {
            case IDLE:
            {
                if(c == '"')
                {
                    cur = new Token;
                    list.push_back(cur);
                    cur->type = Token::C_STRING;
                    state = STRING;
                }
                else if(c == '<')
                {
                    cur = new Token;
                    list.push_back(cur);
                    cur->type = Token::VAR;
                    cur->text += c;
                    state = BLOCK;
                }
                else if(c == '(')
                {
                    cur = new Token;
                    list.push_back(cur);
                    cur->type = Token::VAR;
                    cur->text += c;
                    state = BLOCK_COLON;
                }
                else if(c == '=' || c == '{' || c == '}' || c == ',' ||
                    c == '[' || c == ']' || c == '+' || c == '^' ||
                    c == '~' || c == '@' || c == '&' || c == '*')
                {
                    cur = new Token;
                    list.push_back(cur);
                    cur->text += c;
                    cur->type = Token::UNKNOWN;
                    if(c == '=')
                        cur->type = Token::KEY_EQUAL;
                    if(c == '{')
                        cur->type = Token::KEY_LEFT_BRACE;
                    if(c == '}')
                        cur->type = Token::KEY_RIGHT_BRACE;
                    if(c == '[')
                        cur->type = Token::KEY_LEFT_BAR;
                    if(c == ']')
                        cur->type = Token::KEY_RIGHT_BAR;
                    if(c == ',')
                        cur->type = Token::KEY_COMMA;
                    if(c == '^')
                        cur->type = Token::KEY_UP;
                    if(c == '+')
                        cur->type = Token::KEY_PLUS;
                    if(c == '~')
                        cur->type = Token::KEY_TILDE;
                    if(c == '@')
                        cur->type = Token::KEY_SNABEL;
                    if(c == '&')
                        cur->type = Token::KEY_AND;
                    if(c == '*')
                        cur->type = Token::KEY_STAR;
                    state = IDLE;
                }
                else if( c != ' ')
                {
                    cur = new Token;
                    list.push_back(cur);
                    cur->type = Token::VAR;
                    cur->text = c;
                    state = VAR;
                }
                
            };break;
            case STRING:
            {
                if(prevC != '\\' && c == '\\')
                {
                }
                else if(prevC == '\\')
                {
                    if(c == 'n')
                        cur->text += '\n';
                    else
                        cur->text += c;
                }
                else if(c == '"')
                    state = IDLE;
                else
                    cur->text += c;
            };break;
            case BLOCK_COLON:
            case BLOCK:
            {
                if(prevC != '\\' && c == '\\')
                {
                }
                else if(prevC == '\\')
                {
                    if(c == 'n')
                        cur->text += '\n';
                    else
                        cur->text += c;
                }
                else if((c == '>' && state == BLOCK) ||
                        (c == ')' && state == BLOCK_COLON))
                {
                    cur->text += c;
                    state = IDLE;
                }
                else
                    cur->text += c;
            };break;
            case VAR:
            {
                if(c == ' ' || c == '=' || c == ',' || c == '{' || c == '}')
                {
                    i--;
                    cur->text = cur->text.trimmed();
                    state = IDLE;
                }
                else
                    cur->text += c;
            };break;
            
        }
        prevC = c;
    }
    if(cur)
    {
        if(cur->type == Token::VAR)
            cur->text = cur->text.trimmed();
    }
    return list;
}


Tree* CoreVarValue::toTree()
{
    Tree *tree = NULL;
    
    QList<Token*> tokenList = Core::tokenizeVarString(m_str);

    QList<Token*> orgList = tokenList;
    Token* token;

        token = tokenList.front();

    if(tokenList.size() > 1)
    {
        if(token->getType() == Token::KEY_LEFT_BRACE || token->getType() == Token::KEY_SNABEL)
        {
            TreeNode *rootNode;
            tree = new Tree;
            rootNode = tree->getRoot();

            // Is it a "@0x2202:" type?
            if(token->getType() == Token::KEY_SNABEL)
            {
                QString addrStr;
                Token *extraNameTok;
                token = tokenList.takeFirst();
                
                extraNameTok = tokenList.takeFirst();
                if(extraNameTok)
                    addrStr += " " + extraNameTok->getString();

                TreeNode *childNode = new TreeNode;
                childNode->setName(addrStr);
                rootNode->addChild(childNode);
                rootNode = childNode;

                m_str = addrStr;
            }


            parseVariableData(rootNode, &tokenList);


        }
        else
        {
            errorMsg("Unknown token in beginning of data list. Expected '{', Got:'%s' ", stringToCStr(token->getString()));
        }
        
    }
    else
    {
        //errorMsg("Unknown token ('%s')", stringToCStr(token->getString()));
    }

    for(int i = 0;i < orgList.size();i++)
    {
        Token *tok = orgList[i];
        delete tok;
    }
    return tree;
}



Core::Core()
 : m_inf(NULL)
    ,m_selectedThreadId(0)
    ,m_targetState(ICore::TARGET_STOPPED)
    ,m_lastTargetState(ICore::TARGET_FINISHED)
    ,m_pid(0)
    ,m_currentFrameIdx(-1)
    ,m_varWatchLastId(10)
{
    
    Com& com = Com::getInstance();
    com.setListener(this);

    m_ptsFd = getpt();
   
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
    delete m_ptsListener;
    
    Com& com = Com::getInstance();
    com.setListener(NULL);

    close(m_ptsFd);
}


int Core::initLocal(Settings *cfg, QString gdbPath, QString programPath, QStringList argumentList)
{
    Com& com = Com::getInstance();
    Tree resultData;
    
    if(com.init(gdbPath))
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
    
    gdbSetBreakpointAtFunc("main");

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


    gdbRun();

    
    return 0;
}

int Core::initRemote(Settings *cfg, QString gdbPath, QString programPath, QString tcpHost, int tcpPort)
{
    Com& com = Com::getInstance();
    Tree resultData;
    
    if(com.init(gdbPath))
    {
        errorMsg("Failed to start gdb");
        return -1;
    }

    com.commandF(&resultData, "-target-select remote %s:%d", stringToCStr(tcpHost), tcpPort); 

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

        com.commandF(&resultData, "-target-download");
    }
    

    gdbSetBreakpointAtFunc("main");
    
    gdbGetFiles();

    gdbContinue();

    return 0;
}


void Core::onGdbOutput(int socketFd)
{
    Q_UNUSED(socketFd);
    char buff[128];
    int n =  read(m_ptsFd, buff, sizeof(buff)-1);
    if(n > 0)
    {
        buff[n] = '\0';
    }
    m_inf->ICore_onTargetOutput(buff);
}

void Core::gdbGetFiles()
{
    Com& com = Com::getInstance();
    Tree resultData;
    QMap<QString, bool> fileLookup;
    
    com.command(&resultData, "-file-list-exec-source-files");


    for(int m = 0;m < m_sourceFiles.size();m++)
    {
        SourceFile *sourceFile = m_sourceFiles[m];
        delete sourceFile;
    }
    m_sourceFiles.clear();


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


                SourceFile *sourceFile = NULL;
                // Already added this file?
                if(!fileLookup.contains(fullname) && !name.contains("<built-in>"))
                {
                    fileLookup[fullname] = true;
                    
                    sourceFile = new SourceFile; 

                    sourceFile->name =name;
                    sourceFile->fullName = fullname;

                    m_sourceFiles.append(sourceFile);
                }
            }
        }
    }
}


void Core::gdbSetBreakpointAtFunc(QString func)
{
    Com& com = Com::getInstance();
    Tree resultData;

    com.commandF(&resultData, "-break-insert %s", stringToCStr(func));
}

void Core::gdbRun()
{
    Com& com = Com::getInstance();
    Tree resultData;

    if(m_targetState == ICore::TARGET_RUNNING)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is currently running");
        return;
    }

    m_pid = 0;
    com.commandF(&resultData, "-exec-run");

}


/**
 * @brief  Resumes the execution until a breakpoint is encountered, or until the program exits.
 */
void Core::gdbContinue()
{
    Com& com = Com::getInstance();
    Tree resultData;

    if(m_targetState == ICore::TARGET_RUNNING)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is currently running");
        return;
    }

    com.commandF(&resultData, "-exec-continue");

}

void Core::stop()
{

    if(m_targetState != ICore::TARGET_RUNNING)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is not running");
        return;
    }
    if(m_pid != 0)
        kill(m_pid, SIGINT);
    else
        errorMsg("Failed to stop since PID not known");
}

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

    if(m_targetState != ICore::TARGET_STOPPED)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is not stopped");
        return;
    }

        
    com.commandF(&resultData, "-exec-step");
    com.commandF(&resultData, "-var-update --all-values *");

}


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


int Core::gdbAddVarWatch(QString varName, QString *varType, QString *value, QString *watchId_)
{
    Com& com = Com::getInstance();
    Tree resultData;
    QString watchId;
    GdbResult res;
    int rc = 0;

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


    // debugMsg("%s = %s = %s\n", stringToCStr(varName2),stringToCStr(varValue2), stringToCStr(varType2));

    VarWatch w;
    w.name = varName;
    w.watchId = watchId;
    m_watchList[watchId] = w;
    
    *varType = varType2;
    *value = varValue2;
    }

    *watchId_ = watchId;

    return rc;
}


void Core::gdbExpandVarWatchChildren(QString watchId)
{
    int res;
    Tree resultData;
    Com& com = Com::getInstance();

    // Get the variable name
//    QString varName = m_watchList[watchId].name;

    // Request its children
    res = com.commandF(&resultData, "-var-list-children --all-values %s", stringToCStr(watchId));

    if(res != 0)
    {
        return;
    }

        
    // Enumerate the children
    QStringList childList = resultData.getChildList("children");
    for(int i = 0;i < childList.size();i++)
    {
        // Get name and value
        QString treePath;
        treePath.sprintf("children/#%d",i+1);
        QString childName = resultData.getString(treePath + "/name");
        QString childExp = resultData.getString(treePath + "/exp");
        QString childValue = resultData.getString(treePath + "/value");
        QString childType = resultData.getString(treePath + "/type");
        m_inf->ICore_onWatchVarExpanded(childName, childExp, childValue, childType);
    }
    
}


QString Core::gdbGetVarWatchName(QString watchId)
{
    return m_watchList[watchId].name;
}

void Core::gdbRemoveVarWatch(QString watchId)
{
    Com& com = Com::getInstance();
    Tree resultData;
    
    assert(watchId != "");
    
    
    QMap <QString, VarWatch>::iterator pos = m_watchList.find(watchId);
    if(pos == m_watchList.end())
    {
        assert(0);
    }
    else
    {
        m_watchList.erase(pos);
    
        com.commandF(&resultData, "-var-delete %s", stringToCStr(watchId));
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


        QString p = tree.getString("frame/fullname");
        int lineNo = tree.getInt("frame/line");

        // Get the reason
        QString reasonString = tree.getString("reason");
        ICore::StopReason  reason;
        if(reasonString.isEmpty())
            reason = ICore::UNKNOWN;
        else
            reason = parseReasonString(reasonString);

        if(reason == ICore::EXITED_NORMALLY)
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
        int threadId = threadIdStr.toInt();
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

    if(m_targetState == ICore::TARGET_RUNNING)
    {
        if(m_inf)
            m_inf->ICore_onMessage("Program is currently running");
        return;
    }

    
    com.commandF(&resultData, "-thread-info");    


}


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
                QString watchId = tree.getString(path);
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

                    CoreVarValue val(varData);
                    m_inf->ICore_onLocalVarChanged(varName, val);
                    
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

void Core::gdbSetBreakpoint(QString filename, int lineNo)
{
    Com& com = Com::getInstance();
    Tree resultData;
    
    assert(filename != "");
    
    com.commandF(&resultData, "-break-insert %s:%d", stringToCStr(filename), lineNo);

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

    if(m_targetState == ICore::TARGET_RUNNING)
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



    



