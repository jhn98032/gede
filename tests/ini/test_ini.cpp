#include <assert.h>
#include "../../src/ini.h"

#include <stdio.h>

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
    assert(ini1.getInt("int") == 11);
    ini1.setString("group2/string", "group2/string_data");
    ini1.setString("group1/string", "group1/string_data\u00c4");

    ini1.setDouble("floatv", 123.456);
    ini1.save("test.ini");
}

void readIni()
{
    QByteArray byteArray;
    Ini ini2;

    ini2.appendLoad("test.ini");
    assert(QString(ini2.getString("string")) ==  QString("string_value"));
    assert(ini2.getInt("int") == 11);
    ini2.getByteArray("byteArray", &byteArray);
    assert(byteArray.size() == 2);
    assert(byteArray[0] == (char)0xDE);
    assert(byteArray[1] == (char)0xAD);

    assert(ini2.getDouble("floatv", 999.888) == 123.456);

    QString s = ini2.getString("group1/string");
    assert(s == "group1/string_data\u00c4");

    QSize size;
    size = ini2.getSize("size_test/size", QSize(0,0));
    assert(size.width() == 123);
    assert(size.height() == 456);
    
}

int main(int argc,char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    writeInit();
    

    readIni();
    
    
    return 0;
}
