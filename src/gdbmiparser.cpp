/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "gdbmiparser.h"
#include "util.h"
#include "log.h"
#include <assert.h>
#include "core.h"



/**
 * @brief Creates tokens from a single GDB output row.
 */
QList<Token*> GdbMiParser::tokenizeVarString(QString str)
{
    enum { IDLE, BLOCK, BLOCK_COLON, STRING, VAR, CHAR} state = IDLE;
    QList<Token*> list;
    Token *cur = NULL;
    QChar prevC = ' ';
    bool isEscaped = false;

    if(str.isEmpty())
        return list;

    for(int i = 0;i < str.size();i++)
    {
        QChar c = str[i];

        if(c == '\\' && prevC == '\\')
        {
        }
        else if(prevC == '\\')
            isEscaped = true;
        else if(c == '\\')
        {
            isEscaped = false;
            prevC = c;
            continue;
        }
        else
            isEscaped = false;
        

        switch(state)
        {
            case IDLE:
            {
                if(c == '"')
                {
                    cur = new Token(Token::C_STRING);
                    list.push_back(cur);
                    state = STRING;
                }
                else if(c == '\'')
                {
                    cur = new Token(Token::C_CHAR);
                    list.push_back(cur);
                    state = CHAR;
                }
                else if(c == '<')
                {
                    cur = new Token(Token::VAR);
                    list.push_back(cur);
                    cur->text += c;
                    state = BLOCK;
                }
                else if(c == '(')
                {
                    cur = new Token(Token::VAR);
                    list.push_back(cur);
                    cur->text += c;
                    state = BLOCK_COLON;
                }
                else if(c == '=' || c == '{' || c == '}' || c == ',' ||
                    c == '[' || c == ']' || c == '+' || c == '^' ||
                    c == '~' || c == '@' || c == '&' || c == '*')
                {
                    Token::Type type = Token::UNKNOWN;
                    if(c == '=')
                        type = Token::KEY_EQUAL;
                    if(c == '{')
                        type = Token::KEY_LEFT_BRACE;
                    if(c == '}')
                        type = Token::KEY_RIGHT_BRACE;
                    if(c == '[')
                        type = Token::KEY_LEFT_BAR;
                    if(c == ']')
                        type = Token::KEY_RIGHT_BAR;
                    if(c == ',')
                        type = Token::KEY_COMMA;
                    if(c == '^')
                        type = Token::KEY_UP;
                    if(c == '+')
                        type = Token::KEY_PLUS;
                    if(c == '~')
                        type = Token::KEY_TILDE;
                    if(c == '@')
                        type = Token::KEY_SNABEL;
                    if(c == '&')
                        type = Token::KEY_AND;
                    if(c == '*')
                        type = Token::KEY_STAR;
                    cur = new Token(type);
                    list.push_back(cur);
                    cur->text += c;
                    state = IDLE;
                }
                else if( c != ' ')
                {
                    cur = new Token(Token::VAR);
                    list.push_back(cur);
                    cur->text = c;
                    state = VAR;
                }
                
            };break;
            case CHAR:
            {
                if(isEscaped)
                {
                    cur->text += '\\';
                    cur->text += c;
                }
                else if(c == '\'')
                {
                    state = IDLE;
                }
                else
                    cur->text += c;
            };break;
            case STRING:
            {
                if(isEscaped)
                {
                    cur->text += '\\';
                    cur->text += c;
                }
                else if(c == '"')
                {
                    state = IDLE;
                }
                else
                    cur->text += c;
            };break;
            case BLOCK_COLON:
            case BLOCK:
            {
                if(isEscaped)
                {
                    if(c == 'n')
                        cur->text += '\n';
                    else
                        cur->text += c;
                }
                else if((c == '>' && state == BLOCK) ||
                        (c == ')' && state == BLOCK_COLON))
                {
                    cur->text += c;
                    state = IDLE;
                }
                else
                    cur->text += c;
            };break;
            case VAR:
            {
                if(c == ' ' || c == '=' || c == ',' || c == '{' || c == '}')
                {
                    i--;
                    cur->text = cur->text.trimmed();
                    state = IDLE;
                }
                else
                    cur->text += c;
            };break;
            
        }
        prevC = c;
    }
    if(cur)
    {
        if(cur->getType() == Token::VAR)
            cur->text = cur->text.trimmed();
    }
    return list;
}



/**
 * @brief Parses a variable assignment block.
 */
int GdbMiParser::parseVariableData(CoreVar *var, QList<Token*> *tokenList)
{
    Token *token;
    int rc = 0;

    if(tokenList->isEmpty())
        return -1;
       
    // Take the first item
    token = tokenList->takeFirst();
    assert(token != NULL);
    if(token == NULL)
        return -1;


    if(token->getType() == Token::KEY_LEFT_BAR)
    {
        QString data;
        data = "[";
        while(!tokenList->isEmpty())
        {
            token = tokenList->takeFirst();
            if(token)
                data += token->getString();
        }
        var->setData(data);
        return 0; 
    }

    if(token->getType() == Token::KEY_LEFT_BRACE)
    {
        
    }
    else
    {
        QString valueStr;
        QString defValueStr = token->getString();
        var->setAddress(defValueStr.toLongLong(0,0));


        // Was the previous token only an address and the next token is the actual data? (Eg: '0x0001 "string"' )
        if(tokenList->isEmpty())
        {
            if(token->getType() == Token::C_STRING)
                defValueStr = "\"" + token->getString() + "\"";
            var->setData(defValueStr);
            return 0;
        }
        Token *nextTok = tokenList->first();
        if(nextTok->getType() == Token::C_CHAR)
        {
            var->setData(defValueStr);
        }
        else
        {
        while( nextTok->getType() == Token::VAR || nextTok->getType() == Token::C_STRING)
        {
            nextTok = tokenList->takeFirst();

            if(nextTok->getType() == Token::C_STRING)
                valueStr = "\"" + nextTok->getString() + "\"";
            else
            {
                valueStr = nextTok->getString();
                if(valueStr.startsWith("<"))
                    valueStr = defValueStr + " " + valueStr;
            }
            nextTok = tokenList->isEmpty() ? NULL : tokenList->first();
            if(nextTok == NULL)
                break;
        }
        if(valueStr.isEmpty())
            valueStr = defValueStr;
        var->setData(valueStr);
        }
    }
    
    return rc;
}

