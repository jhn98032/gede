#include "lldbcore.h"

#include <lldb/API/SBDebugger.h>
#include <lldb/API/SBThread.h>
#include <lldb/API/SBTarget.h>
#include <lldb/API/SBLaunchInfo.h>
#include <lldb/API/SBError.h>
#include <lldb/API/SBProcess.h>
#include <lldb/API/SBListener.h>
#include <lldb/API/SBEvent.h>

#define ENABLE_DEBUGMSG

#include "log.h"

using namespace lldb;



enum
{
    LLDBCTRL_STEPOVER = (1<<0),
    LLDBCTRL_EVENT_ALL  = UINT32_MAX
};

const char *stateToDesc(StateType process_state)
 {
     const char *desc = "?";
      switch (process_state)
      {
      case eStateStopped: desc = "stopped";break;
      case eStateRunning:desc = "running";break;
      case eStateExited:desc = "exited";break;
      case eStateDetached:desc = "detached";break;
      case eStateLaunching:desc = "launching";break;
      case eStateInvalid: desc = "invalid";break;
      default:;break;
  }
return desc;
}


LldbThread::LldbThread()
 : gui_event_broadcaster("gui-events")
{

}
    
LldbThread::~LldbThread()
{

}

QString fullPath(SBFileSpec spec)
{
    QString path;
    path.sprintf("%s/%s", spec.GetDirectory(), spec.GetFilename());
    return path;
}


QVector<SourceFile*> LldbThread::detectSourceFiles()
{
    debugMsg("%s()", __func__);
    QVector<SourceFile*> list;
    for(uint32_t modIdx = 0;modIdx < target.GetNumModules();modIdx++)
    {
        lldb::SBModule mod = target.GetModuleAtIndex (modIdx);
        for(uint32_t cidx = 0;cidx < mod.GetNumCompileUnits();cidx++)
        {
            SBCompileUnit cu = mod.GetCompileUnitAtIndex (cidx);
            SBFileSpec spec = cu.GetFileSpec();
            QString path;
            path.sprintf("%s/%s", spec.GetDirectory(), spec.GetFilename());
            // debugMsg("d:'%s'", qPrintable(path));
            SourceFile *sf = new SourceFile(path, QString(spec.GetFilename()));
            list.append(sf);
        }
    
    }
    return list;
}

    
void LldbThread::run()
{

    SBEvent event;
    const uint32_t infinite_timeout = UINT32_MAX;
    StateType process_state = eStateInvalid;
    bool done = false;
    while (!done)
    {
        printf("listening\n");

    printf("state4:%s\n", stateToDesc(process.GetState()));

      if (listener.WaitForEvent(infinite_timeout, event))
      {
          debugMsg("s : %d", event.GetType());
          if(event.GetType() & SBProcess::eBroadcastBitSTDOUT)
          {
              char stdio_buffer[128];
              process.GetSTDOUT(stdio_buffer, sizeof(stdio_buffer));
              debugMsg("stdout: >%s<", stdio_buffer);
          }
        if (SBProcess::EventIsProcessEvent (event))
        {
            
          process_state = SBProcess::GetStateFromEvent (event);
    printf(">%s\n", stateToDesc(process_state));

        emit onEvent(process_state);

         
        }
        else if (event.BroadcasterMatchesRef(gui_event_broadcaster))
        {
            debugMsg("brd");
            int type = event.GetType();
          switch (type)
          {
            case LLDBCTRL_STEPOVER:
            {
                debugMsg("stepping over");
                       SBThread thisThread = process.GetSelectedThread();
                thisThread.StepOver();
                };
                default:;break;
                }
        }
        
      }
    }



    printf("--------\n");

}
    
