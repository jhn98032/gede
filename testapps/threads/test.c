#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void func()
{
    volatile int a = 1;
    a++;
    a++;
    a++;
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



typedef enum {CUSTOM_ENUM1, CUSTOM_ENUM2} CustomEnum;
int main(int argc,char *argv[])
{
    volatile int j = 0;
    pthread_t th;

    pthread_create(&th, NULL, thread_func, 0);

    while(1)
    {
        j++;
        j++;
        j++;
        usleep(10*1000);
    }
    return 0;
}

