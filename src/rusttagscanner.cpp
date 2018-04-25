/*
 * Copyright (C) 2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include <assert.h>
#include <stdio.h>
#include <QStringList>
#include "util.h"
#include "settings.h"

#include "rusttagscanner.h"
#include <QFile>


// #define ENABLE_DEBUGMSG


#include <assert.h>
#include <stdio.h>
#include <QStringList>
#include "util.h"
#include "settings.h"
#include "log.h"

    
int RustTagScanner::scan(QString filepath, QList<Tag> *taglist)
{
    m_filepath = filepath;
    
// Read file content
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errorMsg("Failed to open '%s'", stringToCStr(filepath));
        return -1;
    }
    QString text;
    while (!file.atEnd())
    {
         QByteArray line = file.readLine();
         text += line;
    }

    tokenize(text);

    parse(taglist);
    
    return 0;
}


RustTagScanner::Token* RustTagScanner::pushToken(char c, Token::Type type, int lineNr)
{
    QString text(c);
    return pushToken(text, type, lineNr);
}


RustTagScanner::Token* RustTagScanner::pushToken(QString text, Token::Type type, int lineNr)
{
    Token *tok = new Token(lineNr);
    tok->m_type = type;
    tok->text = text;
    m_tokens.append(tok);
    return tok;
}



    
RustTagScanner::Token* RustTagScanner::popToken()
{
    if(m_tokens.isEmpty())
        return NULL;
    Token* tok = m_tokens.takeFirst();
    if(tok)
        m_freeTokens.append(tok);
    return tok;
}

RustTagScanner::Token* RustTagScanner::peekToken()
{
    if(m_tokens.isEmpty())
        return NULL;
    return m_tokens.first();
}



QString RustTagScanner::tokenToDesc(Token *tok)
{
    QString typeStr;
    switch(tok->m_type)
    {
        case Token::STRING: typeStr = "STRING";break;
        case Token::NUMBER: typeStr = "NUMBER";break;
        case Token::COMMENT: typeStr = "COMMENT";break;
        case Token::WORD: typeStr = "WORD";break;
        case Token::KEYWORD: typeStr = "KEYWORD";break;
    };
        
    QString str;
    str.sprintf("[L%d;%s>%s<]", tok->m_lineNr, qPrintable(typeStr), qPrintable(tok->text));
    return str;
}


void RustTagScanner::parse(QList<Tag> *taglist)
{
    
    Token *tok;
    enum { IDLE, FN_KW} state = IDLE;
    do
    {
        tok = popToken();
        if(tok)
        {
            switch(state)
            {
                case IDLE:
                {
                    debugMsg("tok: %s", qPrintable(tokenToDesc(tok)));
                    if(tok->text == "fn")
                        state = FN_KW;
                };break;
                case FN_KW:
                {
                    debugMsg("found: '%s' at L%d", qPrintable(tok->text), tok->m_lineNr);
                    Tag tag;
                    tag.setLineNo(tok->m_lineNr);
                    tag.m_name = tok->text;
                    tag.filepath = m_filepath;
                    tag.type = Tag::TAG_FUNC;
                    taglist->append(tag);
                    state = IDLE;
                };break;
                default:;break;
            }
        }
    }while(tok);

}
    
RustTagScanner::Row::Row()
{
};


/**
 * @brief Returns the last nonspace field in the row.
 */
TextField *RustTagScanner::Row::getLastNonSpaceField()
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


int RustTagScanner::Row::getCharCount()
{
    int len = 0;
    for(int j = m_fields.size()-1;j >= 0;j--)
    {
        TextField *thisField = m_fields[j];
        len += thisField->getLength();

    }
    return len;
}

        
void RustTagScanner::Row::appendField(TextField* field)
{
    m_fields.push_back(field);
}


RustTagScanner::RustTagScanner()
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

RustTagScanner::~RustTagScanner()
{
    reset();


    for(int i = 0;i < m_tokens.size();i++)
    {
        Token* tok = m_tokens[i];
        delete tok;
    }
    m_tokens.clear();
    for(int i = 0;i < m_freeTokens.size();i++)
    {
        Token* tok = m_freeTokens[i];
        delete tok;
    }
    m_freeTokens.clear();
    


}


bool RustTagScanner::isSpecialChar(char c) const
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

bool RustTagScanner::isSpecialChar(TextField *field) const
{
    if(field->m_text.size() == 1)
    {
        return isSpecialChar(field->m_text[0].toLatin1());
    }
    return false;
}