void LldbThread::sendStepOver()
{
    gui_event_broadcaster.BroadcastEventByType(LLDBCTRL_STEPOVER);
}

      
int LldbThread::test(QString exeName)
{
        char *exeNameCStr = strdup(qPrintable(exeName));

setenv("LLDB_DEBUGSERVER_PATH","/usr/bin/lldb-server", 1);

    SBDebugger::Initialize();



    SBDebugger debugger;


    debugger = SBDebugger::Create();
    if(!debugger.IsValid())
        printf("SBDebugger failed\r\n");


debugger.SetAsync (true);



target = debugger.CreateTargetWithFileAndArch (exeNameCStr, NULL);//LLDB_ARCH_DEFAULT);

if(!target.IsValid())
    printf("target failed\r\n");
else
printf("target ok\r\n");

const char *argv[] = { exeNameCStr, "-l", "-A", "-F", nullptr };
SBLaunchInfo launch_info(argv);
launch_info.SetWorkingDirectory(".");


SBBreakpoint main_bp = target.BreakpointCreateByName ("main", target.GetExecutable().GetFilename());
printf("main_bp: %s\n", main_bp.IsValid() ? "valid" : "invalid");


listener = debugger.GetListener();

listener.StartListeningForEvents(gui_event_broadcaster, LLDBCTRL_EVENT_ALL);
    
SBError error3;

process = target.Launch (launch_info, error3);
/*
const char *env[] = {"", nullptr};
SBProcess process = target.Launch(listener, argv, env, nullptr, nullptr,
                                  nullptr, exeName,
                                  0, true, error3);
  */                                

printf("process: %s (%s)\n", process.IsValid() ? "valid" : "invalid", error3.GetCString());


process.GetBroadcaster().AddListener(listener,
        SBProcess::eBroadcastBitStateChanged | SBProcess::eBroadcastBitSTDOUT);

// process.Continue();

 StateType state4 = process.GetState(); // is stopped
printf("state4:%s\n", stateToDesc(state4));
    

free(exeNameCStr);


return 0;
}



LldbCore::LldbCore()
{   
    m_thread = new LldbThread();

    connect(m_thread, SIGNAL(onEvent(int)), this, SLOT(onEvent(int)));
}


LldbCore::~LldbCore()
{
    m_thread->quit();
    delete m_thread;
}


void LldbCore::onEvent(int evt)
{
int process_state = evt;

debugMsg("%s(%d)", __func__, evt);

 switch (process_state)
          {
          case eStateStopped:
          {
                //SBThread thisThread = process.GetSelectedThread();
                    SBThread thisThread = m_thread->process.GetSelectedThread();
                    if(!thisThread.IsValid())
                        debugMsg("Invalid thread_");
                    SBFrame frame = thisThread.GetSelectedFrame();
                    int line = frame.GetLineEntry().GetLine();
                    debugMsg("stop at %s %d", frame.GetFunctionName(), line);

                    QString filename = fullPath(frame.GetCompileUnit().GetFileSpec());
                    m_inf->ICore_onStopped(ICore::UNKNOWN, filename, line);
          };break;
          case eStateRunning:
          case eStateExited:
          case eStateDetached:
          default:;break;
          }
          
}

int LldbCore::initPid(Settings *cfg, QString gdbPath, QString programPath, int pid)
{
    Q_UNUSED(cfg);
    Q_UNUSED(gdbPath);
    Q_UNUSED(programPath);
    Q_UNUSED(pid);
    return 0;
}

int LldbCore::initLocal(Settings *cfg, QString gdbPath, QString programPath, QStringList argumentList)
{
    Q_UNUSED(cfg);
    Q_UNUSED(gdbPath);
    Q_UNUSED(programPath);
    Q_UNUSED(argumentList);


    m_thread->test(programPath);

 m_sourceFiles = m_thread->detectSourceFiles();
m_thread->start();

    
    return 0;
}

int LldbCore::initCoreDump(Settings *cfg, QString gdbPath, QString programPath, QString coreDumpFile)
{
    Q_UNUSED(cfg);
    Q_UNUSED(gdbPath);
    Q_UNUSED(programPath);
    Q_UNUSED(coreDumpFile);
    return 0;
}

int LldbCore::initRemote(Settings *cfg, QString gdbPath, QString programPath, QString tcpHost, int tcpPort)
{
    Q_UNUSED(cfg);

    Q_UNUSED(gdbPath);
    Q_UNUSED(programPath);
    Q_UNUSED(tcpHost);
    Q_UNUSED(tcpPort);

    return 0;
}

