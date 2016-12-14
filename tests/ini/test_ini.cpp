#include <assert.h>
#include "../../src/ini.h"

#include <stdio.h>

void writeInit()
{
    Ini ini1;
    ini1.setString("string", "string_value");
    ini1.setInt("int", 11);
    QByteArray byteArray;
    byteArray += 0xde;
    byteArray += 0xad;
    ini1.setByteArray("byteArray",byteArray);
    assert(ini1.getInt("int") == 11);

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
    
}

int main(int argc,char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    writeInit();
    

    readIni();
    
    
    return 0;
}
