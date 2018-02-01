/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef  FILE__SYNTAXHIGHLIGHTER
#define  FILE__SYNTAXHIGHLIGHTER

#include <settings.h>

#include <QVector>
#include <QString>
#include <QColor>
#include <QHash>


struct TextField
{
    QColor m_color;
    QString m_text;
    enum {COMMENT, WORD, NUMBER, KEYWORD, CPP_KEYWORD, INC_STRING, STRING, SPACES} m_type;

    bool isHash() const { return m_text == "#" ? true : false; };
    bool isSpaces() const { return m_type == SPACES ? true : false; };
    int getLength() { return m_text.length(); };
};


class SyntaxHighlighter
{
public:
    SyntaxHighlighter(){};
    virtual ~SyntaxHighlighter(){};
    
    virtual void colorize(QString text) = 0;

    virtual QVector<TextField*> getRow(unsigned int rowIdx) = 0;
    virtual unsigned int getRowCount() = 0;
    virtual void reset() = 0;

    virtual bool isKeyword(QString text) const = 0;
    virtual bool isSpecialChar(char c) const = 0;
    virtual bool isSpecialChar(TextField *field) const = 0;
    virtual void setConfig(Settings *cfg) = 0;
};

#endif // #ifndef FILE__SYNTAXHIGHLIGHTER
