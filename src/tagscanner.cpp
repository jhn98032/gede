#include "tagscanner.h"

#include "log.h"
#include "util.h"

#include <QProcess>
#include <QDebug>


static const char ETAGS_CMD[] = "ctags";
static const char ETAGS_ARGS[] = "  -f - --excmd=number --fields=+nmsSk";


void Tag::dump()
{
    qDebug() << "/------------";
    qDebug() << "Name: " << name;
    qDebug() << "Filepath: " << filepath;
    if(TAG_VARIABLE == type)
        qDebug() << "Type: " << " variable";
    else if(TAG_FUNC == type)
        qDebug() << "Type: " << " function";


    qDebug() << "Sig: " << signature;
    qDebug() << "Line: " <<lineno;
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


int TagScanner::scan(QString filepath)
{
    int n = -1;
    QProcess proc;
    QString etagsCmd;
    etagsCmd = ETAGS_ARGS;
    etagsCmd += " ";
    etagsCmd += filepath;
    QString name = ETAGS_CMD;
    QStringList argList;
    argList = etagsCmd.split(' ',  QString::SkipEmptyParts);

    //qDebug() << argList;
    proc.start(name, argList, QProcess::ReadWrite);

    if(!proc.waitForStarted())
    {
        errorMsg("Failed to start program '%s'", ETAGS_CMD);
        infoMsg("ctags can be installed on ubuntu/debian using command:");
        infoMsg("# apt-get install exuberant-ctags");
        return -1;
    }
    proc.waitForFinished();


    QByteArray output = proc.readAllStandardOutput();
    parseOutput(output);


    // Get standard output
    output = proc.readAllStandardError();
    QString all = output;
    if(!all.isEmpty())
    {
        QStringList outputList = all.split('\n', QString::SkipEmptyParts);
        for(int r = 0;r < outputList.size();r++)
        {
            errorMsg("%s", stringToCStr(outputList[r]));
        } 
    }

    n = proc.exitCode();
    return n;
}

int TagScanner::parseOutput(QByteArray output)
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


            qDebug() << "=";


            if(colList.size() < 5)
            {

                errorMsg("Failed to parse output from ctags (%d)", colList.size());
            }
            else
            {

                Tag tag;

                tag.name = colList[0];
                tag.filepath = colList[1];
                QString type = colList[3];
                if(type == "v")
                    tag.type = Tag::TAG_VARIABLE;
                else if(type == "f")
                    tag.type = Tag::TAG_FUNC;
                else
                {
                    tag.type = Tag::TAG_VARIABLE;
                    errorMsg("Unknown type returned from ctags");
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
                        qDebug() << '|' << fieldName << '|' << fieldData << '|';

                        if(fieldName == "signature")
                            tag.signature = fieldData;
                        else if(fieldName == "line")
                            tag.lineno = fieldData.toInt();
                    }
                }

                m_list.push_back(tag);
            }
        }
    }

    return n;
}


void TagScanner::dump()
{
    for(int i = 0;i < m_list.size();i++)
    {
        Tag &tag =   m_list[i];
        tag.dump();
    }
}




