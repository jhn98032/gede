/*
 * Copyright (C) 2014-2021 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "ini.h"

#include <QFile>
#include <QStringList>
#include <assert.h>
#include <QtDebug>

#include "util.h"
#include "log.h"

// Enables this if the entries should be sorted in the ini file.
#define INI_SORT_ENTRIES

//----------------------------------------------------------------
//
//     -- IniEntry --
//
//----------------------------------------------------------------

#if QT_VERSION >= QT_VERSION_CHECK(5,5,0)
#define formatString(str, fmt...)   (str) = QString::asprintf(fmt)
#else
#define formatString(str, fmt...)   (str).sprintf(fmt)
#endif


IniEntry::IniEntry(QString name_)
 : m_name(name_)
{
}

IniEntry::IniEntry(const IniEntry &src)
    : m_name(src.m_name)
    ,m_value(src.m_value)
    ,m_type(src.m_type)
{

}

int IniEntry::getValueAsInt() const
{
    return m_value.toInt();
}


double IniEntry::getValueAsFloat() const
{
    return m_value.toDouble();
}

QString IniEntry::getValueAsString()  const
{
    return m_value.toString();
}



//----------------------------------------------------------------
//
//     -- IniGroup --
//
//----------------------------------------------------------------


IniGroup::IniGroup(const IniGroup &other)
{
    copy(other);
}

IniGroup::IniGroup(QString name_)
    : m_name(name_)
{

}

void IniGroup::dump() const
{

    for(int i = 0;i < m_entries.size();i++)
    {
        IniEntry *entry = m_entries[i];
        QString valueStr = entry->m_value.toString();
        qDebug() << "Name:" << entry->m_name << "=" << stringToCStr(valueStr);
    }

}

        
IniGroup::IniGroup()
{

}
        
IniGroup::~IniGroup()
{
    removeAll();
}

IniEntry *IniGroup::addEntry(QString name, IniEntry::EntryType type)
{
    IniEntry *entry = findEntry(name);
    if(!entry)
    {
        entry = new IniEntry(name);
        m_entries.push_back(entry);
    }
    entry->m_type = type;
    return entry;
}


IniEntry *IniGroup::findEntry(QString entryName)
{
    for(int i = 0;i < m_entries.size();i++)
    {
        IniEntry *entry = NULL;
        entry = m_entries[i];
        if(entry->m_name == entryName)
            return entry;
        assert(entry->m_name.toUpper() != entryName.toUpper());

    }
    return NULL;
}


void IniGroup::removeAll()
{
    for(int i = 0;i < m_entries.size();i++)
    {
        IniEntry *entry = m_entries[i];
        delete entry;
    }
    m_entries.clear();
}

/**
  * @brief Replaces the entries in this ini with another one.
  */
void IniGroup::copy(const IniGroup &src)
{
    removeAll();
    for(int i = 0;i < src.m_entries.size();i++)
    {
        IniEntry *entry = src.m_entries[i];
        IniEntry *newEntry = new IniEntry(*entry);
        m_entries.push_back(newEntry);
    }
}



//----------------------------------------------------------------
//
//     -- Ini --
//
//----------------------------------------------------------------



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
        IniGroup *entry = m_entries[i];
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
        IniGroup *entry = src.m_entries[i];
        IniGroup *newEntry = new IniGroup(*entry);
        m_entries.push_back(newEntry);
    }
}


void Ini::removeAll()
{
    for(int i = 0;i < m_entries.size();i++)
    {
        IniGroup *entry = m_entries[i];
        delete entry;
    }
    m_entries.clear();
}


IniGroup *Ini::findGroup(QString groupName)
{
    for(int i = 0;i < m_entries.size();i++)
    {
        IniGroup *entry = NULL;
        entry = m_entries[i];
        if(entry->m_name == groupName)
            return entry;
    }
    return NULL;
}

void Ini::divideName(QString name, QString *groupName, QString *entryName)
{
    *groupName = "";
    *entryName = "";
    int divPos = name.lastIndexOf('/');
    if(divPos != -1)
    {
        *groupName = name.left(divPos);
        *entryName = name.mid(divPos+1);
    }
    else
        *entryName = name;

}

