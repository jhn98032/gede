/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "varctl.h"


VarCtl::DispFormat VarCtl::findVarType(QString dataString)
{
    dataString = dataString.trimmed();
    if(dataString.isEmpty())
        return DISP_NATIVE;
    if(dataString.startsWith("0x") && dataString.indexOf(' ') == -1)
        return DISP_DEC;
    if(!dataString.isEmpty())
    {
        int u = 0;
        if(dataString[0] == '-')
            u++;
        for(;u < dataString.length();u++)
        {
            if(!dataString[u].isDigit())
            {
                return DISP_NATIVE;
            }
        }
        return DISP_DEC;
        
    }
    return DISP_NATIVE;
 }




/**
 * @brief Formats a string (Eg: 0x2) that represents a decimal value.
 */
QString VarCtl::valueDisplay(long long val, DispFormat format)
{
    QString valueText;
    if(format == DISP_BIN)
    {
        QString subText;
        QString reverseText;
        do
        {
            subText.sprintf("%d", (int)(val & 0x1));
            reverseText = subText + reverseText;
            val = val>>1;
        }
        while(val > 0 || reverseText.length()%8 != 0);
        for(int i = 0;i < reverseText.length();i++)
        {
            valueText += reverseText[i];
            if(i%4 == 3 && i+1 != reverseText.length())
                valueText += "_";
        }
        
        valueText = "0b" + valueText;
        
    }
    else if(format == DISP_HEX)
    {
        QString text;
        text.sprintf("%llx", val);

        // Prefix the string with suitable number of zeroes
        while(text.length()%4 != 0 && text.length() > 4)
            text = "0" + text;
        if(text.length()%2 != 0)
            text = "0" + text;
            
        for(int i = 0;i < text.length();i++)
        {
            valueText = valueText + text[i];
            if(i%4 == 3 && i+1 != text.length())
                valueText += "_";
        }
        valueText = "0x" + valueText;        
    }
    else if(format == DISP_CHAR)
    {
        QChar c = QChar((int)val);
        if(c.isPrint())
            valueText.sprintf("'%c'", c.toAscii());
        else
            valueText.sprintf("' '");
        
    }
    else if(format == DISP_DEC)
    {
        valueText.sprintf("%lld", val);
    }
    return valueText;
}

