/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "syntaxhighlighter.h"

#include <assert.h>
#include <stdio.h>
#include <QStringList>
#include "util.h"
#include "settings.h"


SyntaxHighlighter::Row::Row()
    : isCppRow(0)
{
};


/**
 * @brief Returns the last nonspace field in the row.
 */
TextField *SyntaxHighlighter::Row::getLastNonSpaceField()
{
    for(int j = fields.size()-1;j >= 0;j--)
    {
        TextField *thisField = fields[j];
        if(thisField->m_type != TextField::SPACES &&
            thisField->m_type != TextField::COMMENT)
        {
            return thisField;
        }
    }
    return NULL;
}


void SyntaxHighlighter::Row::appendField(TextField* field)
{
    fields.push_back(field);
}


SyntaxHighlighter::SyntaxHighlighter()
{
    QStringList keywordList = Settings::getDefaultKeywordList();
    for(int u = 0;u < keywordList.size();u++)
    {
        m_keywords[keywordList[u]] = true;
    }

    QStringList cppKeywordList = Settings::getDefaultCppKeywordList();
    for(int u = 0;u < cppKeywordList.size();u++)
    {
        m_cppKeywords[cppKeywordList[u]] = true;
    }


}

SyntaxHighlighter::~SyntaxHighlighter()
{
    reset();
}


bool SyntaxHighlighter::isSpecialChar(char c) const
{
    if(             c == '\t' ||
                    c == ',' ||
                    c == ';' ||
                    c == '|' ||
                    c == '=' ||
                    c == '(' || c == ')' ||
                    c == '[' || c == ']' ||
                    c == '*' || c == '-' || c == '+' ||
                    c == '?' ||
                    c == '#' ||
                    c == '{' || c == '}' ||
                    c == '<' || c == '>' ||
                    c == '/')
        return true;
    else
        return false;
}

bool SyntaxHighlighter::isSpecialChar(TextField *field) const
{
    if(field->m_text.size() == 1)
    {
        return isSpecialChar(field->m_text[0].toLatin1());
    }
    return false;
}


bool SyntaxHighlighter::isCppKeyword(QString text) const
{
    if(text.size() == 0)
        return false;

    if(m_cppKeywords.contains(text))
    {
        return true;
    }
    else
    {
        return false;
    }
}


