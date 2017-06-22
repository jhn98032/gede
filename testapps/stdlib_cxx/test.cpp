#include <stdio.h>

#include <vector>
#include <list>

class MyClass
{
    public:
        MyClass() {};
    
        int a;
        char *str;
};

using namespace std;
int main(int argc,char *argv[])
{
    MyClass myclass1;
    vector<int> g_vectorInt;
    list<int> g_listInt;
    struct {
        const char *a_str;
        int a_int;
    }c_struct;
    char *d_str;
    int e_int;
    const char *f_str = "hej";
    const char g_str[] = "hej";
    const int h_vec[] = {1,2,3};
    const int *i_vec = NULL;
    struct {
        vector<int> list1;
        int j_struct_int;
    }j_struct;
   double k_double = 1.2;

    i_vec = (const int*)0x10;
    g_vectorInt.push_back(1);
    g_vectorInt.push_back(2);
    
    g_listInt.push_back(1);
    g_listInt.push_back(2);

    while(1)
    {
        e_int++;
    }
}


