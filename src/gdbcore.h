/*
 * Copyright (C) 2014-2020 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__GDBCORE_H
#define FILE__GDBCORE_H

#include <QList>
#include <QMap>
#include <QHash>
#include <QSocketNotifier>
#include <QObject>
#include <QVector>

#include "core.h"
#include "com.h"
#include "settings.h"



class GdbCore : public Core, public GdbComListener
{
private:
    Q_OBJECT
    
public:

    GdbCore();
    ~GdbCore();

public:
    

    int initPid(Settings *cfg, QString gdbPath, QString programPath, int pid);
    int initLocal(Settings *cfg, QString gdbPath, QString programPath, QStringList argumentList);
    int initCoreDump(Settings *cfg, QString gdbPath, QString programPath, QString coreDumpFile);
    int initRemote(Settings *cfg, QString gdbPath, QString programPath, QString tcpHost, int tcpPort);
    int evaluateExpression(QString expr, QString *data);
    
    void setListener(ICore *inf) { m_inf = inf; };

    
private:
    
     void onNotifyAsyncOut(Tree &tree, AsyncClass ac);
     void onExecAsyncOut(Tree &tree, AsyncClass ac);
     void onResult(Tree &tree);
     void onStatusAsyncOut(Tree &tree, AsyncClass ac);
     void onConsoleStreamOutput(QString str);
     void onTargetStreamOutput(QString str);
     void onLogStreamOutput(QString str);

    void dispatchBreakpointDeleted(int id);
    void dispatchBreakpointTree(Tree &tree);
    static ICore::StopReason parseReasonString(QString string);
    void detectMemoryDepth();
    static int openPseudoTerminal();
    void ensureStopped();
    int runInitCommands(Settings *cfg);
    
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

    bool isRunning();
    
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


#endif // FILE__GDBCORE_H
