#include <stdio.h>

void func()
{
    volatile int a;
    a++;
    a++;
    a++;
}

int main(int argc,char *argv[])
{
    struct
    {
        int a;
        struct
        {
            int b;
        }s2;
    }s; 
    char *varStr;
    enum {ENUM1, ENUM2}varEnum;
    s.a = 0;
    varEnum = ENUM1;
    varEnum = ENUM2;
    while(1)
    {
        func();
        s.a++;
        s.s2.b++;
        s.s2.b++;
        s.a++;
        
    }
    return 0;
}

