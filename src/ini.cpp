/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "ini.h"

#include <QFile>
#include <QStringList>
#include "util.h"
#include "log.h"
#include <assert.h>

Entry::Entry(const Entry &src)
    : m_name(src.m_name)
    ,m_value(src.m_value)
    ,m_type(src.m_type)
{

}

int Entry::getValueAsInt()
{
    return m_value.toInt();
}

QString Entry::getValueAsString()
{
    return m_value.toString();
}



Ini::Ini()
{

}

Ini::Ini(const Ini &src)
{
    copy(src);
}
    
Ini::~Ini()
{
    for(int i = 0;i < m_entries.size();i++)
    {
        Entry *entry = m_entries[i];
        delete entry;
    }

}


Ini& Ini::operator=(const Ini &src)
{
    if(&src == this)
        return *this;
    copy(src);
    return *this;
}


/**
  * @brief Replaces the entries in this ini with another one.
  */
void Ini::copy(const Ini &src)
{
    removeAll();
    for(int i = 0;i < src.m_entries.size();i++)
    {
        Entry *entry = src.m_entries[i];
        Entry *newEntry = new Entry(*entry);
        m_entries.push_back(newEntry);
    }
}


void Ini::removeAll()
{
    for(int i = 0;i < m_entries.size();i++)
    {
        Entry *entry = m_entries[i];
        delete entry;
    }
    m_entries.clear();
}



Entry *Ini::findEntry(QString name)
{
    for(int i = 0;i < m_entries.size();i++)
    {
        Entry *entry = NULL;
        entry = m_entries[i];
        if(entry->m_name == name)
            return entry;
    }
    return NULL;
}

    
Entry *Ini::addEntry(QString name, Entry::EntryType type)
{
    Entry *entry = findEntry(name);
    if(!entry)
    {
        entry = new Entry(name);
        m_entries.push_back(entry);
    }
    entry->m_type = type;
    return entry;
}

    
void Ini::setInt(QString name, int value)
{
    Entry *entry = addEntry(name, Entry::TYPE_INT);
    entry->m_value = value;
}


void Ini::setByteArray(QString name, const QByteArray &byteArray)
{
    Entry *entry = addEntry(name, Entry::TYPE_BYTE_ARRAY);
    entry->m_value = byteArray;
}


void Ini::setBool(QString name, bool value)
{
    Entry *entry = addEntry(name, Entry::TYPE_INT);
    entry->m_value = (int)value;
}

void Ini::setString(QString name, QString value)
{
    Entry *entry = addEntry(name, Entry::TYPE_STRING);
    entry->m_value = value;
}

void Ini::setStringList(QString name, QStringList value)
{
    Entry *entry = addEntry(name, Entry::TYPE_STRING);
    entry->m_value = value.join(";");
}

int Ini::getInt(QString name, int defaultValue)
{
    Entry *entry = findEntry(name);
    if(!entry)
    {
        setInt(name, defaultValue);
        entry = findEntry(name);
    }
    return entry->getValueAsInt();
}


bool Ini::getBool(QString name, bool defaultValue)
{
    Entry *entry = findEntry(name);
    if(!entry)
    {
        setBool(name, defaultValue);
        entry = findEntry(name);
    }
    return entry->getValueAsInt();
}

QString Ini::getString(QString name, QString defaultValue)
{
    Entry *entry = findEntry(name);
    if(!entry)
    {
        setString(name, defaultValue);
        entry = findEntry(name);
    }
        
    return entry->getValueAsString();
}

QStringList Ini::getStringList(QString name, QStringList defaultValue)
{
    QString list = getString(name, defaultValue.join(";"));
    return list.split(";");
}


void Ini::getByteArray(QString name, QByteArray *byteArray)
{
    Entry *entry = findEntry(name);
    if(!entry)
    {
        setByteArray(name, *byteArray);
        entry = findEntry(name);
    }
    *byteArray = entry->m_value.toByteArray();
}


QColor Ini::getColor(QString name, QColor defaultValue)
{
    Entry *entry = findEntry(name);
    if(!entry)
    {
        setColor(name, defaultValue);
        entry = findEntry(name);
    }
    return QColor(entry->m_value.toString());
    
}

    
void Ini::setColor(QString name, QColor value)
{
    Entry *entry = addEntry(name, Entry::TYPE_COLOR);
    QString valueStr;
    valueStr.sprintf("#%02x%02x%02x", value.red(), value.green(), value.blue());
    entry->m_value = valueStr;
}

void Ini::dump()
{

    for(int i = 0;i < m_entries.size();i++)
    {
        Entry *entry = m_entries[i];
        QString valueStr = entry->m_value.toString();
        printf("_%s_%s_\n", stringToCStr(entry->m_name), stringToCStr(valueStr));
    }

}


/**
 * @brief Saves the content to a ini file.
 * @return 0 on success.
 */
int Ini::save(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::Truncate | QIODevice::ReadWrite | QIODevice::Text))
        return 1;
    for(int i = 0;i < m_entries.size();i++)
    {
        Entry *entry = m_entries[i];
        file.write(stringToCStr(entry->m_name));
        file.write("=");
        QString valueStr = encodeValueString(*entry);
        file.write(stringToCStr(valueStr));
        file.write("\r\n");
    }
    return 0;
}


