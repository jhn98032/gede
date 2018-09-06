#include <stdio.h>
#include <ctype.h>
#include <string.h>


#define YELLOW_CODE "\033[1;33m"
#define GREEN_CODE  "\033[1;32m"
#define RED_CODE    "\033[1;31m"
#define NO_CODE     "\033[1;0m"

#define INT_TO_STR_(i) #i
#define INT_TO_STR(i) INT_TO_STR_(i)

#define ANSI_ERASE_SCREEN  "\033[2J"
#define ANSI_CURSOR_HOME(r,c)  "\033[" INT_TO_STR(r) ";" INT_TO_STR(c) "H"

char* letterToStr(int c)
{
    static char rsp[3] = { '\0','\0','\0'};
    if(isalpha(c) || isdigit(c))
    {
        rsp[0] = (char)c;
        rsp[1] = '\0';
    }
    else if(c == '\n')
        strcpy(rsp, "\\n");
    return rsp;
}

int main(int argc,char *argv[])
{
    char c;
    int i;
    printf("erase me!\n");
    printf(ANSI_ERASE_SCREEN); 
    fflush(stdout);


    for(i = 0;i < 300;i++)
    {
        printf("xxxxxxxxxx.row%d\n", i);
    //    sleep(2);
    }

    printf(ANSI_CURSOR_HOME(1,4));
    printf("456");
    fflush(stdout);
    getchar();
    printf(ANSI_CURSOR_HOME(2,2));
    fflush(stdout);
    printf("23");
    fflush(stdout);
    printf(ANSI_CURSOR_HOME(4,2));
    fflush(stdout);
    
    for(i = 0;i < 10;i++)
    {
        printf(RED_CODE "RED" NO_CODE "\n");
        printf(GREEN_CODE "GREEN" NO_CODE "\n");
        printf(YELLOW_CODE "YELLOW" NO_CODE "\n");
        printf(RED_CODE "RED" NO_CODE "MIXED_WITH" YELLOW_CODE "YELLOW" NO_CODE "AND" GREEN_CODE "GREEN" NO_CODE "\n");
        printf("DEFAULT"  "\n");
        printf("%d Input:", i);
        do
        {
            c = getc(stdin);
            printf("Got: '%d' '%s'\n", (int)c, letterToStr(c));
            if(c == -1)
                return 1;
        } while(c != '\n');
        
        
    }
    return 0;
}

