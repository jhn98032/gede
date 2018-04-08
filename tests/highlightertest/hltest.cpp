
#include "syntaxhighlightercxx.h"
#include "syntaxhighlighterbasic.h"
#include "log.h"
#include "util.h"

#include <QtGlobal>
#if QT_VERSION < 0x050000
#include <QtGui/QApplication>
#endif
#include <QApplication>
#include <QFile>

int dumpUsage()
{
    printf("Usage: ./hltest SOURCE_FILE.c\n");
    printf("Description:\n");
    printf("  Dumps syntax highlight info for a source file\n");
    return 1;
}


int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    QString inputFilename;
    
    // Parse arguments
    for(int i = 1;i < argc;i++)
    {
        const char *curArg = argv[i];
        if(curArg[0] == '-')
            return dumpUsage();
        else
        {
            inputFilename = curArg;
        }
    }
    if(inputFilename.isEmpty())
        return dumpUsage();

    // Open file
    QFile file(inputFilename);
    if(!file.open(QIODevice::ReadOnly  | QIODevice::Text))
    {
        printf("Unable to open %s\n", qPrintable(inputFilename));
        return 1;
    }

    // Read entire content
    QString text;
    while (!file.atEnd())
    {
         QByteArray line = file.readLine();
         text += line;
    }

    Settings cfg;
    
    SyntaxHighlighter *scanner = NULL;
    if(inputFilename.endsWith(".bas"))
        scanner = new SyntaxHighlighterBasic();
    else
        scanner = new SyntaxHighlighterCxx();

    scanner->setConfig(&cfg);

    scanner->colorize(text);

    for(unsigned int rowIdx = 0;rowIdx < scanner->getRowCount();rowIdx++)
    {
        QVector<TextField*> colList = scanner->getRow(rowIdx);
        printf("%3d | ", rowIdx);
        for(int colIdx = 0; colIdx < colList.size();colIdx++)
        {
            TextField* field = colList[colIdx];
            printf("'\033[1;32m%s\033[1;0m' ", stringToCStr(field->m_text));
        }
        printf("\n");
    }
    delete scanner;
    

    return 0;
}


