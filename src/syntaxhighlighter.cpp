#include "syntaxhighlighter.h"

   
#include <assert.h>
#include <QStringList>
#include "util.h"
#include "settings.h"

#include <stdio.h>


SyntaxHighlighter::SyntaxHighlighter()
{
    QStringList keywordList = Settings::getDefaultKeywordList();
    for(int u = 0;u < keywordList.size();u++)
    {
        m_keywords[keywordList[u]] = true;
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
                    c == '*' || c == '-' || c == '+' ||
                    c == '#' ||
                    c == '{' || c == '}' ||
                    c == '/')
        return true;
    else
        return false;
}

bool SyntaxHighlighter::isSpecialChar(TextField *field) const
{
    if(field->m_text.size() == 1)
    {
        return isSpecialChar(field->m_text[0].toAscii());
    }
    return false;
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
        field->m_color = QColor(60,60,255);
    else if(field->m_text.isEmpty())
        field->m_color = Qt::white;
    else if(isKeyword(field->m_text))
       field->m_color = Qt::yellow;
    else if(field->m_text[0].isDigit())
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
        ESCAPED_CHAR
    } state = IDLE;
    char c = '\n';
    char prevC = ' ';
    
    reset();

    currentRow = new Row;
    m_rows.push_back(currentRow);
    

    for(int i = 0;i < text.size();i++)
    {
        c = text[i].toAscii();
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
                    currentRow->fields.push_back(field);
                    field->m_text = c;
                }
                else if(c == ' ' || c == '\t')
                {
                    state = SPACES;
                    field = new TextField;
                    field->m_type = TextField::SPACES;
                    field->m_color = Qt::white;
                    currentRow->fields.push_back(field);
                    field->m_text = c;
                }
                else if(c == '\'')
                {
                    state = ESCAPED_CHAR;
                    field = new TextField;
                    field->m_type = TextField::STRING;
                    currentRow->fields.push_back(field);
                    field->m_text = c;
                }
                else if(c == '"')
                {
                    state = STRING;
                    field = new TextField;
                    field->m_type = TextField::STRING;
                    currentRow->fields.push_back(field);
                    field->m_text = c;
                }
                else if(isSpecialChar(c))
                {
                    field = new TextField;
                    field->m_type = TextField::WORD;
                    field->m_color = Qt::white;
                    currentRow->fields.push_back(field);
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
                    field->m_type = TextField::WORD;
                    field->m_color = Qt::white;
                    currentRow->fields.push_back(field);
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
                    currentRow->fields.push_back(field);
                    
                }
                else if(text[i-1].toAscii() == '*' && c == '/')
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
                field->m_text += c;
                if(c == '>')
                {
                    state = IDLE;
                }
                
            };break;
            case ESCAPED_CHAR:
            {
                field->m_text += c;
                if(prevC != '\\' && c == '\'')
                {
                    field = NULL;
                    state = IDLE;
                }
            };break;
            case STRING:
            {
                field->m_text += c;
                if(prevC != '\\' && c == '"')
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
                    pickColor(field);
                    field = NULL;
                    state = IDLE;
                }
                else
                {
                    
                    field->m_text += c;
                }
                
            };break;
        }
        prevC = c;
    }

    for(int r = 0;r < m_rows.size();r++)
    {
        Row *currentRow = m_rows[r];
    
        for(int j = 0;j < currentRow->fields.size();j++)
        {
            pickColor(currentRow->fields[j]);
        }
    }
}

QVector<TextField*> SyntaxHighlighter::getRow(unsigned int rowIdx)
{
    assert(rowIdx < getRowCount());
    
    Row *row = m_rows[rowIdx];
    return row->fields;
}
