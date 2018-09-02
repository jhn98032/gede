#include "locator.h"

#include "util.h"
#include "log.h"
#include "core.h"
#include "mainwindow.h"
#include "qtutil.h"


Location::Location(QString filename_, int lineNo_)
 :  m_filename(filename_)
    ,m_lineNo(lineNo_)
{
}

void Location::dump()
{
    qDebug() << "Filename:" << m_filename;
    qDebug() << "Lineno:  " << m_lineNo;
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
    if(c.isDigit() || c.isLetter() || c == '.' || c == ':' || c == '_' || c == '(' || c == ')' || c == '~')
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
    if(fileList.isEmpty())
        warnMsg("File '%s' not found", qPrintable(defFilename));
        
    return fileList;
}

QStringList Locator::searchExpression(QString filename, QString expressionStart)
{
    debugMsg("%s('%s', '%s')", __func__, qPrintable(filename), qPrintable(expressionStart));
    
    QStringList list;
    for(int k = 0;k < m_sourceFiles->size();k++)
    {
        FileInfo &info = (*m_sourceFiles)[k];

        
        if(info.name != filename)
            continue;
            
        // Find the tag
        QList<Tag> tagList;
        m_mgr->getTags(info.fullName, &tagList);
        for(int i = 0;i < tagList.size();i++)
        {
            Tag &tag = tagList[i];
            if(tag.type == Tag::TAG_FUNC)
            {
                QString tagName = tag.getName() + "()";
                if(tagName.startsWith(expressionStart) || expressionStart.isEmpty())
                {
                    debugMsg("Found '%s'", qPrintable(tagName));
                    list.append(tagName);
                }
            }
        }
    }
    return list;
}


QStringList Locator::searchExpression(QString expressionStart)
{
    QStringList list;
    for(int k = 0;k < m_sourceFiles->size();k++)
    {
        FileInfo &info = (*m_sourceFiles)[k];
        if(info.name.startsWith(expressionStart))
            list.append(info.name);


        // Find the tag
        QList<Tag> tagList;
        m_mgr->getTags(info.fullName, &tagList);
        for(int i = 0;i < tagList.size();i++)
        {
            Tag &tag = tagList[i];
            if(tag.type == Tag::TAG_FUNC)
            {
            QString tagName = tag.getName() + "()";
            if(tagName.startsWith(expressionStart))
            {
                debugMsg("Found '%s'", qPrintable(tagName));
                list.append(tagName);
            }
            }
        }
    }
                
    return list;
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
                    if(defFilename.isEmpty() ||
                        tag.getFilePath().endsWith("/" + defFilename))
                    {
                        Location loc = Location(tag.getFilePath(), tag.getLineNo());
                        loc.dump();
                        list += loc;
                        
                    }
                }
                if(tokenList.size() >= 1)
                {
                    QString op = tokenList.takeFirst();
                    int val = 0;
                    if(!tokenList.isEmpty())
                        val = tokenList.takeFirst().toInt();
                        
                    if(op == "-" || op == "+")
                    {
                        if(op == "-")
                            val = -val;
                    }
                    else
                        val = op.toInt();
                        

                    debugMsg("Adjusting line numbers with %d.", val);
                    // Updating lineno
                    for(int k = 0;k < list.size();k++)
                    {
                        Location *loc = &list[k];
                        debugMsg("%d -> %d", loc->lineNo, loc->lineNo+val);
                        loc->m_lineNo += val;
                    }

                    
                }
                else
                    warnMsg("Parse error (%d)", tokenList.size());
                    
                
            }
        }
    }

    
    
    return list;
}


/**
 * @brief Search for a function in all files.
 * @param name   The name of the function (Eg: "main").
 * @return List of locations where the function is defined.
 */
QVector<Location> Locator::locateFunction(QString name)
{
    QVector<Location> list;
    for(int k = 0;k < m_sourceFiles->size();k++)
    {
        FileInfo &info = (*m_sourceFiles)[k];

        // Find the tag
        QList<Tag> tagList;
        m_mgr->getTags(info.fullName, &tagList);
        for(int i = 0;i < tagList.size();i++)
        {
            Tag &tag = tagList[i];
            if(tag.type == Tag::TAG_FUNC && tag.getName() == name)
            {
                list.append(Location(tag.getFilePath(), tag.getLineNo()));
            }
            
        }
    }
    return list;
}
 
