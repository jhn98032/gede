
#include "tagscanner.h"
#include "log.h"

#include <QtGlobal>
#include <QCoreApplication>
int dummy;

    
void dummyfunc()
{
    dummy++;

}

int main(int argc, char *argv[])
{
    Settings cfg;
    QString filename = "tagtest.cpp";
    QCoreApplication app(argc,argv);
    TagScanner scanner;

    for(int i = 1;i < argc;i++)
    {
        const char *curArg = argv[i];
        if(curArg[0] != '-')
            filename = curArg;
    }
    scanner.init(&cfg);

    

    QList<Tag> taglist;
    if(scanner.scan(filename, &taglist))
    {
        errorMsg("Failed to scan"); 
        return 1;
    }

    scanner.dump(taglist);

    return 0;
}


