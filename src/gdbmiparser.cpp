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
    Token *token = tokenList->takeFirst();
    TreeNode *childNode = NULL;
    int rc = 0;
    
    assert(token != NULL);
    if(token == NULL)
        return -1;
        
    if(token->getType() == Token::KEY_LEFT_BRACE)
    {
        
        do
        {
            // Double braces?
            Token *token2 = tokenList->first();
            if(token2 == NULL)
                return -1;
                
            if(token2->getType() == Token::KEY_LEFT_BRACE)
            {
                
                rc = parseVariableData(thisNode, tokenList);

                token = tokenList->takeFirst();
            }
            else
            {
            
            // Get name
            QString name;
            Token *nameTok = tokenList->takeFirst();
            assert(nameTok != NULL);
            if(nameTok == NULL)
                return -1;
            name = nameTok->getString();

            // Is it a "static varType" type?
            Token *extraNameTok = tokenList->first();
            if(extraNameTok == NULL)
                return -1;
            if(extraNameTok->getType() == Token::VAR)
            {
                extraNameTok = tokenList->takeFirst();
                name += " " + extraNameTok->getString();
            }
        
            // Get equal sign
            Token *eqToken = tokenList->first();
            assert(eqToken != NULL);
            if(eqToken == NULL)
                return -1;
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
        QString valueStr = token->getString();
        thisNode->setAddress(valueStr.toLongLong(0,0));

        // Was it only an address with data following the address? (Eg: '0x0001 "string"' )
        Token *nextTok = tokenList->first();
        if(nextTok == NULL)
            return -1;
        if( nextTok->getType() == Token::VAR || nextTok->getType() == Token::C_STRING)
        {
            nextTok = tokenList->takeFirst();

            QString addrStr = valueStr;
            thisNode->setAddress(addrStr.toInt(0,0));
            if(nextTok->getType() == Token::C_STRING)
                valueStr = "\"" + nextTok->getString() + "\"";
            else
                valueStr = nextTok->getString();

        }
        
        thisNode->setData(valueStr);
    }
    
    return rc;
}

