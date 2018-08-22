#include "locator.h"

#include "util.h"
#include "log.h"
#include "core.h"
#include "mainwindow.h"


Location::Location(QString filename_, int lineNo_)
 :  filename(filename_)
    ,lineNo(lineNo_)
{
}

void Location::dump()
{
    qDebug() << "Filename:" << filename;
    qDebug() << "Lineno:  " << lineNo;
};

Locator::Locator(TagManager *mgr, QList<FileInfo> *sourceFiles)
    : m_mgr(mgr)
    ,m_sourceFiles(sourceFiles)
{
}

Locator::~Locator()
{

}

void Locator::setCurrentFile(QString filename)
{
    m_currentFilename = filename;
}

static bool validFunctionChar(QChar c)
{
    if(c.isDigit() || c.isLetter() || c == '.' || c == ':' || c == '_' || c == '(' || c == ')')
        return true;
    return false;
}


static QStringList tokenize(QString expr)
{
    expr = expr + ' ';
    QStringList tokenList;
    QString token;
    enum {IDLE, FUNC} state = IDLE;
    for(int i = 0;i < expr.size();i++)
    {
        QChar c = expr[i];
        switch(state)
        {
            case IDLE:
            {
                if(c.isSpace())
                {
                }
                else if(validFunctionChar(c))
                {
                    token = QString(c);
                    state = FUNC;
                }
                else if(c == '+' ||
                        c == '-' ||
                        c == '.')
                {
                    tokenList.append(QString(c));
                }
                else
                {
                    warnMsg("Parse error");
                }
                
            };break;
            case FUNC:
            {
                if(!validFunctionChar(c))
                {
                    tokenList.append(token);
                    state = IDLE;
                    i--;
                }
                else
                    token += c;
            };break;
        };
        

    }
    return tokenList;
}




bool isInteger(QString str)
{
    if(str.size() == 0)
        return false;
    if(str[0].isDigit())
        return true;
    return false;
}

QStringList Locator::findFile(QString defFilename)
{
    QStringList fileList;
    if(!defFilename.contains('/'))
    {
        for(int k = 0;k < m_sourceFiles->size();k++)
        {
            FileInfo &info = (*m_sourceFiles)[k];
            if(info.name == defFilename)
                fileList.append(info.fullName);

        }
    }
    return fileList;
}



QVector<Location> Locator::locate(QString expr)
{
    QVector<Location> list;
    QStringList tokenList = tokenize(expr);

    if(tokenList.size() == 1)
    {
        QString tok = tokenList[0];
        if(isInteger(tok))
        {
            Location loc = Location(m_currentFilename, tok.toInt());
            list += loc;
        }
        else
        {
            return locate(expr + " 0");
        }
            
    }
    else if(tokenList.size() >= 2)
    {
        QString tok;

        // A filename was specified? ("filename.cpp")
        tok = tokenList.takeFirst();
        QString defFilename;;
        if(!tok.endsWith(')') && !isInteger(tok))
        {
            defFilename = simplifyPath(tok);
            debugMsg("filename '%s' was specified", qPrintable(tok));
            
        }
        else
            tokenList.prepend(tok);

        if(!tokenList.isEmpty())
        {
            tok = tokenList.takeFirst();


            // Linenumber?
            if(isInteger(tok))
            {
                if(defFilename.isEmpty())
                    defFilename = m_currentFilename;
                 
                int lineNo = tok.toInt();
                debugMsg("Choosing %s:%d", qPrintable(defFilename), lineNo);

                
                QStringList fileList = findFile(defFilename);
                if(fileList.size() > 0)
                {
                    Location loc = Location(fileList[0], lineNo);
                    list += loc;
                }
            }
            else // function
            {
                if(tok.endsWith("()"))
                    tok = tok.left(tok.length()-2);
                QString funcName = tok;

                debugMsg("Looking for function '%s'", qPrintable(funcName));
            
                // Find the tag
                QList<Tag> tagList;
                m_mgr->lookupTag(funcName, &tagList);
                for(int i = 0;i < tagList.size();i++)
                {
                    Tag &tag = tagList[i];
                    tag.dump();
                    if(defFilename.isEmpty() || tag.getFilePath() == defFilename)
                    {
                        Location loc = Location(tag.getFilePath(), tag.getLineNo());
                        loc.dump();
                        list += loc;
                        
                    }
                }
                if(tokenList.size() == 2)
                {
                    QString op = tokenList[0];
                    int val = tokenList[1].toInt();
                    if(op == "-" || op == "+")
                    {
                        if(op == "-")
                            val = -val;


                        debugMsg("Adjusting line numbers with %d.", val);
                        // Updating lineno
                        for(int k = 0;k < list.size();k++)
                        {
                            Location *loc = &list[k];
                            debugMsg("%d -> %d", loc->lineNo, loc->lineNo+val);
                            loc->lineNo += val;
                        }

                    }
                    else
                        warnMsg("Unknown op '%s'.", qPrintable(op));
                    
                }
                else
                    warnMsg("Parse error (%d)", tokenList.size());
                    
                
            }
        }
    }

    
    
    return list;
}


