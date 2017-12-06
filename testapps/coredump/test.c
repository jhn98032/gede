#include <stdio.h>

void func()
{
    int *ptr = 0;
    volatile int a = 1;
    a++;
    a++;
    a++;

*ptr = 2;
}

typedef enum {CUSTOM_ENUM1, CUSTOM_ENUM2} CustomEnum;
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
    float f1;
    char *varStr;
    enum {ENUM1, ENUM2}varEnum;
    CustomEnum customEnum1;

    f1 = 0.0;
    s.a = 0;
    s.s2.b = 1;
    varEnum = ENUM1;
    varEnum = ENUM2;
    while(1)
    {
        f1 += 0.1;
        func();
        s.a++;
        s.s2.b++;
        s.s2.b++;
        s.a++;
        
    }
    return 0;
}

