#include <stdio.h>

#define YELLOW_CODE "\033[1;33m"
#define GREEN_CODE  "\033[1;32m"
#define RED_CODE    "\033[1;31m"
#define NO_CODE     "\033[1;0m"


int main(int argc,char *argv[])
{
    char c;
    int i;
    for(i = 0;i < 3;i++)
    {
        printf(RED_CODE "RED" NO_CODE "\n");
        printf(GREEN_CODE "GREEN" NO_CODE "\n");
        printf(YELLOW_CODE "YELLOW" NO_CODE "\n");
        printf("DEFAULT"  "\n");
        printf("%d Input:", i);
        do
        {
            c = getc(stdin);
            printf("Got: '%d'\n", (int)c);
            if(c == -1)
                return 1;
        } while(c != '\n');
        
        
    }
    return 0;
}

