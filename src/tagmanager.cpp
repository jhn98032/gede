#include "tagmanager.h"

#include "tagscanner.h"
#include "mainwindow.h"
#include "log.h"



ScannerWorker::ScannerWorker()
{
#ifndef NDEBUG
    m_dbgMainThread = QThread::currentThreadId ();
#endif
    m_quit = false;
    m_scanner.init();
}


void ScannerWorker::requestQuit()
{
    m_quit = true;
    m_wait.wakeAll();
}
        
void ScannerWorker::run()
{
    assert(m_dbgMainThread != QThread::currentThreadId ());

    while(m_quit == false)
    {
        m_mutex.lock();
        m_wait.wait(&m_mutex);
        while(!m_workQueue.isEmpty())
        {
            ScannerWork* work = m_workQueue.takeFirst();
            m_mutex.unlock();

            scan(work->fileInfo, work->filename);
            delete work;
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

void ScannerWorker::queueScan(FileInfo *info)
{
    m_mutex.lock();
    ScannerWork *work = new ScannerWork;
    work->fileInfo = info;
    work->filename = info->fullName;
    
    m_workQueue.append(work);
    m_wait.wakeAll();
    m_mutex.unlock();
}


void ScannerWorker::scan(FileInfo *info, QString filepath)
{
    QList<Tag> *taglist = new QList<Tag>;
    

    assert(m_dbgMainThread != QThread::currentThreadId ());
    
    
    m_scanner.scan(filepath,taglist);


    emit onScanDone(info, taglist);
}


TagManager::TagManager()
{
#ifndef NDEBUG
    m_dbgMainThread = QThread::currentThreadId ();
#endif

    m_worker.start();
    
    connect(&m_worker, SIGNAL(onScanDone(FileInfo *, QList<Tag>* )), this, SLOT(onScanDone(FileInfo *, QList<Tag>* )));
    
}

TagManager::~TagManager()
{
    m_worker.requestQuit();
    m_worker.wait();
}

void TagManager::waitAll()
{
    m_worker.waitAll();
}



void TagManager::onScanDone(FileInfo *info, QList<Tag> *tags)
{
    assert(m_dbgMainThread == QThread::currentThreadId ());

    
    info->m_tagList.append(*tags);
    delete tags;
}
    
int TagManager::queueScan(FileInfo *info)
{
    assert(m_dbgMainThread == QThread::currentThreadId ());

  // m_tagScanner.scan(info->fullName, &info->m_tagList);  
    m_worker.queueScan(info);
    return 0;
}

int TagManager::scan(QString filename, QList<Tag> *tagList)
{
    m_tagScanner.init();
    return m_tagScanner.scan(filename, tagList);
}

        

    
