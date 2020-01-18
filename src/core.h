/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__CORE_H
#define FILE__CORE_H

#include <QList>
#include <QMap>
#include <QHash>
#include <QSocketNotifier>
#include <QObject>
#include <QVector>

#include "com.h"
#include "settings.h"


class Core;
class GdbCore;

struct ThreadInfo
{
    int m_id;             //!< The numeric id assigned to the thread by GDB.
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
    SourceFile() {};
    
    SourceFile(QString fullName, QString name) : m_name(name), m_fullName(fullName)  {};
    QString m_name;
    QString m_fullName;
};

/**
 * @brief A breakpoint.
 */
class BreakPoint
{
public:
    BreakPoint(int number) : m_number(number) { };

public:
    int m_number;
    QString m_fullname;
    int m_lineNo;
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
    quint64 getPointerAddress();
    void setPointerAddress(quint64 addr) { m_addressValid = true; m_address = addr; };
    bool hasPointerAddress() { return m_addressValid; };

    bool hasChildren() { return m_hasChildren; };
    

private:
    void clear();


private:

    QString m_name;
    QVariant m_data;
    quint64 m_address; //!< The address of data the variable points to.
    Type m_type;
    QString m_varType;
    bool m_hasChildren;
    bool m_addressValid;
};


class VarWatch
{
    public:
        VarWatch();
        VarWatch(QString watchId_, QString name_);
        
        QString getName() { return m_name; };
        QString getWatchId() { return m_watchId; };

        bool hasChildren();
        bool inScope() { return m_inScope;};
        QString getVarType() { return m_varType; };
        QString getValue(CoreVar::DispFormat fmt = CoreVar::FMT_NATIVE) { return m_var.getData(fmt); };

        CoreVar& getVar() { return m_var; };
        
        long long getPointerAddress() { return m_var.getPointerAddress(); };
        bool hasPointerAddress() { return m_var.hasPointerAddress(); };

    private:

        QString m_watchId;
        QString m_name;
        bool m_inScope;
        CoreVar m_var;
        QString m_varType;
        bool m_hasChildren;
        
        QString m_parentWatchId;

    friend Core;
    friend GdbCore;
    
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





class Core  : public QObject
{
private:
    Q_OBJECT
    
protected:

    Core(){};
    virtual ~Core(){};

public:

    static Core* getInstance();
    static void setInstance(Core *c);
    

    virtual int initPid(Settings *cfg, QString gdbPath, QString programPath, int pid) = 0;
    virtual int initLocal(Settings *cfg, QString gdbPath, QString programPath, QStringList argumentList) = 0;
    virtual int initCoreDump(Settings *cfg, QString gdbPath, QString programPath, QString coreDumpFile) = 0;
    virtual int initRemote(Settings *cfg, QString gdbPath, QString programPath, QString tcpHost, int tcpPort) = 0;
    virtual int evaluateExpression(QString expr, QString *data) = 0;
    
    virtual void setListener(ICore *inf) = 0;
    
    
    virtual int gdbSetBreakpointAtFunc(QString func) = 0;
    virtual void gdbNext() = 0;
    virtual void gdbStepIn() = 0;
    virtual void gdbStepOut() = 0;
    virtual void gdbContinue() = 0;
    virtual void gdbRun() = 0;
    virtual bool gdbGetFiles() = 0;

    virtual int getMemoryDepth() = 0;

    virtual int changeWatchVariable(QString variable, QString newValue) = 0;
    
    virtual QStringList getLocalVars() = 0;
    
    virtual quint64 getAddress(VarWatch &w) = 0;
    

    virtual int jump(QString filename, int lineNo) = 0;

    virtual int gdbSetBreakpoint(QString filename, int lineNo) = 0;
    virtual void gdbGetThreadList() = 0;
    virtual void getStackFrames() = 0;
    virtual void stop() = 0;
    virtual int gdbExpandVarWatchChildren(QString watchId) = 0;
    virtual int gdbGetMemory(quint64 addr, size_t count, QByteArray *data) = 0;
    
    virtual void selectThread(int threadId) = 0;
    virtual void selectFrame(int selectedFrameIdx) = 0;

    // Breakpoints
    virtual QList<BreakPoint*> getBreakPoints() = 0;
    virtual BreakPoint* findBreakPoint(QString fullPath, int lineNo) = 0;
    virtual BreakPoint* findBreakPointByNumber(int number) = 0;
    virtual void gdbRemoveBreakpoint(BreakPoint* bkpt) = 0;
    virtual void gdbRemoveAllBreakpoints() = 0;

    virtual QList<ThreadInfo> getThreadList() = 0;

    // Watch
    virtual VarWatch *getVarWatchInfo(QString watchId) = 0;
    virtual QList <VarWatch*> getWatchChildren(VarWatch &watch) = 0;
    virtual int gdbAddVarWatch(QString varName, VarWatch **watchPtr) = 0;
    virtual void gdbRemoveVarWatch(QString watchId) = 0;
    virtual QString gdbGetVarWatchName(QString watchId) = 0;

    
    virtual QVector <SourceFile*> getSourceFiles() = 0;
    
    virtual void writeTargetStdin(QString text) = 0;

    virtual bool isRunning() = 0;
};


#endif // FILE__CORE_H
