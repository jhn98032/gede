#include "tagscanner.h"

#include "log.h"
#include "util.h"

#include <QMessageBox>
#include <QProcess>
#include <QDebug>


static const char ETAGS_CMD[] = "ctags";
static const char ETAGS_ARGS[] = "  -f - --excmd=number --fields=+nmsSk";

Tag::Tag()
: m_signature("()")
{
}


QString Tag::getLongName() const
{
    QString longName;
    if(this->className.isEmpty())
        longName = m_name;
    else
        longName = this->className + "::" + m_name;
    longName += this->m_signature;
    return longName;
}


void Tag::dump() const
{
    qDebug() << "/------------";
    qDebug() << "Name: " << m_name;
    qDebug() << "Class: " << className;
    qDebug() << "Filepath: " << filepath;
    if(TAG_VARIABLE == type)
        qDebug() << "Type: " << " variable";
    else if(TAG_FUNC == type)
        qDebug() << "Type: " << " function";


    qDebug() << "Sig: " << m_signature;
    qDebug() << "Line: " <<lineNo;
    qDebug() << "\\------------";

}

/**
 *-------------------------------------------------------------
 *
 *
 *
 *
 *
 *
 *-------------------------------------------------------------
 */

TagScanner::TagScanner()
{

}

TagScanner::~TagScanner()
{

}

int TagScanner::execProgram(QString name, QStringList argList,
                            QByteArray *stdoutContent,
                            QByteArray *stderrContent)
{

    int n = -1;
    QProcess proc;

    proc.start(name, argList, QProcess::ReadWrite);

    if(!proc.waitForStarted())
    {
        return -1;
    }
    proc.waitForFinished();

    if(stdoutContent)
        *stdoutContent =  proc.readAllStandardOutput();

    // Get standard output
    if(stderrContent)
        *stderrContent = proc.readAllStandardError();
    
    n = proc.exitCode();
    return n;


}


void TagScanner::init()
{

    // Check if ctags exists?
    QStringList argList;
    argList.push_back("--version");
    QByteArray stdoutContent;
    int n = execProgram(ETAGS_CMD, argList, &stdoutContent, NULL);
    QStringList outputList = QString(stdoutContent).split('\n');
    for(int u = 0;u < outputList.size();u++)
    {
        debugMsg("ETAGS: %s", stringToCStr(outputList[u]));
    }
    if(n)
    {
        QString msg;

        msg.sprintf("Failed to start program '%s'\n", ETAGS_CMD);
        msg += "ctags can be installed on ubuntu/debian using command:\n";
        msg +  "\n";
        msg += " apt-get install exuberant-ctags";

        QMessageBox::warning(NULL,
                    "Failed to start ctags",
                    msg);
        m_ctagsExist = false;
    }
    else
        m_ctagsExist = true;
}
 

int TagScanner::scan(QString filepath, QList<Tag> *taglist)
{
    if(!m_ctagsExist)
        return 0;

    QString etagsCmd;
    etagsCmd = ETAGS_ARGS;
    etagsCmd += " ";
    etagsCmd += filepath;
    QString name = ETAGS_CMD;
    QStringList argList;
    argList = etagsCmd.split(' ',  QString::SkipEmptyParts);

    QByteArray stdoutContent;
    QByteArray stderrContent;
    int rc = execProgram(ETAGS_CMD, argList,
                            &stdoutContent,
                            &stderrContent);

    parseOutput(stdoutContent, taglist);

    // Display stderr
    QString all = stderrContent;
    if(!all.isEmpty())
    {
        QStringList outputList = all.split('\n', QString::SkipEmptyParts);
        for(int r = 0;r < outputList.size();r++)
        {
            errorMsg("%s", stringToCStr(outputList[r]));
        } 
    }

    return rc;
}

int TagScanner::parseOutput(QByteArray output, QList<Tag> *taglist)
{
    int n = 0;
    QList<QByteArray> rowList = output.split('\n');

    /*
       for(int rowIdx = 0;rowIdx < rowList.size();rowIdx++)
       {
       qDebug() << rowList[rowIdx];
       }
     */        

    for(int rowIdx = 0;rowIdx < rowList.size();rowIdx++)
    {
        QByteArray row = rowList[rowIdx];
        if(!row.isEmpty())
        {
            QList<QByteArray> colList = row.split('\t');

            if(colList.size() < 5)
            {

                errorMsg("Failed to parse output from ctags (%d)", colList.size());
            }
            else
            {

                Tag tag;

                tag.m_name = colList[0];
                tag.filepath = colList[1];
                QString type = colList[3];
                if(type == "v")
                    tag.type = Tag::TAG_VARIABLE;
                else if(type == "f")
                    tag.type = Tag::TAG_FUNC;
                else
                {
                    tag.type = Tag::TAG_VARIABLE;
                    //debugMsg("Unknown type (%s) returned from ctags", stringToCStr(type));
                }    
                for(int colIdx = 4;colIdx < colList.size();colIdx++)
                {
                    QString field = colList[colIdx];
                    int div = field.indexOf(':');
                    if(div == -1)
                        errorMsg("Failed to parse output from ctags (%d)", colList.size());
                    else
                    {
                        QString fieldName = field.left(div);
                        QString fieldData = field.mid(div+1);
                        // qDebug() << '|' << fieldName << '|' << fieldData << '|';

                        if(fieldName == "class")
                            tag.className = fieldData;
                        if(fieldName == "signature")
                        {
                            tag.m_signature = fieldData;
                        }
                        else if(fieldName == "line")
                            tag.lineNo = fieldData.toInt();
                    }
                }

                taglist->push_back(tag);
            }
        }
    }

    return n;
}


void TagScanner::dump(const QList<Tag> &taglist)
{
    for(int i = 0;i < taglist.size();i++)
    {
        const Tag &tag = taglist[i];
        tag.dump();
    }
}




