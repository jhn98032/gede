/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "gdbmiparser.h"
#include "util.h"
#include "log.h"
#include <assert.h>



/**
 * @brief Creates tokens from a single GDB output row.
 */
QList<Token*> GdbMiParser::tokenizeVarString(QString str)
{
    enum { IDLE, BLOCK, BLOCK_COLON, STRING, VAR} state = IDLE;
    QList<Token*> list;
    Token *cur = NULL;
    QChar prevC = ' ';
    
    if(str.isEmpty())
        return list;

    for(int i = 0;i < str.size();i++)
    {
        QChar c = str[i];
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
            case STRING:
            {
                if(prevC != '\\' && c == '\\')
                {
                }
                else if(prevC == '\\')
                {
                    if(c == 'n')
                        cur->text += '\n';
                    else
                        cur->text += c;
                }
                else if(c == '"')
                    state = IDLE;
                else
                    cur->text += c;
            };break;
            case BLOCK_COLON:
            case BLOCK:
            {
                if(prevC != '\\' && c == '\\')
                {
                }
                else if(prevC == '\\')
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
int GdbMiParser::parseVariableData(TreeNode *thisNode, QList<Token*> *tokenList)
{
    Token *token;
    TreeNode *childNode = NULL;
    int rc = 0;

    if(tokenList->isEmpty())
        return -1;
       
    // Take the first item
    token = tokenList->takeFirst();
    assert(token != NULL);
    if(token == NULL)
        return -1;
        
    if(token->getType() == Token::KEY_LEFT_BRACE)
    {
        
        do
        {
            // Double braces?
            if(tokenList->isEmpty())
                return -1;
            Token *token2 = tokenList->first();
                
            if(token2->getType() == Token::KEY_LEFT_BRACE)
            {
                
                rc = parseVariableData(thisNode, tokenList);

                token = tokenList->takeFirst();
            }
            else
            {
            
            // Get name
            QString name;
            if(tokenList->isEmpty())
                return -1;
            Token *nameTok = tokenList->takeFirst();
            assert(nameTok != NULL);
            name = nameTok->getString();


            // Is it a "static varType" type?
            if(tokenList->isEmpty())
                return -1;
            Token *extraNameTok = tokenList->first();
            if(extraNameTok->getType() == Token::VAR)
            {
                extraNameTok = tokenList->takeFirst();
                name += " " + extraNameTok->getString();
            }
        
            // Get equal sign
            if(tokenList->isEmpty())
                return -1;
            Token *eqToken = tokenList->first();
            assert(eqToken != NULL);
            if(eqToken->getType() == Token::KEY_EQUAL)
            {
                eqToken = tokenList->takeFirst();

                // Create treenode
                childNode = new TreeNode;
                childNode->setName(name);
                thisNode->addChild(childNode);

                // Get variable data
                rc = parseVariableData(childNode, tokenList);

                // End of the data
                token = tokenList->takeFirst();
            }
            else if(eqToken->getType() == Token::KEY_COMMA)
            {
                token = tokenList->isEmpty() ? NULL : tokenList->takeFirst();
            }
            else if(eqToken->getType() == Token::KEY_RIGHT_BRACE)
            {
                if(thisNode->getChildCount() == 0)
                    thisNode->setData(nameTok->getString());
                // Triggered by for example: "'{','<No data fields>', '}'"
                token = tokenList->isEmpty() ? NULL : tokenList->takeFirst();
            }
            else
            {
                errorMsg("Unknown token. Expected '=', Got:'%s'", stringToCStr(eqToken->getString()));

                // End of the data
                token = tokenList->isEmpty() ? NULL : tokenList->takeFirst();
            }
            }
            
        }while(token != NULL && token->getType() == Token::KEY_COMMA);

        //
        if(token == NULL)
            errorMsg("Unexpected end of token");
        else if (token->getType() != Token::KEY_RIGHT_BRACE)
            errorMsg("Unknown token. Expected '}', Got:'%s'", stringToCStr(token->getString()));
    }
    else
    {
        QString valueStr;
        QString defValueStr = token->getString();
        thisNode->setAddress(defValueStr.toLongLong(0,0));


        // Was the previous token only an address and the next token is the actual data? (Eg: '0x0001 "string"' )
        if(tokenList->isEmpty())
            return -1;
        Token *nextTok = tokenList->first();
        while( nextTok->getType() == Token::VAR || nextTok->getType() == Token::C_STRING)
        {
            nextTok = tokenList->takeFirst();

            QString addrStr = valueStr;
            thisNode->setAddress(addrStr.toInt(0,0));
            if(nextTok->getType() == Token::C_STRING)
                valueStr = "\"" + nextTok->getString() + "\"";
            else
            {
                if(valueStr.isEmpty())
                    valueStr = nextTok->getString();
            }
            nextTok = tokenList->isEmpty() ? NULL : tokenList->first();
            if(nextTok == NULL)
                break;
        }
        if(valueStr.isEmpty())
            valueStr = defValueStr;
        thisNode->setData(valueStr);
    }
    
    return rc;
}