IniEntry *Ini::findEntry(QString name)
{
    QString groupName;
    QString entryName;
    divideName(name, &groupName, &entryName);
    IniGroup *group = findGroup(groupName);
    IniEntry *entry = NULL;
    if(group)
        entry = group->findEntry(entryName);

    //assert(entry != NULL);
    //debugMsg("Unable to find '%s'", stringToCStr(name));

    return entry;
}

IniEntry *Ini::addEntry(QString name, IniEntry::EntryType type)
{
    QString groupName;
    QString entryName;
    divideName(name, &groupName, &entryName);
    return addEntry(groupName, entryName, type);
}
    
IniEntry *Ini::addEntry(QString groupName, QString entryName, IniEntry::EntryType type)
{
    IniGroup *group = findGroup(groupName);
    if(group == NULL)
    {
        group = new IniGroup(groupName);
        m_entries.push_back(group);
    }
    return group->addEntry(entryName, type);
}

    
void Ini::setInt(QString name, int value)
{
    IniEntry *entry = addEntry(name, IniEntry::TYPE_INT);
    entry->m_value = value;
}

void Ini::setDouble(QString name, double value)
{
    IniEntry *entry = addEntry(name, IniEntry::TYPE_FLOAT);
    entry->m_value = value;
}


void Ini::setByteArray(QString name, const QByteArray &byteArray)
{
    IniEntry *entry = addEntry(name, IniEntry::TYPE_BYTE_ARRAY);
    entry->m_value = byteArray;
}


void Ini::setBool(QString name, bool value)
{
    IniEntry *entry = addEntry(name, IniEntry::TYPE_INT);
    entry->m_value = (int)value;
}

void Ini::setString(QString name, QString value)
{
    IniEntry *entry = addEntry(name, IniEntry::TYPE_STRING);
    entry->m_value = value;
}

void Ini::setStringList(QString name, QStringList value)
{
    IniEntry *entry = addEntry(name, IniEntry::TYPE_STRING);
    entry->m_value = value.join(";");
}

int Ini::getInt(QString name, int defaultValue)
{
    IniEntry *entry = findEntry(name);
    if(!entry)
    {
        setInt(name, defaultValue);
        entry = findEntry(name);
    }
    return entry->getValueAsInt();
}

double Ini::getDouble(QString name, double defaultValue)
{
    IniEntry *entry = findEntry(name);
    if(!entry)
    {
        setDouble(name, defaultValue);
        entry = findEntry(name);
    }
    return entry->getValueAsFloat();
}


bool Ini::getBool(QString name, bool defaultValue)
{
    IniEntry *entry = findEntry(name);
    if(!entry)
    {
        setBool(name, defaultValue);
        entry = findEntry(name);
    }
    return entry->getValueAsInt();
}

QString Ini::getString(QString name, QString defaultValue)
{
    IniEntry *entry = findEntry(name);
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
    IniEntry *entry = findEntry(name);
    if(!entry)
    {
        setByteArray(name, *byteArray);
        entry = findEntry(name);
    }
    *byteArray = entry->m_value.toByteArray();
}


QColor Ini::getColor(QString name, QColor defaultValue)
{
    IniEntry *entry = findEntry(name);
    if(!entry)
    {
        setColor(name, defaultValue);
        entry = findEntry(name);
    }
    return QColor(entry->m_value.toString());
    
}

QSize Ini::getSize(QString name, QSize defaultSize)
{
    IniEntry *entry = findEntry(name);
    if(!entry)
    {
        setSize(name, defaultSize);
        entry = findEntry(name);
    }
    return entry->m_value.toSize();
}

void Ini::setSize(QString name, QSize size)
{
    IniEntry *entry = addEntry(name, IniEntry::TYPE_SIZE);
    entry->m_value = size;
}


void Ini::setColor(QString name, QColor value)
{
    IniEntry *entry = addEntry(name, IniEntry::TYPE_COLOR);
    QString valueStr;
    formatString(valueStr,"#%02x%02x%02x", value.red(), value.green(), value.blue());
    entry->m_value = valueStr;
}

void Ini::dump()
{

    for(int i = 0;i < m_entries.size();i++)
    {
        IniGroup *entry = m_entries[i];
        entry->dump();
    }

}


