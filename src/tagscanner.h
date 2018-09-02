/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE_TAGS_H
#define FILE_TAGS_H

#include <QString>
#include <QList>
#include "settings.h"



class Tag
{
    public:
        Tag();
        void dump() const;

        QString getName() const { return m_name; };
        QString getLongName() const;
        QString getSignature() const { return m_signature; };
        void setSignature(QString signature) { m_signature = signature; };
        void setLineNo(int lineNo) { m_lineNo = lineNo;};
        int getLineNo() const { return m_lineNo; };
        QString getFilePath() const { return m_filepath; };
        QString getClassName() const { return m_className;};
        
        QString m_className;
        QString m_name;
        QString m_filepath;
        enum { TAG_FUNC, TAG_VARIABLE} m_type;
private:
        QString m_signature;
        int m_lineNo;
};


class TagScanner
{
    public:

        TagScanner();
        ~TagScanner();

        void init(Settings *cfg);

        int scan(QString filepath, QList<Tag> *taglist);
        void dump(const QList<Tag> &taglist);

    private:
        int parseOutput(QByteArray output, QList<Tag> *taglist);

        void checkForCtags();

    static int execProgram(QString name, QStringList argList,
                            QByteArray *stdoutContent,
                            QByteArray *stderrContent);


        Settings *m_cfg;
};


#endif

