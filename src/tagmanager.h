#ifndef FILE__TAGMANAGER_H
#define FILE__TAGMANAGER_H

#include <QList>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QString>
#include <QMap>

#include "tagscanner.h"

class FileInfo;

struct ScannerResult
{
    QString m_filePath;
    QList<Tag> m_tagList;
};

class ScannerWorker : public QThread
{
    Q_OBJECT
    
    public:
        ScannerWorker();

        void run();
        
        void abort();
        void waitAll();

        void requestQuit();
        void queueScan(QString filePath);

        bool isIdle();

        void setConfig(Settings cfg);
        
    private:
        void scan(QString filePath);
    
    signals:
        void onScanDone(QString filePath, QList<Tag> *taglist);

    private:
        TagScanner m_scanner;
        
#ifndef NDEBUG
        Qt::HANDLE m_dbgMainThread;
#endif

        QMutex m_mutex;
        QWaitCondition m_wait;
        QWaitCondition m_doneCond;
        QList<QString> m_workQueue;
        bool m_quit;
        Settings m_cfg;
        bool m_isIdle;

};


class TagManager : public QObject
{

    Q_OBJECT

private:
    TagManager(){};
public:
    TagManager(Settings &cfg);
    virtual ~TagManager();


    int queueScan(QStringList filePathList);
    void scan(QString filePath, QList<Tag> *tagList);

    void waitAll();

    void abort();

    void getTags(QString filePath, QList<Tag> *tagList);

    void lookupTag(QString name, QList<Tag> *tagList);

    void setConfig(Settings &cfg);
signals:
    void onAllScansDone();
    
private slots:
    void onScanDone(QString filePath, QList<Tag> *tags);
    
private:
    ScannerWorker m_worker;
    TagScanner m_tagScanner;

#ifndef NDEBUG
    Qt::HANDLE m_dbgMainThread;
#endif
    QMap<QString, ScannerResult*> m_db;

    Settings m_cfg;
};


#endif // FILE__TAGMANAGER_H
