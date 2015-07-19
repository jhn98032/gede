#ifndef FILE__TAGMANAGER_H
#define FILE__TAGMANAGER_H

#include <QList>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
 

#include "tagscanner.h"

class FileInfo;

class ScannerWorker : public QThread
{
    Q_OBJECT
    
    public:
        ScannerWorker();

        void run();
        
        void waitAll();

        void requestQuit();
        void queueScan(FileInfo *info);
    private:
        void scan(FileInfo *info, QString filepath);
    
    signals:
        void onScanDone(FileInfo *info, QList<Tag> *taglist);

    private:
        TagScanner m_scanner;
        
#ifndef NDEBUG
        Qt::HANDLE m_dbgMainThread;
#endif

        QMutex m_mutex;
        QWaitCondition m_wait;
        QWaitCondition m_doneCond;
        QList<FileInfo*> m_queue;
        bool m_quit;
};


class TagManager : public QObject
{

    Q_OBJECT
    
public:
    TagManager();
    virtual ~TagManager();


    int queueScan(FileInfo *info);
    int scan(QString filename, QList<Tag> *tagList);

    void waitAll();

private slots:
    void onScanDone(FileInfo *info, QList<Tag> *tags);
    
private:
    ScannerWorker m_worker;
    TagScanner m_tagScanner;

#ifndef NDEBUG
    Qt::HANDLE m_dbgMainThread;
#endif
};


#endif // FILE__TAGMANAGER_H