int LldbCore::evaluateExpression(QString expr, QString *data)
{
    Q_UNUSED(expr);
    Q_UNUSED(data);

    return 0;
}

void LldbCore::setListener(ICore *inf)
{
    m_inf = inf;
}


int LldbCore::gdbSetBreakpointAtFunc(QString func)
{
    Q_UNUSED(func);
    return 0;
}

void LldbCore::gdbNext()
{
    debugMsg("LldbCore::%s()", __func__); 
    m_thread->sendStepOver();
}

void LldbCore::gdbStepIn()
{
}

void LldbCore::gdbStepOut()

{
}
void LldbCore::gdbContinue()
{
}

void LldbCore::gdbRun()
{
    
}

bool LldbCore::gdbGetFiles()
{
    return false;
}


int LldbCore::getMemoryDepth()
{
    return 0;
}


int LldbCore::changeWatchVariable(QString variable, QString newValue)
{
        Q_UNUSED(variable);
    Q_UNUSED(newValue);

    return 0;
}


QStringList LldbCore::getLocalVars()
{
    QStringList lst;
    return lst;
}


quint64 LldbCore::getAddress(VarWatch &w)
{
    Q_UNUSED(w);
    
    return 0;
}



int LldbCore::jump(QString filename, int lineNo)
{
    Q_UNUSED(filename);
    Q_UNUSED(lineNo);
    return 0;
}


int LldbCore::gdbSetBreakpoint(QString filename, int lineNo)
{
    Q_UNUSED(filename);
    Q_UNUSED(lineNo);
    return 0;
}

void LldbCore::gdbGetThreadList()
{
}

void LldbCore::getStackFrames()
{
}

void LldbCore::stop()
{
}

int LldbCore::gdbExpandVarWatchChildren(QString watchId)
{
    Q_UNUSED(watchId);
    return 0;
}

int LldbCore::gdbGetMemory(quint64 addr, size_t count, QByteArray *data)
{
    Q_UNUSED(addr);
    Q_UNUSED(count);
    Q_UNUSED(data);

    return 0;
}


void LldbCore::selectThread(int threadId)
{
    Q_UNUSED(threadId);
}

void LldbCore::selectFrame(int selectedFrameIdx)
{
    Q_UNUSED(selectedFrameIdx);
}


// Breakpoints
QList<BreakPoint*> LldbCore::getBreakPoints()
{
    QList<BreakPoint*> lst;
    return lst;
}

BreakPoint* LldbCore::findBreakPoint(QString fullPath, int lineNo)
{
    Q_UNUSED(fullPath);
    Q_UNUSED(lineNo);
    return NULL;
}

BreakPoint* LldbCore::findBreakPointByNumber(int number)
{
    Q_UNUSED(number);
    return NULL;
}

void LldbCore::gdbRemoveBreakpoint(BreakPoint* bkpt)
{
    Q_UNUSED(bkpt);
}

void LldbCore::gdbRemoveAllBreakpoints()
{
}


QList<ThreadInfo> LldbCore::getThreadList()
{
    QList<ThreadInfo> list;
    return list;
}


// Watch
VarWatch *LldbCore::getVarWatchInfo(QString watchId)
{
    Q_UNUSED(watchId);
    return NULL;
}

QList <VarWatch*> LldbCore::getWatchChildren(VarWatch &watch)
{
    Q_UNUSED(watch);
    QList <VarWatch*> lst;
    return lst;
}

int LldbCore::gdbAddVarWatch(QString varName, VarWatch **watchPtr)
{
    Q_UNUSED(varName);
    Q_UNUSED(watchPtr);
    return 0;
}

void LldbCore::gdbRemoveVarWatch(QString watchId)
{
    Q_UNUSED(watchId);
}

QString LldbCore::gdbGetVarWatchName(QString watchId)
{
    Q_UNUSED(watchId);
    QString str;
    return str;
}


QVector <SourceFile*> LldbCore::getSourceFiles()
{
    return m_sourceFiles;
}


void LldbCore::writeTargetStdin(QString text)
{
    Q_UNUSED(text);
}

bool LldbCore::isRunning()
{
    return false;
}