bool SyntaxHighlighter::isKeyword(QString text) const
{
    if(text.size() == 0)
        return false;
    if(m_keywords.contains(text))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void SyntaxHighlighter::pickColor(TextField *field)
{
    assert(field != NULL);
    if(field->m_type == TextField::COMMENT)
        field->m_color = Qt::green;
    else if(field->m_type == TextField::STRING)
        field->m_color = QColor(0,125, 250);
    else if(field->m_type == TextField::INC_STRING)
        field->m_color = QColor(0,125, 250);
    else if(field->m_text.isEmpty())
        field->m_color = Qt::white;
    else if(field->m_type == TextField::KEYWORD)
       field->m_color = Qt::yellow;
    else if(field->m_type == TextField::CPP_KEYWORD)
        field->m_color = QColor(240,110,110);
    else if(field->m_type == TextField::NUMBER)
        field->m_color = Qt::magenta;
    else
       field->m_color = Qt::white;
    
}


void SyntaxHighlighter::reset()
{
    for(int r = 0;r < m_rows.size();r++)
    {
        Row *currentRow = m_rows[r];

        assert(currentRow != NULL);
        for(int j = 0;j < currentRow->fields.size();j++)
        {
            delete currentRow->fields[j];
        }
        delete currentRow;
    }
    m_rows.clear();
}


void SyntaxHighlighter::colorize(QString text)
{
    Row *currentRow;
    TextField *field = NULL;
    enum {IDLE,
        MULTI_COMMENT,
        SPACES,
        WORD, GLOBAL_INCLUDE_FILE, COMMENT1,COMMENT,
        STRING,
        ESCAPED_CHAR,
        INC_STRING
    } state = IDLE;
    char c = '\n';
    char prevC = ' ';
    char prevPrevC = ' ';
    bool isEscaped = false;
    
    reset();

    currentRow = new Row;
    m_rows.push_back(currentRow);
    

    for(int i = 0;i < text.size();i++)
    {
        c = text[i].toLatin1();

        // Was the last character an escape?
        if(prevC == '\\' && prevPrevC != '\\')
            isEscaped = true;
        else
            isEscaped = false;
        prevPrevC = prevC;
        prevC = c;
        
        
        switch(state)
        {   
            case IDLE:
            {
                if(c == '/')
                {
                    state = COMMENT1;
                    field = new TextField;
                    field->m_type = TextField::WORD;
                    field->m_color = Qt::white;
                    currentRow->appendField(field);
                    field->m_text = c;
                }
                else if(c == ' ' || c == '\t')
                {
                    state = SPACES;
                    field = new TextField;
                    field->m_type = TextField::SPACES;
                    field->m_color = Qt::white;
                    currentRow->appendField(field);
                    field->m_text = c;
                }
                else if(c == '\'')
                {
                    state = ESCAPED_CHAR;
                    field = new TextField;
                    field->m_type = TextField::STRING;
                    currentRow->appendField(field);
                    field->m_text = c;
                }
                else if(c == '"')
                {
                    state = STRING;
                    field = new TextField;
                    if(currentRow->isCppRow)
                        field->m_type = TextField::INC_STRING;
                    else
                        field->m_type = TextField::STRING;
                    currentRow->appendField(field);
                    field->m_text = c;
                }
                else if(c == '<' && currentRow->isCppRow)
                {
                    // Is it a include string?
                    bool isIncString = false;
                    TextField *lastField = currentRow->getLastNonSpaceField();
                    if(lastField)
                    {
                        if(lastField->m_text == "include")
                            isIncString = true;
                    }

                    // Add the field
                    field = new TextField;
                    field->m_text = c;
                    if(isIncString)
                    {
                        state = INC_STRING;
                        field->m_type = TextField::INC_STRING;
                    }
                    else
                    {
                        field->m_type = TextField::WORD;
                        field->m_color = Qt::white;
                    }
                    currentRow->appendField(field);
                
                }
                else if(c == '#')
                {
                    // Only spaces before the '#' at the line?
                    bool onlySpaces = true;
                    for(int j = 0;onlySpaces == true && j < currentRow->fields.size();j++)
                    {
                        if(currentRow->fields[j]->m_type != TextField::SPACES &&
                            currentRow->fields[j]->m_type != TextField::COMMENT)
                        {
                            onlySpaces = false;
                        }
                    }
                    currentRow->isCppRow = onlySpaces ? true : false;

                    // Create a new field structure
                    field = new TextField;
                    if(currentRow->isCppRow)
                        field->m_type = TextField::CPP_KEYWORD;
                    else
                        field->m_type = TextField::WORD;
                    field->m_color = Qt::white;
                    currentRow->appendField(field);
                    field->m_text = c;
                }
                else if(isSpecialChar(c))
                {
                    field = new TextField;
                    field->m_type = TextField::WORD;
                    field->m_color = Qt::white;
                    currentRow->appendField(field);
                    field->m_text = c;
                }
                else if(c == '\n')
                {
                    currentRow = new Row;
                    m_rows.push_back(currentRow);
                    state = IDLE;
                }
                else
                {
                    state = WORD;
                    field = new TextField;
                    if(QChar(c).isDigit())
                        field->m_type = TextField::NUMBER;
                    else
                        field->m_type = TextField::WORD;
                    field->m_color = Qt::white;
                    currentRow->appendField(field);
                    field->m_text = c;
                }
            };break;
            case COMMENT1:
            {
                if(c == '*')
                {
                    field->m_text += c;
                    field->m_type = TextField::COMMENT;
                    field->m_color = Qt::green;
                    state = MULTI_COMMENT;
                    
                }
                else if(c == '/')
                {
                    field->m_text += c;
                    field->m_type = TextField::COMMENT;
                    field->m_color = Qt::green;
                    state = COMMENT;
                }
                else
                {
                    i--;
                    state = IDLE;
                }
            };break;
            case MULTI_COMMENT:
            {
                if(c == '\n')
                {
                    currentRow = new Row;
                    m_rows.push_back(currentRow);

                    field = new TextField;
                    field->m_type = TextField::COMMENT;
                    currentRow->appendField(field);
                    
                }
                else if(text[i-1].toLatin1() == '*' && c == '/')
                {
                    field->m_text += c;
                    state = IDLE;
                }
                else
                {
                    field->m_text += c;
                }
            };break;
            case COMMENT:
            {
                if(c == '\n')
                {
                    i--;
                    state = IDLE;
                }
                else
                    field->m_text += c;
                    
            };break;
            case SPACES:
            {
                if(c == ' ' || c == '\t')
                {
                    field->m_text += c;
                }
                else
                {
                    i--;
                    field = NULL;
                    state = IDLE;
                }  
            };break;
            case GLOBAL_INCLUDE_FILE:
            {
                if(!isEscaped && c == '\n')
                {
                    state = IDLE;
                }
                else
                {
                    field->m_text += c;
                    if(c == '>')
                    {
                        state = IDLE;
                    }
                }
            };break;
            case ESCAPED_CHAR:
            {
                field->m_text += c;
                if(!isEscaped && c == '\'')
                {
                    field = NULL;
                    state = IDLE;
                }
            };break;
            case INC_STRING:
            {
                if(!isEscaped && c == '\n')
                {
                    i--;
                    field = NULL;
                    state = IDLE;
                }
                else
                {
                    field->m_text += c;
                    if(!isEscaped && c == '>')
                    {
                        field = NULL;
                        state = IDLE;
                    }
                }
            };break;
            case STRING:
            {
                field->m_text += c;
                if(!isEscaped && c == '"')
                {
                    field = NULL;
                    state = IDLE;
                }
                  
            };break;
            case WORD:
            {
                if(isSpecialChar(c) || c == ' ' || c == '\t' || c == '\n')
                {
                    i--;
                    if(currentRow->isCppRow)
                    {
                        if(isCppKeyword(field->m_text))
                            field->m_type = TextField::CPP_KEYWORD;
                    }
                    else
                    {
                        if(isKeyword(field->m_text))
                            field->m_type = TextField::KEYWORD;
                    }
    
                    field = NULL;
                    state = IDLE;
                }
                else
                {
                    
                    field->m_text += c;
                }
                
            };break;
        }
    }

    for(int r = 0;r < m_rows.size();r++)
    {
        Row *currentRow = m_rows[r];

        for(int j = 0;j < currentRow->fields.size();j++)
        {
            TextField* currentField = currentRow->fields[j];
            pickColor(currentField);
        }
    }
}



QVector<TextField*> SyntaxHighlighter::getRow(unsigned int rowIdx)
{
    assert(rowIdx < getRowCount());
    
    Row *row = m_rows[rowIdx];
    return row->fields;
}
