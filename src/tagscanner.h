#ifndef FILE_TAGS_H
#define FILE_TAGS_H

#include <QString>
#include <QList>



class Tag
{
    public:
        Tag();
        void dump() const;

        QString getLongName() const;
        QString getSignature() const { return m_signature; };
        void setSignature(QString signature) { m_signature = signature; };
        void setLineNo(int lineNo) { m_lineNo = lineNo;};
        int getLineNo() const { return m_lineNo; };
        
        QString className;
        QString m_name;
        QString filepath;
        enum { TAG_FUNC, TAG_VARIABLE} type;
private:
        QString m_signature;
        int m_lineNo;
};


class TagScanner
{
    public:

        TagScanner();
        ~TagScanner();

        void init();

        int scan(QString filepath, QList<Tag> *taglist);
        void dump(const QList<Tag> &taglist);

    private:
        int parseOutput(QByteArray output, QList<Tag> *taglist);


    static int execProgram(QString name, QStringList argList,
                            QByteArray *stdoutContent,
                            QByteArray *stderrContent);


        bool m_ctagsExist;
};


#endif

