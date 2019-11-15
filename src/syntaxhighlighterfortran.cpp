/*
 * Copyright (C) 2014-2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "syntaxhighlighterfortran.h"

#include <assert.h>
#include <stdio.h>
#include <QStringList>

#include "util.h"
#include "settings.h"


SyntaxHighlighterFortran::Row::Row()
    : isCppRow(0)
{
};


/**
 * @brief Returns the last nonspace field in the row.
 */
TextField *SyntaxHighlighterFortran::Row::getLastNonSpaceField()
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
int SyntaxHighlighterFortran::Row::getCharCount()
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
void SyntaxHighlighterFortran::Row::appendField(TextField* field)
{
    m_fields.push_back(field);
}


SyntaxHighlighterFortran::SyntaxHighlighterFortran()
    : m_cfg(NULL)

{
    QStringList keywordList = Settings::getDefaultFortranKeywordList();
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

SyntaxHighlighterFortran::~SyntaxHighlighterFortran()
{
    reset();
}


bool SyntaxHighlighterFortran::isSpecialChar(QChar c) const
{
    return isSpecialChar(c.toLatin1());
}

/**
 * @brief Checks if a character is a special character.
 * @return Returns true if the character is a special character (Eg: '\t').
*/
bool SyntaxHighlighterFortran::isSpecialChar(char c) const
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

/**
 * @brief Checks if a field is a special character (eg: '>').
 */
bool SyntaxHighlighterFortran::isSpecialChar(TextField *field) const
{
    if(field->m_text.size() == 1)
    {
        return isSpecialChar(field->m_text[0].toLatin1());
    }
    return false;
}


bool SyntaxHighlighterFortran::isCppKeyword(QString text) const
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
bool SyntaxHighlighterFortran::isKeyword(QString text) const
{
    text = text.toLower();
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
void SyntaxHighlighterFortran::pickColor(TextField *field)
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
void SyntaxHighlighterFortran::reset()
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
 * @brief Deallocates all the rows.
 */
void SyntaxHighlighterFortran::setConfig(Settings *cfg)
{
    m_cfg = cfg;

}

/**
 * @brief Creates the row for a number of lines of text.
 */
void SyntaxHighlighterFortran::colorize(QString text)
{
    ParseCharQueue pq(text);
    colorize(pq);
}

void SyntaxHighlighterFortran::colorize(ParseCharQueue pq)
{
    Row *currentRow;
    TextField *field = NULL;
    enum {STATE_STARTLINE,
        STATE_MIDLINE,
        STATE_PRE_SPACES,
        STATE_MID_SPACES,
        STATE_WORD,
        STATE_STRING,
        STATE_ESCAPED_CHAR,
        STATE_INC_STRING
        ,STATE_LINE_COMMENT
    } state = STATE_STARTLINE;
    QChar c = '\n';
    const int tabIndent = m_cfg->getTabIndentCount();
    reset();

    currentRow = new Row;
    m_rows.push_back(currentRow);
    

    while(!pq.isEmpty())
    {
        // Was the last character an escape?
        bool isEscaped = false;
        c = pq.popNext(&isEscaped);

        switch(state)
        {   
            case STATE_STARTLINE:
            {
                if(c == '!')
                {
                    state = STATE_LINE_COMMENT;
                    field = new TextField;
                    field->m_type = TextField::COMMENT;
                    field->m_color = Qt::white;
                    currentRow->appendField(field);
                    field->m_text = c;
                }
                else if(c == ' ' || c == '\t')
                {
                    state = STATE_PRE_SPACES;
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
                else if(c == '\n')
                {
                    currentRow = new Row;
                    m_rows.push_back(currentRow);
                    state = STATE_STARTLINE;
                }
                else
                {
                    pq.revertPop();
                    state = STATE_MIDLINE;
                    field = NULL;
                }
            };break;
            case STATE_MIDLINE:
            {
                
                if(c == ' ' || c == '\t')
                {
                    state = STATE_MID_SPACES;
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
                    state = STATE_ESCAPED_CHAR;
                    field = new TextField;
                    field->m_type = TextField::STRING;
                    currentRow->appendField(field);
                    field->m_text = c;
                }
                else if(c == '"')
                {
                    state = STATE_STRING;
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
                        state = STATE_INC_STRING;
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
                    state = STATE_STARTLINE;
                }
                else
                {
                    state = STATE_WORD;
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
            case STATE_LINE_COMMENT:
            {
                if(c == '\n')
                {
                    currentRow = new Row;
                    m_rows.push_back(currentRow);

                    field = new TextField;
                    field->m_type = TextField::COMMENT;
                    currentRow->appendField(field);

                    state = STATE_STARTLINE;
                }
                else
                {
                    field->m_text += c;
                }

            };break;
            case STATE_PRE_SPACES:
            case STATE_MID_SPACES:
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
                    pq.revertPop();
                    field = NULL;
                    if(state == STATE_PRE_SPACES)
                        state = STATE_STARTLINE;
                    else
                        state = STATE_MIDLINE;
                }  
            };break;
            case STATE_ESCAPED_CHAR:
            {
                
                field->m_text += c;
                if(!isEscaped && c == '\'')
                {
                    field = NULL;
                    state = STATE_MIDLINE;
                }
            };break;
            case STATE_INC_STRING:
            {
                if(!isEscaped && c == '\n')
                {
                    pq.revertPop();
                    field = NULL;
                    state = STATE_STARTLINE;
                }
                else
                {
                    field->m_text += c;
                    if(!isEscaped && c == '>')
                    {
                        field = NULL;
                        state = STATE_MIDLINE;
                    }
                }
            };break;
            case STATE_STRING:
            {
                field->m_text += c;
                if(!isEscaped && c == '"')
                {
                    field = NULL;
                    state = STATE_MIDLINE;
                }
                  
            };break;
            case STATE_WORD:
            {
                if(isSpecialChar(c) || c == ' ' || c == '\t' || c == '\n')
                {
                    pq.revertPop();
                    
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
                    if(c == '\n')
                        state = STATE_STARTLINE;
                    else
                        state = STATE_MIDLINE;
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
QVector<TextField*> SyntaxHighlighterFortran::getRow(unsigned int rowIdx)
{
    assert(rowIdx < getRowCount());
    
    Row *row = m_rows[rowIdx];
    return row->m_fields;
}

