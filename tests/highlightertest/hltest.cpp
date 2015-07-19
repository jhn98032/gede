
#include "syntaxhighlighter.h"
#include "log.h"
#include "util.h"

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
    SyntaxHighlighter scanner;
    const char *inputFilename = NULL;

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
    if(inputFilename == NULL)
        return dumpUsage();

    // Open file
    QFile file(inputFilename);
    if(!file.open(QIODevice::ReadOnly  | QIODevice::Text))
    {
        printf("Unable to open %s\n", inputFilename);
        return 1;
    }

    // Read entire content
    QString text;
    while (!file.atEnd())
    {
         QByteArray line = file.readLine();
         text += line;
    }


    scanner.colorize(text);

    for(unsigned int rowIdx = 0;rowIdx < scanner.getRowCount();rowIdx++)
    {
        QVector<TextField*> colList = scanner.getRow(rowIdx);
        printf("%3d | ", rowIdx);
        for(int colIdx = 0; colIdx < colList.size();colIdx++)
        {
            TextField* field = colList[colIdx];
            printf("'%s' ", stringToCStr(field->m_text));
        }
        printf("\n");
    }
    
    
    
    return 0;
}


