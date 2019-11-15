#include "parsecharqueue.h"


ParseCharQueue::ParseCharQueue(QString str)
  : m_list(str)
    ,m_idx(0)
    ,m_isEscMode(false)
{
    
}


ParseCharQueue::~ParseCharQueue()
{

}



QChar ParseCharQueue::popNext(bool *isEscaped)
{
    QChar c = ' ';
    if(isEscaped)
        *isEscaped = false;

    if(m_idx < m_list.size())
    {
        c = m_list[m_idx++];
        if(m_isEscMode)
        {
            m_isEscMode = false;
            if(isEscaped)
                *isEscaped = true;
        }
        else if(c == '\\')
        {
            m_isEscMode = true;
        }
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

    if(m_isEscMode)
        m_isEscMode = false;
    else
    {
        if(m_idx > 1)
        {
            QChar prev = m_list[m_idx-1];
            if(prev == '\\')
                m_isEscMode = true;
        }
    }
}