/**
 * @brief Fills in a entry from a ini-file string.
 */
void Ini::decodeValueString(Entry *entry, QString valueStr)
{
    if(valueStr.startsWith("@ByteArray("))
    {
        valueStr = valueStr.mid(11);
        if(valueStr.endsWith(")"))
            valueStr = valueStr.left(valueStr.length()-1);

        QByteArray byteArray;
        char hexStr[3];
        enum {IDLE, ESC, FIRST_HEX, SECOND_HEX} state = IDLE;
        for(int i = 0;i < valueStr.length();i++)
        {
            QChar c = valueStr[i];
            switch(state)
            {
                case IDLE:
                {
                    if(c == '\\')
                        state = ESC;
                    else
                        byteArray += c;
                    
                };break;
                case ESC:
                {
                    if(c == '\\')
                    {
                        byteArray += '\\';
                        state = IDLE;
                    }
                    else if(c == '0')
                    {
                        byteArray += '\0';
                        state = IDLE;
                    }
                    else if(c == 'x' || c == 'X')
                    {
                        state = FIRST_HEX;
                        memset(hexStr, 0, sizeof(hexStr));
                    }
                    else if(c == '\r' || c == '\n')
                        state = IDLE;
                    else
                    {
                        byteArray += c;
                        state = IDLE;
                    }
                    
                };break;
                case FIRST_HEX:
                {
                    hexStr[0] = c.toAscii();
                    state = SECOND_HEX;
                };break;
                case SECOND_HEX:
                {
                    hexStr[1] = c.toAscii();
                    byteArray += hexStringToU8(hexStr);
                    state = IDLE;
                };break;
            };
            
        }
        entry->m_value = byteArray;
        entry->m_type = Entry::TYPE_BYTE_ARRAY;
        //printf("_>%s<\n", stringToCStr(valueStr));
    }
    else
        entry->m_value = valueStr;
}

/**
 * @brief Converts a entry to a string suitable to storing in a ini file.
 */
QString Ini::encodeValueString(const Entry &entry)
{
    QString value;
    if(entry.m_type == Entry::TYPE_BYTE_ARRAY)
    {
        QByteArray byteArray = entry.m_value.toByteArray();
        value = "@ByteArray(";
        for (int i = 0;i < byteArray.size();i++)
        {
            QString subStr;
            subStr.sprintf("\\x%02x", (unsigned char)byteArray[i]);
            value += subStr;
        }
        value += ")";
    }
    else
        value = "\"" + entry.m_value.toString() + "\"";
    return value;
}


/**
 * @brief Loads the content of a ini file.
 * @return 0 on success.
 */
int Ini::appendLoad(QString filename)
{
    int lineNo = 1;
    QString str;
    QString name;
    QString value;
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return 1;

    
    QString allContent(file.readAll ());


    enum { IDLE, SKIP_LINE, KEY, VALUE, VALUE_STR } state = IDLE;
    for(int i = 0;i < allContent.size();i++)
    {

        QChar c = allContent[i];

    switch(state)
    {
            case IDLE:
            {
                if(c == QChar('='))
                {
                    errorMsg("Empty key at L%d", lineNo);
                    state = SKIP_LINE;
                }
                else if(c == QChar('\n') || c == QChar('\r'))
                {
                    lineNo++;
                }
                else if(c.isSpace())
                {

                }
                else if(c == QChar('#'))
                {
                    state = SKIP_LINE;
                }
                else
                {
                    str = c;
                    state = KEY;
                }
                
            };break;
            case KEY:
            {
                if(c == QChar('\n') || c == QChar('\r'))
                {
                    errorMsg("Parse error at L%d", lineNo);
                    lineNo++;
                    state = IDLE;
                }
                else if(c == QChar('='))
                {
                    name = str;
                    value = "";
                    state = VALUE;
                }
                else
                    str += c;

            };break;
            case VALUE:
            {
                if(c == QChar('\n') || c == QChar('\r'))
                {
                    lineNo++;

                    Entry *entry = addEntry(name.trimmed(), Entry::TYPE_STRING);
                    decodeValueString(entry, value.trimmed());
                             
                    state = IDLE;
                }
                else
                {
                    if(value.isEmpty())
                    {
                        if(c == '"')
                            state = VALUE_STR;
                        else if(!c.isSpace())
                            value += c;
                    }
                    else
                        value += c;
                }
            };break;
            case VALUE_STR:
            {
                if(c == QChar('"'))
                {
                    lineNo++;

                    Entry *entry = addEntry(name.trimmed(), Entry::TYPE_STRING);
                    decodeValueString(entry,value.trimmed());
                             
                    state = IDLE;
                }
                else
                    value += c;
                
            };break;
            case SKIP_LINE:
            {
                if(c == QChar('\n') || c == QChar('\r'))
                {
                    lineNo++;
                    state = IDLE;
                }
            };break;

        };
    }
    return 0;
}




