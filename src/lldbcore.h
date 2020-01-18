#ifndef FILE__LLDBCORE_H
#define FILE__LLDBCORE_H

#include <QThread>

#include "core.h"

class LldbThread;

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
