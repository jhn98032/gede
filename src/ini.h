/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__INI_H
#define FILE__INI_H

#include <QSize>
#include <QVector>
#include <QString>
#include <QColor>
#include <QVariant>

class IniGroup;
class Ini;

class IniEntry
{
public:
    IniEntry(const IniEntry &other);
    IniEntry(QString name_);
    
    int getValueAsInt() const;
    double getValueAsFloat() const;
    QString getValueAsString() const;

    typedef enum {TYPE_BYTE_ARRAY, TYPE_SIZE, TYPE_STRING, TYPE_FLOAT, TYPE_INT, TYPE_COLOR} EntryType;

    QString getName() const { return m_name; };

private:
    QString m_name;
    QVariant m_value;
    EntryType m_type;

friend IniGroup;
friend Ini;
};

class IniGroup
{
    public:
        IniGroup(const IniGroup &other);
        IniGroup(QString name_);
        IniGroup();
        virtual ~IniGroup();

        void removeAll();
        void dump() const;
        IniEntry *findEntry(QString entryName);
        IniEntry *addEntry(QString name, IniEntry::EntryType type);
        void copy(const IniGroup &src);

        QString getName() const { return m_name; };

    private:
        QString m_name;
        QVector<IniEntry *> m_entries;
        
    friend Ini;
};

class Ini
{
public:

    Ini();
    Ini(const Ini &src);

    virtual ~Ini();

    Ini& operator= (const Ini &src);
    void copy(const Ini &src);
    
    void setByteArray(QString name, const QByteArray &byteArray);
    void setInt(QString name, int value);
    void setString(QString name, QString value);
    void setStringList(QString name, QStringList value);
    void setBool(QString name, bool value);
    void setColor(QString name, QColor value);
    void setSize(QString name, QSize size);
    void setDouble(QString name, double value);
    
    bool getBool(QString name, bool defaultValue = false);
    void getByteArray(QString name, QByteArray *byteArray);
    int getInt(QString name, int defaultValue = -1);
    QColor getColor(QString name, QColor defaultValue);
    QString getString(QString name, QString defaultValue = "");
    QStringList getStringList(QString name, QStringList defaultValue);
    QSize getSize(QString name, QSize defaultSize);
    double getDouble(QString name, double defValue);
    
    
    int appendLoad(QString filename);
    int save(QString filename);
    void dump();

private:
    IniEntry *addEntry(QString groupName, QString name, IniEntry::EntryType type);
    void divideName(QString name, QString *groupName, QString *entryName);
    IniGroup *findGroup(QString groupName);
    void removeAll();
    IniEntry *findEntry(QString name);
    IniEntry *addEntry(QString name, IniEntry::EntryType type);
    int decodeValueString(IniEntry *entry, QString specialKind, QByteArray valueStr);
    QString encodeValueString(const IniEntry &entry);
    
private:
    QVector<IniGroup*> m_entries;
};

#endif // FILE__INI_H

