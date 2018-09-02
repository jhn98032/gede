/*
 * Copyright (C) 2018 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

// #define ENABLE_DEBUGMSG

#include "rusttagscanner.h"

#include <assert.h>
#include <stdio.h>
#include <QStringList>
#include <QFile>

#include "util.h"
#include "settings.h"
#include "log.h"


RustTagScanner::RustTagScanner()
    : m_cfg(NULL)

{
    QStringList keywordList = Settings::getDefaultRustKeywordList();
    for(int u = 0;u < keywordList.size();u++)
    {
        m_keywords[keywordList[u]] = true;
    }



}

RustTagScanner::~RustTagScanner()
{
    clearTokenList();
}

    
int RustTagScanner::scan(QString filepath, QList<Tag> *taglist)
{
    m_filepath = filepath;
    
    // Open file
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errorMsg("Failed to open '%s'", stringToCStr(filepath));
        return -1;
    }

    // Read file content
    QString text;
    while (!file.atEnd())
    {
         QByteArray line = file.readLine();
         text += line;
    }

    tokenize(text);

    parse(taglist);


    clearTokenList();


    return 0;
}


RustTagScanner::Token* RustTagScanner::pushToken(char c, Token::Type type, int lineNr)
{
    QString text(c);
    return pushToken(text, type, lineNr);
}


RustTagScanner::Token* RustTagScanner::pushToken(QString text, Token::Type type, int lineNr)
{
    Token *tok = new Token(lineNr, type);
    tok->setText(text);
    m_tokens.append(tok);
    return tok;
}



    
RustTagScanner::Token* RustTagScanner::popToken()
{
    if(m_tokens.isEmpty())
        return NULL;
    Token* tok = m_tokens.takeFirst();
    return tok;
}

RustTagScanner::Token* RustTagScanner::peekToken()
{
    if(m_tokens.isEmpty())
        return NULL;
    return m_tokens.first();
}



QString RustTagScanner::Token::toDesc()
{
    Token *tok = this;
    QString typeStr;
    switch(tok->getType())
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


/**
 * @brief Removes a token if it matches.
 * @return false if a token was poped. 
 */
bool RustTagScanner::eatToken(QString text)
{
   Token *tok = peekToken();
    if(!tok)
        return true;
    if(tok->getText() == text)
    {
        tok = popToken();
        delete tok;
        return false;
    }
    return true;
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
                    debugMsg("tok: %s", qPrintable(tok->toDesc()));
                    if(tok->getText() == "fn")
                        state = FN_KW;
                };break;
                case FN_KW:
                {
                    debugMsg("found: '%s' at L%d", qPrintable(tok->getText()), tok->getLineNr());
                    Tag tag;
                    tag.setLineNo(tok->getLineNr());
                    tag.m_name = tok->getText();
                    tag.m_filepath = m_filepath;
                    tag.m_type = Tag::TAG_FUNC;
                    state = IDLE;

                    // Parse signature
                    if(eatToken("("))
                    {
                    }
                    else
                    {
                        QString tokText;
                        QString signature;
                        signature += "(";
                        do
                        {
                            Token *tok2 = popToken();
                            if(tok2)
                            {
                                tokText = tok2->getText();
                                signature += tokText;
                                if(tokText == ",")
                                    signature += " ";
                                delete tok2;
                            }
                            else
                                tokText = "";
                            
                        }while(tokText != ")");

                        debugMsg("Found signature:%s", qPrintable(signature));
                        tag.setSignature(signature);
                    }

                    // Add the tag to the list
                    taglist->append(tag);
                
                };break;
                default:;break;
            }
            delete tok;
        }
    }while(tok);

}
    




void RustTagScanner::clearTokenList()
{
    for(int i = 0;i < m_tokens.size();i++)
    {
        Token* tok = m_tokens[i];
        delete tok;
    }
    m_tokens.clear();



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


    clearTokenList();
    

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