bool RustTagScanner::isKeyword(QString text) const
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


void RustTagScanner::reset()
{
/*
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

*/
}

void RustTagScanner::setConfig(Settings *cfg)
{
    m_cfg = cfg;

}

void RustTagScanner::tokenize(QString text2)
{
    int lineNr = 1;
    enum {IDLE,
        MULTI_COMMENT,
        SPACES,
        WORD, GLOBAL_INCLUDE_FILE, COMMENT1,COMMENT,
        STRING,
        ESCAPED_CHAR,
        INC_STRING,
        MINUS
    } state = IDLE;
    char c = '\n';
    char prevC = ' ';
    char prevPrevC = ' ';
    bool isEscaped = false;
    QString text;

    reset();

    

    for(int i = 0;i < text2.size();i++)
    {
        c = text2[i].toLatin1();

        if(prevC != '\\' && c == '\n')
            lineNr++;

        // Was the last character an escape?
        if(prevC == '\\' && prevPrevC != '\\')
            isEscaped = true;
        else
            isEscaped = false;
        prevPrevC = prevC;
        
        
        switch(state)
        {   
            case IDLE:
            {
                text = "";
                if(c == '/')
                {
                    text = "/";
                    state = COMMENT1;
                }
                else if(c == ' ' || c == '\t')
                {
                    state = SPACES;
                }
                else if(c == '\'')
                {
                    state = ESCAPED_CHAR;
                }
                else if(c == '"')
                {
                    state = STRING;
                }
                // An '->' token?
                else if(c == '-')
                {
                    state = MINUS;
                }
                else if(isSpecialChar(c))
                {
                    pushToken(c, Token::WORD, lineNr);
                }
                else if(c == '\n')
                {
                    state = IDLE;
                }
                else
                {
                    text = c;
                    state = WORD;
                }
            };break;
            case MINUS:
            {
                if(c == '>')
                {
                    pushToken("->", Token::WORD, lineNr);
                }
                else
                {
                    pushToken(c, Token::WORD, lineNr);
                }
                state = IDLE;
            };break;
            case COMMENT1:
            {
                if(c == '*')
                {
                    text += c;
                    state = MULTI_COMMENT;
                    
                }
                else if(c == '/')
                {
                    text += c;
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
                    pushToken(text, Token::COMMENT, lineNr);
                    text = "";
                }
                else if(prevC == '*' && c == '/')
                {
                    text += c;
                    pushToken(text, Token::COMMENT, lineNr);
                    state = IDLE;
                }
                else
                {
                    text += c;
                }
            };break;
            case COMMENT:
            {
                if(c == '\n')
                {
                    pushToken(text, Token::COMMENT, lineNr);
                    state = IDLE;
                }
                else
                    text += c;
                    
            };break;
            case SPACES:
            {
                if(c == ' ' || c == '\t')
                {
                                                              
                }
                else
                {
                    i--;
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
                    text += c;
                    if(c == '>')
                    {
                        pushToken(text, Token::COMMENT, lineNr);
                        state = IDLE;
                    }
                }
            };break;
            case ESCAPED_CHAR:
            {
                text += c;
                if(!isEscaped && c == '\'')
                {
                    pushToken(text, Token::STRING, lineNr);
                    state = IDLE;
                }
            };break;
            case INC_STRING:
            {
                if(!isEscaped && c == '\n')
                {
                    i--;
                    pushToken(text, Token::STRING, lineNr);
                    state = IDLE;
                }
                else
                {
                    text += c;
                    if(!isEscaped && c == '>')
                    {
                        pushToken(text, Token::STRING, lineNr);
                        state = IDLE;
                    }
                }
            };break;
            case STRING:
            {
                text += c;
                if(!isEscaped && c == '"')
                {
                    pushToken(text, Token::STRING, lineNr);
                    state = IDLE;
                }
                  
            };break;
            case WORD:
            {
                if(isSpecialChar(c) || c == ' ' || c == '\t' || c == '\n')
                {
                    i--;

                    if(QChar(text[0]).isDigit())
                        pushToken(text, Token::NUMBER, lineNr);
                    else
                    {
                     
                        if(isKeyword(text))
                            pushToken(text, Token::KEYWORD, lineNr);
                        else
                            pushToken(text, Token::WORD, lineNr);
                    }

                    state = IDLE;
                }
                else
                {
                    
                    text += c;
                }
                
            };break;
        }
        prevC = c;
    }

    
}




