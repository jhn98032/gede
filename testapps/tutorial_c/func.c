#include "func.h"

const char *my_function(int a)
{
    int b;
    const char *str;
    b = a;
    
    if(b > 2)
        str = "Greater than 2";
    else
        str = "Less or equal to 2";

    return str;
}