static bool compareGroup(const IniGroup* s1, const IniGroup* s2)
{
    return s1->getName() < s2->getName();
}

static bool compareEntry(const IniEntry* s1, const IniEntry* s2)
{
    return s1->getName() < s2->getName();
}

/**
 * @brief Saves the content to a ini file.
 * @return 0 on success.
 */
int Ini::save(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text))
        return 1;

    QVector<IniGroup*> entriesList;
    entriesList = m_entries;
#ifdef INI_SORT_ENTRIES
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    std::sort(entriesList.begin(), entriesList.end(), compareGroup);
#else
    qSort(entriesList.begin(), entriesList.end(), compareGroup);
#endif
#endif
    
    for(int j = 0;j < entriesList.size();j++)
    {
        IniGroup *group = entriesList[j];
        assert(group != NULL);
        QString groupName = group->m_name;
        if(!groupName.isEmpty())
        {
            QString data = "[" + groupName + "]\r\n";
            file.write(data.toUtf8());
        }

        QVector<IniEntry*> entryList = group->m_entries;
#ifdef INI_SORT_ENTRIES
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
        std::sort(entryList.begin(), entryList.end(), compareEntry);
#else
        qSort(entryList.begin(), entryList.end(), compareEntry);
#endif
#endif
        for(int i = 0;i < entryList.size();i++)
        {
            IniEntry *entry = entryList[i];
            file.write(entry->m_name.toUtf8());
            file.write("=");
            QString valueStr = encodeValueString(*entry);
            file.write(valueStr.toUtf8());
            file.write("\r\n");
        }

        file.write("\r\n");

    }
    file.close();
    return 0;
}


/**
 * @brief Fills in a entry from a ini-file string.
 */
int Ini::decodeValueString(IniEntry *entry, QString specialKind, QString dataArray)
{
    int rc = 0;
    if(specialKind == "ByteArray")
    {
        QByteArray byteArray;
        char hexStr[3];
        enum {IDLE, ESC, FIRST_HEX, SECOND_HEX} state = IDLE;
        QString valueStr = dataArray;
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
                        byteArray += c.toLatin1();
                    
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
                        byteArray += c.toLatin1();
                        state = IDLE;
                    }
                    
                };break;
                case FIRST_HEX:
                {
                    hexStr[0] = c.toLatin1();
                    state = SECOND_HEX;
                };break;
                case SECOND_HEX:
                {
                    hexStr[1] = c.toLatin1();
                    byteArray += hexStringToU8(hexStr);
                    state = IDLE;
                };break;
            };
            
        }
        entry->m_value = byteArray;
        entry->m_type = IniEntry::TYPE_BYTE_ARRAY;
        //printf("_>%s<\n", stringToCStr(valueStr));
    }
    else if(specialKind == "Size")
    {
        QString valueStr = dataArray;
        QStringList list = valueStr.split(' ');
        QSize s;
        if(list.size() > 2)
            rc = -1;
        else
            s = QSize(list[0].toInt(), list[1].toInt());
        entry->m_value = s;
        entry->m_type = IniEntry::TYPE_SIZE;
    }
    else if(specialKind == "")
    {
        entry->m_value =  dataArray;
    }
    else
    {
        entry->m_value = dataArray;
        rc = -1;
    }
    return rc;
}


/**
 * @brief Converts a entry to a string suitable to storing in a ini file.
 */
