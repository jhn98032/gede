/*
 * Copyright (C) 2014-2015 Johan Henriksson.
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

#include "settings.h"

struct ThreadInfo
{
    int id;             //!< The numeric id assigned to the thread by GDB.
    QString m_name;     //!< Target-specific string identifying the thread.

    QString m_func; 
    QString m_details;  //!< Additional information about the thread provided by the target.
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
class CoreVarValue
{
public:
    CoreVarValue(QString str) { m_str = str; };

    QString toString();
    
    Tree *toTree();
    QString m_str;
};


class VarWatch
{
    public:
        VarWatch() { m_parent = NULL; };
        VarWatch(QString watchId_, QString name_)
        : watchId(watchId_),
            name(name_)
         { m_parent = NULL; };

        QString getName() { return name; };
        QString getWatchId() { return watchId; };

    private:

        QString watchId;
        QString name;

    public:
    
        QVector <VarWatch*> m_children;
        VarWatch* m_parent;
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
    virtual void ICore_onLocalVarReset() = 0;
    virtual void ICore_onLocalVarChanged(QString name, CoreVarValue value) = 0;
    virtual void ICore_onFrameVarReset() = 0;
    virtual void ICore_onFrameVarChanged(QString name, QString value) = 0;
    virtual void ICore_onWatchVarChanged(VarWatch &watch, QString value, bool hasChildren, bool inScope) = 0;
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
    virtual void ICore_onWatchVarChildAdded(VarWatch &watch, QString valueString, QString varType, bool hasChildren) = 0;
    
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
    int initLocal(Settings *cfg, QString gdbPath, QString programPath, QStringList argumentList);
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
    
public:
    int gdbSetBreakpointAtFunc(QString func);
    void gdbNext();
    void gdbStepIn();
    void gdbStepOut();
    void gdbContinue();
    void gdbRun();
    bool gdbGetFiles();
    int gdbAddVarWatch(QString varName, QString *varType, QString *value, QString *watchId, bool *hasChildren);
    void gdbRemoveVarWatch(QString watchId);
    QString gdbGetVarWatchName(QString watchId);
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

    QList<ThreadInfo> getThreadList();
    

    QVector <SourceFile*> getSourceFiles() { return m_sourceFiles; };


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
    QHash <QString, VarWatch> m_watchList;
    int m_varWatchLastId;
    bool m_isRemote; //!< True if "remote target" or false if it is a "local target".
    int m_ptsFd;
    bool m_scanSources; //!< True if the source filelist may have changed
    QSocketNotifier  *m_ptsListener;

};


#endif // FILE__CORE_H
