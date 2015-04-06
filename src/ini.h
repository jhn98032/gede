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

class Entry
{
public:
    Entry(const Entry &other);
    Entry(QString name_) : name(name_) {};
    
    int getValueAsInt() { return value.toInt(0,0); };
    QString getValueAsString() { return value; };

    QString name;
    QString value;
};

class Ini
{
public:

    Ini();
    Ini(const Ini &src);

    virtual ~Ini();

    Ini& operator= (const Ini &src);
    void copy(const Ini &src);
    
    void setInt(QString name, int value);
    void setString(QString name, QString value);
    void setStringList(QString name, QStringList value);
    void setBool(QString name, bool value);
    
    bool getBool(QString name, bool defaultValue = false);
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
    Entry *addEntry(QString name);

private:
    QVector<Entry*> m_entries;
};

#endif // FILE__INI_H
