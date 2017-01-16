#include <stdio.h>
#include "func.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int i;
    int loops;

    if(argc > 1)
       loops = atoi(argv[1]);

    for(i = 0;i < loops;i++)
    {
        int a;
        const char* str;

        a = i;
        str = my_function(a);

        printf("a:%d str:%s\n", a, str);
    }


    return 0;
}


