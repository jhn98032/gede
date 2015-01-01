
#include "tagscanner.h"
#include "log.h"

int dummy;

void dummyfunc()
{
    dummy++;

}

int main(int argc, char *argv[])
{
    TagScanner scanner;

    Q_UNUSED(argc);
    Q_UNUSED(argv);

    if(scanner.scan("tagtest.cpp"))
        errorMsg("Failed to scan"); 

    scanner.dump();

    return 0;
}


