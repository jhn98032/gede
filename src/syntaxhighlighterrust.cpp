/*
 * Copyright (C) 2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */


#include "syntaxhighlighterrust.h"

#include <assert.h>
#include <stdio.h>
#include <QStringList>
#include "util.h"
#include "settings.h"


SyntaxHighlighterRust::Row::Row()
    : isCppRow(0)
{
};


/**
 * @brief Returns the last nonspace field in the row.
 */
TextField *SyntaxHighlighterRust::Row::getLastNonSpaceField()
{
    for(int j = m_fields.size()-1;j >= 0;j--)
    {
        TextField *thisField = m_fields[j];
        if(thisField->m_type != TextField::SPACES &&
            thisField->m_type != TextField::COMMENT)
        {
            return thisField;
        }
    }
    return NULL;
}


int SyntaxHighlighterRust::Row::getCharCount()
{
    int len = 0;
    for(int j = m_fields.size()-1;j >= 0;j--)
    {
        TextField *thisField = m_fields[j];
        len += thisField->getLength();

    }
    return len;
}

        
void SyntaxHighlighterRust::Row::appendField(TextField* field)
{
    m_fields.push_back(field);
}


SyntaxHighlighterRust::SyntaxHighlighterRust()
    : m_cfg(NULL)

{
    QStringList keywordList = Settings::getDefaultRustKeywordList();
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

SyntaxHighlighterRust::~SyntaxHighlighterRust()
{
    reset();
}


bool SyntaxHighlighterRust::isSpecialChar(char c) const
{
    if(             c == '\t' ||
                    c == ',' ||
                    c == ';' ||
                    c == '|' ||
                    c == '=' ||
                    c == '(' || c == ')' ||
                    c == '[' || c == ']' ||
                    c == '*' || c == '-' || c == '+' || c == '%' ||
                    c == '?' ||
                    c == '#' ||
                    c == '{' || c == '}' ||
                    c == '<' || c == '>' ||
                    c == '/')
        return true;
    else
        return false;
}

bool SyntaxHighlighterRust::isSpecialChar(TextField *field) const
{
    if(field->m_text.size() == 1)
    {
        return isSpecialChar(field->m_text[0].toLatin1());
    }
    return false;
}


bool SyntaxHighlighterRust::isCppKeyword(QString text) const
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


bool SyntaxHighlighterRust::isKeyword(QString text) const
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

void SyntaxHighlighterRust::pickColor(TextField *field)
{
    assert(field != NULL);
    assert(m_cfg != NULL);
    if(m_cfg == NULL)
        field->m_color = Qt::white;
        
    if(field->m_type == TextField::COMMENT)
        field->m_color = m_cfg->m_clrComment;
    else if(field->m_type == TextField::STRING)
        field->m_color = m_cfg->m_clrString;
    else if(field->m_type == TextField::INC_STRING)
        field->m_color = m_cfg->m_clrIncString;
    else if(field->m_text.isEmpty())
        field->m_color = m_cfg->m_clrForeground;
    else if(field->m_type == TextField::KEYWORD)
       field->m_color = m_cfg->m_clrKeyword;
    else if(field->m_type == TextField::CPP_KEYWORD)
        field->m_color = m_cfg->m_clrCppKeyword;
    else if(field->m_type == TextField::NUMBER)
        field->m_color = m_cfg->m_clrNumber;
    else
       field->m_color = m_cfg->m_clrForeground;
    
}


void SyntaxHighlighterRust::reset()
{
    for(int r = 0;r < m_rows.size();r++)
    {
        Row *currentRow = m_rows[r];

        assert(currentRow != NULL);
        for(int j = 0;j < currentRow->m_fields.size();j++)
        {
            delete currentRow->m_fields[j];
        }
        delete currentRow;
    }
    m_rows.clear();
}

void SyntaxHighlighterRust::setConfig(Settings *cfg)
{
    m_cfg = cfg;

}

void SyntaxHighlighterRust::colorize(QString text)
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
    const int tabIndent = m_cfg->getTabIndentCount();
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
                    if(c == '\t')
                    {
                        int spacesToAdd = tabIndent-(currentRow->getCharCount()%tabIndent);
                        field->m_text = QString(spacesToAdd, ' ');
                    }
                    else
                        field->m_text = c;
                    currentRow->appendField(field);
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
                    for(int j = 0;onlySpaces == true && j < currentRow->m_fields.size();j++)
                    {
                        if(currentRow->m_fields[j]->m_type != TextField::SPACES &&
                            currentRow->m_fields[j]->m_type != TextField::COMMENT)
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
                // An '->' token?
                else if(c == '>' && field != NULL)
                {
                    if(field->m_text == "-")
                        field->m_text += ">";
                    else
                    {
                        field = new TextField;
                        field->m_type = TextField::WORD;
                        field->m_color = Qt::white;
                        currentRow->appendField(field);
                        field->m_text = c;
                    }
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
                    if(c == '\t')
                    {
                        int spacesToAdd = tabIndent-(currentRow->getCharCount()%tabIndent);
                        field->m_text += QString(spacesToAdd, ' ');
                    }
                    else
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

        for(int j = 0;j < currentRow->m_fields.size();j++)
        {
            TextField* currentField = currentRow->m_fields[j];
            pickColor(currentField);
        }
    }
}



QVector<TextField*> SyntaxHighlighterRust::getRow(unsigned int rowIdx)
{
    assert(rowIdx < getRowCount());
    
    Row *row = m_rows[rowIdx];
    return row->m_fields;
}
