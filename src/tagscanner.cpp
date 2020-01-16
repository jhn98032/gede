/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "tagscanner.h"

#include <QMessageBox>
#include <QProcess>
#include <QDebug>

#include "config.h"
#include "log.h"
#include "util.h"
#include "rusttagscanner.h"
#include "adatagscanner.h"


static bool g_ctagsExist = true;
static bool g_doneCtagCheck = false;
static QString g_ctagsCmd; //!< Name of executable
        

Tag::Tag()
 : m_lineNo(0)
{
}


QString Tag::getLongName() const
{
    QString longName;
    if(m_className.isEmpty())
        longName = m_name;
    else
        longName = m_className + "::" + m_name;
    longName += m_signature;
    return longName;
}


void Tag::dump() const
{
    qDebug() << "/------------";
    qDebug() << "Name: " << m_name;
    qDebug() << "Class: " << m_className;
    qDebug() << "Filepath: " << m_filepath;
    if(TAG_VARIABLE == m_type)
        qDebug() << "Type: " << " variable";
    else if(TAG_FUNC == m_type)
        qDebug() << "Type: " << " function";


    qDebug() << "Sig: " << m_signature;
    qDebug() << "Line: " << m_lineNo;
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
 : m_cfg(0)
{

}

void TagScanner::checkForCtags()
{
    // Only do check once
    if(g_doneCtagCheck)
        return;
    g_doneCtagCheck = true;
    
    // Check which executable to use
    g_ctagsExist = true;
    if(exeExists(ETAGS_CMD2))
        g_ctagsCmd = ETAGS_CMD2;
    else if(exeExists(ETAGS_CMD1))
        g_ctagsCmd = ETAGS_CMD1;
    else
        g_ctagsExist = false;

    // Found a executable?
    if(!g_ctagsExist)
    {
        QString msg;

        msg.sprintf("Failed to start program '%s/%s'\n", ETAGS_CMD1, ETAGS_CMD2);
        msg += "ctags can be installed on ubuntu/debian using command:\n";
        msg +=  "\n";
        msg += " apt-get install exuberant-ctags";

        QMessageBox::warning(NULL,
                    "Failed to start ctags",
                    msg);

    }
    else
    {
        // Check if ctags can startup?
        QStringList argList;
        argList.push_back("--version");
        QByteArray stdoutContent;
        int n = execProgram(g_ctagsCmd, argList, &stdoutContent, NULL);
        QStringList outputList = QString(stdoutContent).split('\n');
        for(int u = 0;u < outputList.size();u++)
        {
            debugMsg("ETAGS: %s", stringToCStr(outputList[u]));
        }
        if(n)
        {
            QString msg;

            msg.sprintf("Failed to start program '%s'\n", qPrintable(g_ctagsCmd));
        
            QMessageBox::warning(NULL,
                        "Failed to start ctags",
                        msg);
            g_ctagsExist = false;
        }
        else
        {
            infoMsg("Found ctags ('%s')", qPrintable(g_ctagsCmd));
            g_ctagsExist = true;
        }
    }

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


void TagScanner::init(Settings *cfg)
{
    m_cfg = cfg;

    checkForCtags();
    
}


/**
 * @brief Scans a sourcefile for tags.
 */
int TagScanner::scan(QString filepath, QList<Tag> *taglist)
{
    
    // Rust file?
    QString extension = getExtensionPart(filepath);
    if(extension.toLower() == RUST_FILE_EXTENSION)
    {
        RustTagScanner rs;
        rs.setConfig(m_cfg);
        return rs.scan(filepath, taglist);
    }
    if(extension.toLower() == ADA_FILE_EXTENSION)
    {
        AdaTagScanner rs;
        rs.setConfig(m_cfg);
        return rs.scan(filepath, taglist);
    }



    if(!g_ctagsExist)
        return 0;

    QString etagsCmd;
    etagsCmd = ETAGS_ARGS;
    etagsCmd += " ";
    etagsCmd += filepath;
    QString name = g_ctagsCmd;
    QStringList argList;
    argList = etagsCmd.split(' ',  QString::SkipEmptyParts);

    QByteArray stdoutContent;
    QByteArray stderrContent;
    int rc = execProgram(g_ctagsCmd, argList,
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
                tag.m_filepath = colList[1];
                QString type = colList[3];
                if(type == "v") // v = variable
                    tag.m_type = Tag::TAG_VARIABLE;
                else if(type == "f") // f = function
                {
                    tag.m_type = Tag::TAG_FUNC;
                    tag.setSignature("()");
                }
                else if(type == "s") // s = subroutine?
                {
                    tag.m_type = Tag::TAG_FUNC;
                    tag.setSignature("()");
                }
                else if(type == "p") // p = program?
                {
                    tag.m_type = Tag::TAG_FUNC;
                    tag.setSignature("()");
                }
                else
                {
                    tag.m_type = Tag::TAG_VARIABLE;
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
                            tag.m_className = fieldData;
                        if(fieldName == "signature")
                        {
                            tag.setSignature(fieldData);
                        }
                        else if(fieldName == "line")
                            tag.setLineNo(fieldData.toInt());
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




