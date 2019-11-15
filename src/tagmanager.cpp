//#define ENABLE_DEBUGMSG


/*
 * Copyright (C) 2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
 
#include "tagmanager.h"

#include "tagscanner.h"
#include "mainwindow.h"
#include "log.h"
#include "util.h"


ScannerWorker::ScannerWorker()
 : m_isIdle(true)
{
#ifndef NDEBUG
    m_dbgMainThread = QThread::currentThreadId ();
#endif
    m_quit = false;
}


void ScannerWorker::requestQuit()
{
    m_quit = true;
    m_wait.wakeAll();
}
        
void ScannerWorker::abort()
{
    QMutexLocker locker(&m_mutex);
    m_workQueue.clear();
}

bool ScannerWorker::isIdle()
{
    QMutexLocker locker(&m_mutex);
    return m_isIdle;
}

    
void ScannerWorker::run()
{
    assert(m_dbgMainThread != QThread::currentThreadId ());

    m_cfg.load();
    
    m_scanner.init(&m_cfg);

    while(m_quit == false)
    {
        m_mutex.lock();
        m_wait.wait(&m_mutex);
        while(!m_workQueue.isEmpty())
        {
            m_isIdle = false;
            QString filePath = m_workQueue.takeFirst();
            m_mutex.unlock();

            scan(filePath);

            m_mutex.lock();
        }
        m_isIdle = true;
        m_mutex.unlock();
        m_doneCond.wakeAll();
    }
}

void ScannerWorker::waitAll()
{
    m_mutex.lock();
    while(!m_workQueue.isEmpty() || m_isIdle == false)
    {
        m_doneCond.wait(&m_mutex);
    }
    m_mutex.unlock();
    
}

void ScannerWorker::queueScan(QString filePath)
{
    m_mutex.lock();
    m_isIdle = false;
    m_workQueue.append(filePath);
    m_mutex.unlock();
    m_wait.wakeAll();
}




void ScannerWorker::scan(QString filePath)
{
    QList<Tag> *taglist = new QList<Tag>;
    

    assert(m_dbgMainThread != QThread::currentThreadId ());
    
    
    m_scanner.scan(filePath, taglist);


    emit onScanDone(filePath, taglist);
}


TagManager::TagManager()
{
#ifndef NDEBUG
    m_dbgMainThread = QThread::currentThreadId ();
#endif

    m_cfg.load();
    
    m_worker.start();
    
    connect(&m_worker, SIGNAL(onScanDone(QString, QList<Tag>* )), this, SLOT(onScanDone(QString, QList<Tag>* )));
    

    m_tagScanner.init(&m_cfg);

}

TagManager::~TagManager()
{
    m_worker.requestQuit();
    m_worker.wait();

    
    foreach (ScannerResult* info, m_db)
    {
        delete info;
    }
}

void TagManager::waitAll()
{
    m_worker.waitAll();
}



void TagManager::onScanDone(QString filePath, QList<Tag> *tags)
{
    assert(m_dbgMainThread == QThread::currentThreadId ());

    ScannerResult *info = new ScannerResult;
    info->m_filePath = filePath;
    info->m_tagList = *tags;

    if(m_db.contains(filePath))
    {
        ScannerResult *oldInfo = m_db[filePath];
        delete oldInfo;
    }

    m_db[filePath] = info;

    if(m_worker.isIdle())
        emit onAllScansDone();

    delete tags;
}

/**
 * @brief Tags a scan to be made later (in a seperate thread).
 */
int TagManager::queueScan(QStringList filePathList)
{
    bool queuedAny = false;

    assert(m_dbgMainThread == QThread::currentThreadId ());
    for(int i = 0;i < filePathList.size();i++)
    {
        QString filePath = filePathList[i];
        if(!m_db.contains(filePath))
        {
            m_worker.queueScan(filePath);
            queuedAny = true;
        }
    }

    if(!queuedAny)
        emit onAllScansDone();

    return 0;
}



void TagManager::scan(QString filePath, QList<Tag> *tagList)
{
    if(!m_db.contains(filePath))
    {
        ScannerResult *res = new ScannerResult;
        res->m_filePath = filePath;

        m_tagScanner.scan(res->m_filePath, &res->m_tagList);

        m_db[filePath] = res;
    }

    *tagList = m_db[filePath]->m_tagList;

}

void TagManager::abort()
{
    m_worker.abort();
}

void TagManager::getTags(QString filePath, QList<Tag> *tagList)
{
    if(m_db.contains(filePath))
    {
        *tagList = m_db[filePath]->m_tagList;
    }
}


/**
 * @brief Lookup tags with a specific name.
 * @param name       The name of the tag (Eg: "main" or "Class::myFunc").
 * @return tagList   The found tags.
 */
void TagManager::lookupTag(QString name, QList<Tag> *tagList)
{
    debugMsg("%s(name:'%s')", __func__, qPrintable(name)); 

    QString funcName;
    QString className;

    if(name.contains("::"))
    {
        int divPos = name.indexOf("::");
        funcName = name.mid(divPos+2);
        className = name.left(divPos);
    }
    else
        funcName = name;

    foreach (ScannerResult* info, m_db)
    {
        for(int j = 0;j < info->m_tagList.size();j++)
        {
            Tag &tag = info->m_tagList[j];
            
            if(tag.getName() == funcName && className == tag.getClassName())
                tagList->append(tag);
            
        }
    }

}



