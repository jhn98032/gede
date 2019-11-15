#ifndef FILE__STRINGQUEUE_H
#define FILE__STRINGQUEUE_H

#include <QString>

class ParseCharQueue
{
public:

    ParseCharQueue(QString str);
    virtual ~ParseCharQueue();

    QChar popNext(bool *isEscaped = NULL);
    void revertPop();
    bool isEmpty();
    
private:
    QString m_list;
    int m_idx;
    bool m_isEscMode;
};


#endif // FILE__STRINGQUEUE_H

