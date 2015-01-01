#ifndef FILE_TAGS_H
#define FILE_TAGS_H

#include <QString>
#include <QList>


struct Tag
{
    public:
        void dump() const;

        QString className;
        QString name;
        QString filepath;
        enum { TAG_FUNC, TAG_VARIABLE} type;
        QString signature;
        int lineno;
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

