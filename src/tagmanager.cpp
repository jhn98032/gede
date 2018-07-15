#include "tagmanager.h"

#include "tagscanner.h"
#include "mainwindow.h"
#include "log.h"
#include "util.h"


ScannerWorker::ScannerWorker()
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
            QString filePath = m_workQueue.takeFirst();
            m_mutex.unlock();

            scan(filePath);

            m_mutex.lock();
        }
        m_mutex.unlock();
        m_doneCond.wakeAll();
    }
}

void ScannerWorker::waitAll()
{
    m_mutex.lock();
    while(!m_workQueue.isEmpty())
    {
        m_doneCond.wait(&m_mutex);
    }
    m_mutex.unlock();
    
}

void ScannerWorker::queueScan(QString filePath)
{
    m_mutex.lock();
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

    delete tags;
}
    
int TagManager::queueScan(QString filePath)
{
    assert(m_dbgMainThread == QThread::currentThreadId ());

    if(!m_db.contains(filePath))
    {
        m_worker.queueScan(filePath);
    }
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


