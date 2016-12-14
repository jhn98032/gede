/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__INI_H
#define FILE__INI_H

#include <QVector>
#include <QString>
#include <QColor>
#include <QVariant>


class Entry
{
public:
    Entry(const Entry &other);
    Entry(QString name_) : m_name(name_) {};
    
    int getValueAsInt();
    QString getValueAsString();
    
    QString m_name;
    QVariant m_value;
    typedef enum {TYPE_BYTE_ARRAY, TYPE_STRING, TYPE_INT, TYPE_COLOR} EntryType;
    EntryType m_type;
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
    
    bool getBool(QString name, bool defaultValue = false);
    void getByteArray(QString name, QByteArray *byteArray);
    int getInt(QString name, int defaultValue = -1);
    QColor getColor(QString name, QColor defaultValue);
    QString getString(QString name, QString defaultValue = "");
    QStringList getStringList(QString name, QStringList defaultValue);
    void setColor(QString name, QColor value);
    
    int appendLoad(QString filename);
    int save(QString filename);
    void dump();

private:
    void removeAll();
    Entry *findEntry(QString name);
    Entry *addEntry(QString name, Entry::EntryType type);
    void decodeValueString(Entry *entry, QString valueStr);
    QString encodeValueString(const Entry &entry);
    
private:
    QVector<Entry*> m_entries;
};

#endif // FILE__INI_H

