/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE__GDBMI_VALUE_PARSER_H
#define FILE__GDBMI_VALUE_PARSER_H

#include "tree.h"
#include <QList>
#include "com.h"


class GdbMiParser
{
    public:
    
    GdbMiParser(){};

    static void parseVariableData(TreeNode *thisNode, QList<Token*> *tokenList);
    static QList<Token*> tokenizeVarString(QString str);

};


#endif // FILE__GDBMI_VALUE_PARSER_H

