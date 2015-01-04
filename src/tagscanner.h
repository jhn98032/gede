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
        
        QString className;
        QString m_name;
        QString filepath;
        enum { TAG_FUNC, TAG_VARIABLE} type;
        QString m_signature;
        int lineNo;
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

