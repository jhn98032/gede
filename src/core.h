/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__CORE_H
#define FILE__CORE_H

#include "com.h"
#include <QList>
#include <QMap>
#include <QHash>
#include <QSocketNotifier>
#include <QObject>
#include <QVector>


#include "settings.h"

class Core;

struct ThreadInfo
{
    int id;             //!< The numeric id assigned to the thread by GDB.
    QString m_name;     //!< Target-specific string identifying the thread.

    QString m_func; //!< The name of the function (Eg: "func"). 
    QString m_details;  //!< Additional information about the thread provided by the target.
};


struct StackFrameEntry
{
    public:
        QString m_functionName; //!< Eg: "main".
        int m_line; //!< The line number. Eg: 1.
        QString m_sourcePath; //!< The full path of the source file. Eg: "/test/file.c".
};


class SourceFile
{
public:
    QString name;
    QString fullName;
};

/**
 * @brief A breakpoint.
 */
class BreakPoint
{
public:
    BreakPoint(int number) : m_number(number) { };


    int m_number;
    QString fullname;
    int lineNo;
    QString m_funcName;
    unsigned long long m_addr;
    
private:
    BreakPoint(){};
};


/**
 * @brief The value of a variable.
 */
class CoreVar
{
public:
    CoreVar();
    CoreVar(QString name);
    virtual ~CoreVar();

    typedef enum { FMT_HEX = 1, FMT_DEC, FMT_BIN, FMT_CHAR, FMT_NATIVE } DispFormat;
    typedef enum { TYPE_HEX_INT = 1, TYPE_DEC_INT, TYPE_FLOAT, TYPE_STRING, TYPE_ENUM, TYPE_ERROR_MSG, TYPE_CHAR, TYPE_UNKNOWN }
    Type;

    QString getName() const { return m_name; };
    QString getData(DispFormat fmt) const;

    void setVarType(QString varType) { m_varType = varType; };
    QString getVarType() { return m_varType; };
    void setData(Type type, QVariant data);
    long long getAddress();
    void setAddress(long long addr) { m_address = addr; };


    bool hasChildren() { return m_hasChildren; };
    
    void valueFromGdbString(QString data);

private:
    void clear();


private:

    QString m_name;
    QVariant m_data;
    long long m_address;
    Type m_type;
    QString m_varType;
    bool m_hasChildren;
};


class VarWatch
{
    public:
        VarWatch();
        VarWatch(QString watchId_, QString name_);
        
        QString getName() { return name; };
        QString getWatchId() { return watchId; };

        bool hasChildren();
        bool inScope() { return m_inScope;};
        QString getVarType() { return m_varType; };
        QString getValue(CoreVar::DispFormat fmt = CoreVar::FMT_NATIVE) { return m_var.getData(fmt); };

        void setValue(QString value);
        long long getAddress() { return m_var.getAddress(); };
    
    private:

        QString watchId;
        QString name;
        bool m_inScope;
        CoreVar m_var;
        QString m_varType;
        bool m_hasChildren;
        
        QString m_parentWatchId;

    friend Core;
};



class ICore
{
    public:

    enum TargetState 
    {
        TARGET_STOPPED,
        TARGET_STARTING,
        TARGET_RUNNING,
        TARGET_FINISHED 
    }; 
    
    enum StopReason
    {
        UNKNOWN,
        END_STEPPING_RANGE,
        BREAKPOINT_HIT,
        SIGNAL_RECEIVED,
        EXITED_NORMALLY,
        FUNCTION_FINISHED,
        EXITED
    };
    
    enum SignalType
    {
        SIGINT,
        SIGTERM,
        SIGKILL,
        SIGUNKNOWN
    };
    virtual void ICore_onStopped(StopReason reason, QString path, int lineNo) = 0;
    virtual void ICore_onStateChanged(TargetState state) = 0;
    virtual void ICore_onSignalReceived(QString signalName) = 0;
    virtual void ICore_onLocalVarChanged(QStringList varNames) = 0;
    virtual void ICore_onFrameVarReset() = 0;
    virtual void ICore_onFrameVarChanged(QString name, QString value) = 0;
    virtual void ICore_onWatchVarChanged(VarWatch &watch) = 0;
    virtual void ICore_onConsoleStream(QString text) = 0;
    virtual void ICore_onBreakpointsChanged() = 0;
    virtual void ICore_onThreadListChanged() = 0;
    virtual void ICore_onCurrentThreadChanged(int threadId) = 0;
    virtual void ICore_onStackFrameChange(QList<StackFrameEntry> stackFrameList) = 0;
    virtual void ICore_onMessage(QString message) = 0;
    virtual void ICore_onTargetOutput(QString message) = 0;
    virtual void ICore_onCurrentFrameChanged(int frameIdx) = 0;
    virtual void ICore_onSourceFileListChanged() = 0;

    /**
     * @brief Called when a new child item has been added for a watched item.
     * @param watchId    The watchId of the new child.
     * @param name       The name of the child.
     * @param valueString  The value of the child.
     */
    virtual void ICore_onWatchVarChildAdded(VarWatch &watch) = 0;
    
};





class Core : public ComListener
{
private:
    Q_OBJECT;
    
private:

    Core();
    ~Core();

public:
    

    static Core& getInstance();
    int initPid(Settings *cfg, QString gdbPath, QString programPath, int pid);
    int initLocal(Settings *cfg, QString gdbPath, QString programPath, QStringList argumentList);
    int initCoreDump(Settings *cfg, QString gdbPath, QString programPath, QString coreDumpFile);
    int initRemote(Settings *cfg, QString gdbPath, QString programPath, QString tcpHost, int tcpPort);
    
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
    void detectMemoryDepth();
    static int openPseudoTerminal();
    void ensureStopped();
    
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
    
    QStringList getLocalVars() { return m_localVars; };


    int gdbSetBreakpoint(QString filename, int lineNo);
    void gdbGetThreadList();
    void getStackFrames();
    void stop();
    int gdbExpandVarWatchChildren(QString watchId);
    int gdbGetMemory(uint64_t addr, size_t count, QByteArray *data);
    
    void selectThread(int threadId);
    void selectFrame(int selectedFrameIdx);

    // Breakpoints
    QList<BreakPoint*> getBreakPoints() { return m_breakpoints; };
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

    
    QVector <SourceFile*> getSourceFiles() { return m_sourceFiles; };

    void writeTargetStdin(QString text);
    
private slots:
        void onGdbOutput(int socketNr);

private:
    ICore *m_inf;
    QList<BreakPoint*> m_breakpoints;
    QVector <SourceFile*> m_sourceFiles;
    QMap <int, ThreadInfo> m_threadList;
    int m_selectedThreadId;
    ICore::TargetState m_targetState;
    ICore::TargetState m_lastTargetState;
    int m_pid;
    int m_currentFrameIdx;
    QList <VarWatch*> m_watchList;
    int m_varWatchLastId;
    bool m_isRemote; //!< True if "remote target" or false if it is a "local target".
    int m_ptsFd;
    bool m_scanSources; //!< True if the source filelist may have changed
    QSocketNotifier  *m_ptsListener;

    QStringList m_localVars;
    int m_memDepth; //!< The memory depth. (Either 64 or 32).
};


#endif // FILE__CORE_H
