/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

//#define ENABLE_DEBUGMSG

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
                    state = BLOCK;
                }
                else if(c == '(')
                {
                    cur = new Token(Token::VAR);
                    list.push_back(cur);
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
                    if(state == BLOCK_COLON)
                    {
                        int barIdx = cur->text.indexOf('|');
                        if(barIdx != -1)
                            cur->text = cur->text.mid(barIdx+1);
                    }
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


void GdbMiParser::setData(CoreVar *var, QString data)
{
    QVariant m_data = data;
    CoreVar::Type m_type;

    // A parent?
    if(data == "...")
    {
        m_data = "{...}";
        m_type = CoreVar::TYPE_UNKNOWN;
    }
    // String?
    else if(data.startsWith('"'))
    {
        if(data.endsWith('"'))
            data = data.mid(1, data.length()-2);
        m_data = data;
        m_type = CoreVar::TYPE_STRING;
    }
    // Character?
    else if(data.startsWith('\''))
    {
        if(data.endsWith('\''))
            data = data.mid(1, data.length()-2);
        else
            data = data.mid(1);
        
        if(data.startsWith("\\0"))
            m_data = (int)data.mid(2).toInt();
        else
            m_data = (int)data[0].toLatin1();
        
        m_type = CoreVar::TYPE_CHAR;
    }
    // Gdb Error message?
    else if(data.endsWith(">"))
    {
        m_data = data;
        m_type = CoreVar::TYPE_ERROR_MSG;
    }
    // Vector?
    else if(data.startsWith("["))
    {
        m_data = data;
        m_type = CoreVar::TYPE_UNKNOWN;
    }
    else if(data.length() > 0)
    {
        // Integer?
        if(data[0].isDigit() || data[0] == '-')
        {
            // Float?
            if(data.contains("."))
            {
                m_data = data;
                m_type = CoreVar::TYPE_FLOAT;
            }
            else // or integer?
            {
                if(data.startsWith("0x"))
                {
                    m_data = (qulonglong)data.toULongLong(0,0);
                    m_type = CoreVar::TYPE_HEX_INT;
                }
                else
                {
                    int firstSpacePos = data.indexOf(' ');
                    if(firstSpacePos != -1)
                        data = data.left(firstSpacePos);
                    m_data = data.toLongLong(0,0);
                    m_type = CoreVar::TYPE_DEC_INT;
                }
            }           
        }
        else
        {
            m_data = data;
            m_type = CoreVar::TYPE_ENUM;
        }
            
    }
    else
        m_type = CoreVar::TYPE_UNKNOWN;

    var->setData(m_type, m_data);
    
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
        setData(var, data);
        return 0; 
    }
    else if(token->getType() == Token::KEY_LEFT_BRACE)
    {
        QString str;
        while( !tokenList->isEmpty())
        {
            str += token->getString();
            token = tokenList->takeFirst();
        }
        str += token->getString();
        
        var->setData(CoreVar::TYPE_UNKNOWN, str);
        
    }
    else
    {
        QString firstTokenStr = token->getString();
            
        // Did we not get an address followed by the data? (Eg: '0x0001 "string"' )
        if(tokenList->isEmpty())
        {
            if(token->getType() == Token::C_STRING)
            {
                debugMsg("  String");
                var->setData(CoreVar::TYPE_STRING, token->getString());
            }
            else
            {
                debugMsg("  Var");
                
                setData(var, token->getString());
            }
            return 0;
        }
        Token *nextTok = tokenList->first();

        // A character (Eg: '90 'Z')
        if(nextTok->getType() == Token::C_CHAR)
        {
            var->setData(CoreVar::TYPE_CHAR, (int)token->getString().toInt());
        }
        else
        {
            QString valueStr;
            var->setPointerAddress(firstTokenStr.toLongLong(0,0));

            while( nextTok->getType() == Token::VAR || nextTok->getType() == Token::C_STRING)
            {
                nextTok = tokenList->takeFirst();

                if(nextTok->getType() == Token::C_STRING)
                {
                    var->setData(CoreVar::TYPE_STRING, nextTok->getString());
                    return 0;
                }
                else
                {
                    valueStr = nextTok->getString();
                    if(valueStr.startsWith("<"))
                        valueStr = firstTokenStr + " " + valueStr;
                }
                nextTok = tokenList->isEmpty() ? NULL : tokenList->first();
                if(nextTok == NULL)
                    break;
            }
            if(valueStr.isEmpty())
                valueStr = firstTokenStr;
            setData(var, valueStr);
        }
    }
    
    return rc;
}

