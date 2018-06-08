#include <stdio.h>
	 	//rad1
   	//kalle
	//tab1
    //kalle
  	//nisse


void long_func_with_many_arguments(int a_very_long_function_variable_name_xxxxxxx, int another_very_long_variable_name___, int another_another_very_long_variable_name___, int another_another_very_long_variable_name___2)
{
}

void func()
{
    volatile int a = 1;
    a++;
    a++;
    a++;
}

typedef enum {CUSTOM_ENUM1, CUSTOM_ENUM2} CustomEnum;
int main(int argc,char *argv[])
{
    const char **strList;
    const char *strListData[] = {"hej", "kalle"};
    int i3[] = { 111,222};
    struct
    {
        int a;
        struct
        {
            int b;
        }s2;
    }s;
    char c = 'Z';
    float f1;
    char *varStr;
    enum {ENUM1, ENUM2}varEnum;
    unsigned char d = 0;
    CustomEnum customEnum1;

    varStr = "stri\n\r\t\03ng1";
    varStr = "string2";
    f1 = 0.0;
    s.a = 0;
    s.s2.b = 1;
    varEnum = ENUM1;
    varEnum = ENUM2;
    c = '\'';
    
    strList = strListData;

    switch(s.a)
    {
        case 0:
            {
                printf("Hej!\n");
            };break;
        default:;break;
    }
    
    while(1)
    {
        c++;
        f1 += 0.1;
        func();
        s.a++;
        s.s2.b++;
        s.s2.b++;
        s.a++;
        d++;
    }
    return 0;
}

