/*
 * Copyright (C) 2014-2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */


#include "syntaxhighlightercxx.h"

#include <assert.h>
#include <stdio.h>
#include <QStringList>

#include "util.h"
#include "settings.h"


SyntaxHighlighterCxx::Row::Row()
    : isCppRow(0)
{
};


/**
 * @brief Returns the last nonspace field in the row.
 */
TextField *SyntaxHighlighterCxx::Row::getLastNonSpaceField()
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


/**
 * @brief Returns the number of characters in the row.
 */
int SyntaxHighlighterCxx::Row::getCharCount()
{
    int len = 0;
    for(int j = m_fields.size()-1;j >= 0;j--)
    {
        TextField *thisField = m_fields[j];
        len += thisField->getLength();

    }
    return len;
}


/**
 * @brief Appends a field to the row.
 */
void SyntaxHighlighterCxx::Row::appendField(TextField* field)
{
    m_fields.push_back(field);
}


SyntaxHighlighterCxx::SyntaxHighlighterCxx()
    : m_cfg(NULL)

{
    QStringList keywordList = Settings::getDefaultCxxKeywordList();
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

SyntaxHighlighterCxx::~SyntaxHighlighterCxx()
{
    reset();
}


/**
 * @brief Checks if a character is a special character.
 * @return Returns true if the character is a special character (Eg: '\t').
*/
bool SyntaxHighlighterCxx::isSpecialChar(char c) const
{
    if(             c == '\t' ||
                    c == ':' ||
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


/**
 * @brief Checks if a field is a special character (eg: '>').
 */
bool SyntaxHighlighterCxx::isSpecialChar(TextField *field) const
{
    if(field->m_text.size() == 1)
    {
        return isSpecialChar(field->m_text[0].toLatin1());
    }
    return false;
}


bool SyntaxHighlighterCxx::isCppKeyword(QString text) const
{
    if(text.isEmpty())
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


/**
 * @brief Checks if a string is a keyword.
 */
bool SyntaxHighlighterCxx::isKeyword(QString text) const
{
    if(text.isEmpty())
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


/**
 * @brief Picks a color for a specific field.
 */
void SyntaxHighlighterCxx::pickColor(TextField *field)
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


/**
 * @brief Deallocates all the rows.
 */
void SyntaxHighlighterCxx::reset()
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

/**
 * @brief Sets the configuration to use.
 */
void SyntaxHighlighterCxx::setConfig(Settings *cfg)
{
    m_cfg = cfg;

}

/**
 * @brief Creates the row for a number of lines of text.
 */
void SyntaxHighlighterCxx::colorize(QString text)
{
    Row *currentRow;
    TextField *field = NULL;
    enum {IDLE,
        MULTI_COMMENT,
        SPACES,
        WORD, COMMENT1,COMMENT,
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
                        if(lastField->m_text.compare("include",Qt::CaseInsensitive) == 0)
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



/**
 * @brief Returns a text row.
 * @return rowIdx   The row to get (0=first row).
 * @return The fields of the row.
 */
QVector<TextField*> SyntaxHighlighterCxx::getRow(unsigned int rowIdx)
{
    assert(rowIdx < getRowCount());
    
    Row *row = m_rows[rowIdx];
    return row->m_fields;
}

