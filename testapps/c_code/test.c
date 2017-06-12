#include <stdio.h>

int main(int argc,char *argv[])
{
    int int_a;
    enum {ENUM1,ENUM2} enum_a = ENUM1;

    enum_a = ENUM1;
    enum_a = ENUM2;
    
    if(enum_a == ENUM2)
    {

    }

    return 0;
}

