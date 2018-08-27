/*
 * Copyright (C) 2014-2016 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
 
#include "qtutil.h"



bool isInteger(QString str)
{
    if(str.size() == 0)
        return false;
    if(str[0] == '-')
        return true;
    if(str[0].isDigit())
        return true;
    return false;
}

