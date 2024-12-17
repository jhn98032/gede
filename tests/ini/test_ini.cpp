#include <assert.h>
#include "../../src/ini.h"

#include <unistd.h>
#include <stdio.h>

#define TEST_INI_FILENAME   "test.ini"

void test_verify_(int lineNo, int t, const char *testStr)
{
    if(!t)
    {
        fprintf(stderr, "Test failed L%d: '%s'\n", lineNo, testStr);
        exit(1);
    }
}
#define test_verify(t)  test_verify_(__LINE__, t, #t)

void writeInit()
{
    Ini ini1;

    QSize size(123,456);
    ini1.setSize("size_test/size", size);

    ini1.setString("string", "string_value");
    ini1.setInt("int", 11);
    QByteArray byteArray;
    byteArray += 0xde;
    byteArray += 0xad;
    ini1.setByteArray("byteArray", byteArray);
    QByteArray emptyByteArray;
    ini1.setByteArray("emptyByteArray", emptyByteArray);
    test_verify(ini1.getInt("int") == 11);
    ini1.setString("group2/string", "group2/string_data");
    ini1.setString("group1/string", "group1/string_data\u00c4");

    ini1.setDouble("floatv", 123.456);
    ini1.save(TEST_INI_FILENAME);
}

void readIni()
{
    QByteArray byteArray;
    Ini ini2;

    ini2.appendLoad(TEST_INI_FILENAME);
    test_verify(QString(ini2.getString("string")) ==  QString("string_value"));
    test_verify(ini2.getInt("int") == 11);
    ini2.getByteArray("byteArray", &byteArray);
    test_verify(byteArray.size() == 2);
    test_verify(byteArray[0] == (char)0xDE);
    test_verify(byteArray[1] == (char)0xAD);

    test_verify(ini2.getDouble("floatv", 999.888) == 123.456);

    QString s = ini2.getString("group1/string");
    test_verify(s == "group1/string_data\u00c4");

    QSize size;
    size = ini2.getSize("size_test/size", QSize(0,0));
    test_verify(size.width() == 123);
    test_verify(size.height() == 456);
    
}

int main(int argc,char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    printf("Running INI tests\n");

    writeInit();
    

    readIni();

    unlink(TEST_INI_FILENAME);
    printf("All INI tests done\n");
    
    return 0;
}
