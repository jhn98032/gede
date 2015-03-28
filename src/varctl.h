#ifndef FILE__VAR_CTL_H
#define FILE__VAR_CTL_H


#include <QString>
#include <QMap>
#include <QObject>


class VarCtl : public QObject
{
    Q_OBJECT

public:
    VarCtl(){};
    

    enum DispFormat
    {
        DISP_NATIVE,
        DISP_DEC,
        DISP_BIN,
        DISP_HEX,
        DISP_CHAR,
    };
    typedef struct
    {
        QString orgValue;
        DispFormat orgFormat;
        DispFormat dispFormat;
        bool isExpanded;
    }DispInfo;

    typedef QMap<QString, DispInfo>  DispInfoMap;


    static DispFormat findVarType(QString dataString);
    static QString valueDisplay(long long value, DispFormat format);

};

#endif // FILE__VAR_CTL_H
