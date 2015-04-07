#ifndef FILE__GDBMI_VALUE_PARSER_H
#define FILE__GDBMI_VALUE_PARSER_H

#include "tree.h"
#include <QList>
#include "com.h"


class GdbMiParser
{
    public:
    
    GdbMiParser(){};

    static int parseVariableData(TreeNode *thisNode, QList<Token*> *tokenList);
    static QList<Token*> tokenizeVarString(QString str);

};


#endif // FILE__GDBMI_VALUE_PARSER_H