QString Ini::encodeValueString(const IniEntry &entry)
{
    QString value;
    if(entry.m_type == IniEntry::TYPE_BYTE_ARRAY)
    {
        QByteArray byteArray = entry.m_value.toByteArray();
        value = "@ByteArray(";
        for (int i = 0;i < byteArray.size();i++)
        {
            QString subStr;
            formatString(subStr,"\\x%02x", (unsigned char)byteArray[i]);
            value += subStr;
        }
        value += ")";
    }
    else if(entry.m_type == IniEntry::TYPE_SIZE)
    {
        QSize s = entry.m_value.toSize();
        formatString(value, "@Size(%d %d)", s.width(), s.height());
    }
    else if(entry.m_type == IniEntry::TYPE_INT)
    {
        value.setNum(entry.getValueAsInt());
    }
    else if(entry.m_type == IniEntry::TYPE_FLOAT)
    {
        value.setNum(entry.getValueAsFloat());
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
    QString value2;
    QString specialKind;
    QString groupName;

    debugMsg("Ini::%s(filename:\"%s\")", __func__, stringToCStr(filename));
    
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return 1;
    }
    
    QString allContent(file.readAll ());


    enum { STATE_IDLE, STATE_SKIP_LINE, STATE_KEY, STATE_GROUP, STATE_VALUE,
        STATE_SPECIAL_TYPE, STATE_SPECIAL_DATA, STATE_VALUE_STR } state = STATE_IDLE;
    for(int i = 0;i < allContent.size();i++)
    {

        QChar c = allContent[i];

    switch(state)
    {
            case STATE_IDLE:
            {
                if(c == QChar('='))
                {
                    errorMsg("Empty key at L%d", lineNo);
                    state = STATE_SKIP_LINE;
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
                    state = STATE_SKIP_LINE;
                }
                else if(c == QChar('['))
                {
                    str = "";
                    state = STATE_GROUP;
                }
                else
                {
                    str = c;
                    state = STATE_KEY;
                    specialKind = "";
                }
                
            };break;
            case STATE_GROUP:
            {
                if(c == QChar('\n') || c == QChar('\r'))
                {
                    errorMsg("Parse error at L%d", lineNo);
                    lineNo++;
                    state = STATE_IDLE;
                }
                else if(c == QChar(']'))
                {
                    groupName = str;
                    state = STATE_IDLE;
                }
                else
                    str += c;


            };break;
            case STATE_KEY:
            {
                if(c == QChar('\n') || c == QChar('\r'))
                {
                    errorMsg("Parse error at L%d", lineNo);
                    lineNo++;
                    state = STATE_IDLE;
                }
                else if(c == QChar('='))
                {
                    name = str;
                    value2.clear();
                    state = STATE_VALUE;
                }
                else
                    str += c;

            };break;
            case STATE_VALUE:
            {
                if(c == QChar('\n') || c == QChar('\r'))
                {
                    lineNo++;

                    IniEntry *entry = addEntry(groupName, name.trimmed(), IniEntry::TYPE_STRING);
                    if(decodeValueString(entry, "", value2.trimmed()))
                        warnMsg("Parse error in %s:L%d", stringToCStr(filename), lineNo);
                             
                    state = STATE_IDLE;
                }
                else
                {
                    if(value2.isEmpty())
                    {
                        if(c == '"')
                            state = STATE_VALUE_STR;
                        else if(c == '@')
                            state = STATE_SPECIAL_TYPE;
                        else if(!c.isSpace())
                            value2 += c;
                    }
                    else
                    {
                        value2 += c;
                    }
                }
            };break;
            case STATE_SPECIAL_TYPE:
            {
                if(c == QChar('\n') || c == QChar('\r'))
                {
                    lineNo++;

                    errorMsg("Parse error in L%d", lineNo);
                             
                    state = STATE_IDLE;
                }
                else
                {
                    if(c == '(')
                    {
                        specialKind = value2;
                        value2.clear();
                        state = STATE_SPECIAL_DATA;
                    }
                    else if(!c.isSpace())
                        value2 += c;
                }
            };break;
            case STATE_SPECIAL_DATA:
            {
                if(c == QChar(')'))
                {
                    IniEntry *entry = addEntry(groupName, name.trimmed(), IniEntry::TYPE_STRING);
                    decodeValueString(entry,specialKind, value2.trimmed());
                             
                    state = STATE_IDLE;
                }
                else
                    value2 += c;

            };break;
            case STATE_VALUE_STR:
            {
                if(c == QChar('"'))
                {
                    IniEntry *entry = addEntry(groupName, name.trimmed(), IniEntry::TYPE_STRING);
                    entry->m_value = value2;
                             
                    state = STATE_IDLE;
                }
                else
                    value2 += c;
                
            };break;
            case STATE_SKIP_LINE:
            {
                if(c == QChar('\n') || c == QChar('\r'))
                {
                    lineNo++;
                    state = STATE_IDLE;
                }
            };break;

        };
    }
    return 0;
}




