#ifndef FILE__AUTO_SIGNAL_BLOCKER_H
#define FILE__AUTO_SIGNAL_BLOCKER_H

#include <QObject>

class AutoSignalBlocker
{
private:
    bool m_signalBlocked;
    QObject *m_obj;

public:
    AutoSignalBlocker(QObject *obj)
        :m_obj(obj)
    {
         m_signalBlocked = m_obj->blockSignals(true);
    };
    virtual ~AutoSignalBlocker()
    {
         m_obj->blockSignals(m_signalBlocked);
    }

    private:
        AutoSignalBlocker(){};
    
};

#endif
