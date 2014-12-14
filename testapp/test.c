// FIRST_ROW
#include <stdio.h>
#include <unistd.h>
#include "subdir/subfile.h"
#include <pthread.h>

// Single row comment

/**
  Multiline comment
  */

void testfunc(int v)
{
    int i = 2/1;
    while(v--)
    {
        i--;
    }
}

void *thread_func(void *ptr)
{
    unsigned int i = 0;
    while(1)
     {
         i++;
         usleep(1000);
     }
    return NULL;
}

void sleep_forever()
{
    int a = 0;

    while(1)
    {
        usleep(1000);
        a++;
        a++;
    }
}

int main(int argc, char*argv[])
{
    double k = 0.0;
    int i = 0;
    int j = 0;
     pthread_t th;
    struct {
        int a;
        int b;
    }local_struct;
    char *str = "hej";

    local_struct.a = 3;

    printf("Hej1\n");
    printf("Hej2\n");
    printf("Hej3\n");
    printf("Hej4\n");





    pthread_create(&th, NULL, thread_func, 0);



    subfunc(1);
    testfunc(10);

    for(i = 1;i < argc;i++)
    {
        printf(">%s<\n", argv[i]);
    }
    argc--;
    while(1)
    {
        k += 0.12;
        local_struct.a++;
        i++;
        j++;
        j++;
  // sleep_forever();
        usleep(10*1000);
    }
    return 0;
}
// LAST_ROW
