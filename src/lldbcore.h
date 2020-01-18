#ifndef FILE__LLDBCORE_H
#define FILE__LLDBCORE_H

#include <QThread>

#include "core.h"

#include <lldb/API/SBDebugger.h>
#include <lldb/API/SBThread.h>
#include <lldb/API/SBTarget.h>
#include <lldb/API/SBLaunchInfo.h>
#include <lldb/API/SBError.h>
#include <lldb/API/SBProcess.h>
#include <lldb/API/SBListener.h>
#include <lldb/API/SBEvent.h>


class LldbThread;


class LldbThread : public QThread
{
    Q_OBJECT
public:

    LldbThread();
    virtual ~LldbThread();

    void run();

  int test(QString exeNameStr);
    QVector<SourceFile*> detectSourceFiles();
    void sendStepOver();

signals:
        void onEvent(int evt);

public:    
lldb::SBBroadcaster gui_event_broadcaster;
lldb::SBTarget target;
lldb::SBListener listener;
lldb::SBProcess process;
};


class LldbCore : public Core
{
private:
    Q_OBJECT
    
public:

    LldbCore();
    ~LldbCore();

public:
    

    int initPid(Settings *cfg, QString gdbPath, QString programPath, int pid);
    int initLocal(Settings *cfg, QString gdbPath, QString programPath, QStringList argumentList);
    int initCoreDump(Settings *cfg, QString gdbPath, QString programPath, QString coreDumpFile);
    int initRemote(Settings *cfg, QString gdbPath, QString programPath, QString tcpHost, int tcpPort);
    int evaluateExpression(QString expr, QString *data);
    
    void setListener(ICore *inf);
    
private slots:
    void onEvent(int evt);
    
public:
    int gdbSetBreakpointAtFunc(QString func);
    void gdbNext();
    void gdbStepIn();
    void gdbStepOut();
    void gdbContinue();
    void gdbRun();
    bool gdbGetFiles();

    int getMemoryDepth();

    int changeWatchVariable(QString variable, QString newValue);
    
    QStringList getLocalVars();
    
    quint64 getAddress(VarWatch &w);
    

    int jump(QString filename, int lineNo);

    int gdbSetBreakpoint(QString filename, int lineNo);
    void gdbGetThreadList();
    void getStackFrames();
    void stop();
    int gdbExpandVarWatchChildren(QString watchId);
    int gdbGetMemory(quint64 addr, size_t count, QByteArray *data);
    
    void selectThread(int threadId);
    void selectFrame(int selectedFrameIdx);

    // Breakpoints
    QList<BreakPoint*> getBreakPoints();
    BreakPoint* findBreakPoint(QString fullPath, int lineNo);
    BreakPoint* findBreakPointByNumber(int number);
    void gdbRemoveBreakpoint(BreakPoint* bkpt);
    void gdbRemoveAllBreakpoints();

    QList<ThreadInfo> getThreadList();

    // Watch
    VarWatch *getVarWatchInfo(QString watchId);
    QList <VarWatch*> getWatchChildren(VarWatch &watch);
    int gdbAddVarWatch(QString varName, VarWatch **watchPtr);
    void gdbRemoveVarWatch(QString watchId);
    QString gdbGetVarWatchName(QString watchId);

    
    QVector <SourceFile*> getSourceFiles();
    
    void writeTargetStdin(QString text);

    bool isRunning();
    
private slots:

private:
    ICore *m_inf;
    LldbThread *m_thread;
    QVector<SourceFile*> m_sourceFiles;
};



#endif // FILE__LLDBCORE_H
