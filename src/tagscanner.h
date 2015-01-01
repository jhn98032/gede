#ifndef FILE_TAGS_H
#define FILE_TAGS_H

#include <QString>
#include <QList>


struct Tag
{
    public:
        void dump();

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

        int scan(QString filepath);
        void dump();

    private:
        int parseOutput(QByteArray output);


        QList <Tag> m_list;
};


#endif

