#ifndef FILE__CORE_H
#define FILE__CORE_H

#include "com.h"
#include <QList>
#include <QMap>


struct ThreadInfo
{
    int id;
    QString m_name;

    QString m_func;
};


struct StackFrameEntry
{
    public:
        QString m_functionName; //!< Eg: "main".
        int m_line; //!< Eg: 1.
        QString m_sourcePath; //!< Eg: "/test/file.c".
};


class SourceFile
{
public:
    QString name;
    QString fullName;
};

class BreakPoint
{
public:
    BreakPoint(int number) : m_number(number) { };


    int m_number;
    QString fullname;
    int lineno;
    QString m_funcName;
    unsigned long long m_addr;
    
private:
    BreakPoint(){};
};


class ICore
{
    public:

    enum StopReason
    {
        UNKNOWN,
        END_STEPPING_RANGE,
        BREAKPOINT_HIT,
        SIGNAL_RECEIVED,
        EXITED_NORMALLY,
        EXITED
    };

    enum SignalType
    {
        SIGINT,
        SIGTERM,
        SIGKILL,
        SIGUNKNOWN
    };
    virtual void ICore_onStopped(StopReason reason, QString path, int lineno) = 0;
    virtual void ICore_onSignalReceived(QString signalName) = 0;
    virtual void ICore_onLocalVarReset() = 0;
    virtual void ICore_onLocalVarChanged(QString name, QString value) = 0;
    virtual void ICore_onFrameVarReset() = 0;
    virtual void ICore_onFrameVarChanged(QString name, QString value) = 0;
    virtual void ICore_onWatchVarChanged(int watchId, QString name, QString value) = 0;
    virtual void ICore_onConsoleStream(QString text) = 0;
    virtual void ICore_onBreakpointsChanged() = 0;
    virtual void ICore_onThreadListChanged() = 0;
    virtual void ICore_onCurrentThreadChanged(int threadId) = 0;
    virtual void ICore_onStackFrameChange(QList<StackFrameEntry> stackFrameList) = 0;
    virtual void ICore_onMessage(QString message) = 0;
    virtual void ICore_onCurrentFrameChanged(int frameIdx) = 0;
    
};

struct VarWatch
{
    QString name;
    int id;
};
    

class Core : public ComListener
{
private:

    Core();
    ~Core();

public:

    static Core& getInstance();
    int init(QStringList argumentList);

    void setListener(ICore *inf) { m_inf = inf; };

private:
    
     void onNotifyAsyncOut(Tree &tree, AsyncClass ac);
     void onExecAsyncOut(Tree &tree, AsyncClass ac);
     void onResult(Tree &tree);
     void onStatusAsyncOut(Tree &tree, AsyncClass ac);
     void onConsoleStreamOutput(QString str);
     void onTargetStreamOutput(QString str);
     void onLogStreamOutput(QString str);

    void dispatchBreakpointTree(Tree &tree);
    static ICore::StopReason parseReasonString(QString string);
    
public:
    void gdbInsertBreakPoint(QString func);
    void gdbNext();
    void gdbStepIn();
    void gdbContinue();
    void gdbRun();
    void gdbGetFiles();
    int gdbAddVarWatch(QString varName, QString *varType, QString *value, int *watchId);
    void gdbRemoveVarWatch(int vatchId);
    QString gdbGetVarWatchName(int vatchId);
    void gdbSetBreakpoint(QString filename, int lineno);
    void gdbGetThreadList();
    void getStackFrames();
    void stop();
    
    void selectThread(int threadId);
    void selectFrame(int selectedFrameIdx);

    // Breakpoints
    QList<BreakPoint*> getBreakPoints() { return m_breakpoints; };
    BreakPoint* findBreakPoint(QString fullPath, int lineno);
    BreakPoint* findBreakPointByNumber(int number);
    void gdbRemoveBreakpoint(BreakPoint* bkpt);

    QList<ThreadInfo> getThreadList();
    

    QVector <SourceFile*> getSourceFiles() { return m_sourceFiles; };
private:
    ICore *m_inf;
    QList<BreakPoint*> m_breakpoints;
    QVector <SourceFile*> m_sourceFiles;
    QMap <int, ThreadInfo> m_threadList;
    int m_selectedThreadId;
    enum {TARGET_STOPPED, TARGET_RUNNING,TARGET_FINISHED } m_targetState;
    int m_pid;
    int m_currentFrameIdx;
    QMap <int, VarWatch> m_watchList;
    int m_varWatchLastId;
};


#endif // FILE__CORE_H
