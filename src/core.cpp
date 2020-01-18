/*
 * Copyright (C) 2014-2016 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

//#define ENABLE_DEBUGMSG

#include "core.h"

#include <QByteArray>
#include <QDebug>

#include "util.h"
#include "log.h"


VarWatch::VarWatch()
    : m_inScope(true)
    ,m_hasChildren(false)
{
}

VarWatch::VarWatch(QString watchId_, QString name_)
  : m_watchId(watchId_)
    ,m_name(name_)
    ,m_inScope(true)
    ,m_var(name_)
    ,m_hasChildren(false)
{

}


bool VarWatch::hasChildren()
{
    return m_hasChildren;
}



CoreVar::CoreVar()
 : m_address(0)
   ,m_type(TYPE_UNKNOWN)
   ,m_addressValid(false)
{

}

    
CoreVar::CoreVar(QString name)
    : m_name(name)
    ,m_address(0)
      ,m_type(TYPE_UNKNOWN)
    ,m_addressValid(false)
{

}

CoreVar::~CoreVar()
{
    clear();
}

    
quint64 CoreVar::getPointerAddress()
{
    return m_address;
}

void CoreVar::clear()
{
}


void CoreVar::setData(Type type, QVariant data)
{
    m_type = type;
    m_data = data;
}


QString CoreVar::getData(DispFormat fmt) const
{
    QString valueText;

    if(fmt == FMT_NATIVE)
    {
        if(m_type == TYPE_HEX_INT)
            fmt = FMT_HEX;
        if(m_type == TYPE_CHAR)
            fmt = FMT_CHAR;
    }

    if(m_type == TYPE_ENUM)
        return m_data.toString();
    else if(m_type == TYPE_CHAR || m_type == TYPE_HEX_INT || m_type == TYPE_DEC_INT)
    {
        if(fmt == FMT_CHAR)
        {
            QChar c = m_data.toInt();
            char clat = c.toLatin1();
            if(isprint(clat))
                valueText.sprintf("%d '%c'", (int)m_data.toInt(), clat);
            else
                valueText.sprintf("%d ' '", (int)m_data.toInt());
        }
        else if(fmt == FMT_BIN)
        {
            QString subText;
            QString reverseText;
            qlonglong val = m_data.toULongLong();
            do
            {
                subText.sprintf("%d", (int)(val & 0x1));
                reverseText = subText + reverseText;
                val = val>>1;
            }
            while(val > 0 || reverseText.length()%8 != 0);
            for(int i = 0;i < reverseText.length();i++)
            {
                valueText += reverseText[i];
                if(i%4 == 3 && i+1 != reverseText.length())
                    valueText += "_";
            }

            valueText = "0b" + valueText;
        }
        else if(fmt == FMT_HEX)
        {
            QString text;
            text.sprintf("%llx", m_data.toLongLong());

            // Prefix the string with suitable number of zeroes
            while(text.length()%4 != 0 && text.length() > 4)
                text = "0" + text;
            if(text.length()%2 != 0)
                text = "0" + text;
                
            for(int i = 0;i < text.length();i++)
            {
                valueText = valueText + text[i];
                if(i%4 == 3 && i+1 != text.length())
                    valueText += "_";
            }
            valueText = "0x" + valueText;
        }
        else// if(fmt == FMT_DEC)
        {
            valueText = m_data.toString();
        }
    }
    else if(m_type == TYPE_STRING)
    {
          valueText = '"' + m_data.toString() + '"';
    }
    else
        valueText = m_data.toString();

    return valueText;
}

static Core *m_core = NULL;

Core* Core::getInstance()
{
    return m_core;
}

void Core::setInstance(Core *c)
{
    m_core = c;
}

  
