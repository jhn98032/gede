#include "parsecharqueue.h"


ParseCharQueue::ParseCharQueue(QString str)
  : m_list(str),
  m_idx(0)
{

}


ParseCharQueue::~ParseCharQueue()
{

}


QChar ParseCharQueue::getPrev()
{
    QChar c = ' ';
    int prevIdx = m_idx-1;
    if(0 <= prevIdx)
        c = m_list[prevIdx];
    return c;
}

QChar ParseCharQueue::getPrevPrev()
{
    QChar c = ' ';
    int prevIdx = m_idx-2;
    if(0 <= prevIdx)
        c = m_list[prevIdx];
    return c;
}


QChar ParseCharQueue::popNext(bool *isEscaped)
{
    QChar c = ' ';
    if(isEscaped)
    {
        if(getPrev() == '\\' && getPrevPrev() != '\\')
            *isEscaped = true;
        else
            *isEscaped = false;
    }

    if(m_idx < m_list.size())
    {
        c = m_list[m_idx++];
    }
    return c;
}

bool ParseCharQueue::isEmpty()
{
    if(m_idx < m_list.size())
        return false;
    return true;
}

void ParseCharQueue::revertPop()
{
    if(m_idx > 0)
        m_idx--;
}


