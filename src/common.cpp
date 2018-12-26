/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2018 Boomaga team https://github.com/Boomaga
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "common.h"
#include <cstdarg>
#include <sstream>
#include <iostream>

using namespace std;

static string LogPrefix = "Boomaga";

/************************************************
 *
 ************************************************/
string Log::prefix()
{
    return LogPrefix;
}


/************************************************
 *
 ************************************************/
void Log::setPrefix(const string &prefix)
{
    LogPrefix = prefix;
}


/************************************************
 *
 ************************************************/
static void print(const char *level, const char *fmt, va_list args)
{
    char buffer[1024];
    vsnprintf(buffer,1024,fmt, args);

    string line;
    stringstream str(buffer);
    while(std::getline(str, line))
    {
        cerr << level << ": [" << LogPrefix << " ] " << level << ": " << line << endl;
    }
}


/************************************************
 *
 ************************************************/
void Log::debug(const char *str, ...)
{
    va_list args;
    va_start(args, str);
    print("DEBUG", str, args);
    va_end(args);
}


/************************************************
 *
 ************************************************/
void Log::info(const char *str, ...)
{
    va_list args;
    va_start(args, str);
    print("INFO", str, args);
    va_end(args);
}


/************************************************
 *
 ************************************************/
void Log::warn(const char *str, ...)
{
    va_list args;
    va_start(args, str);
    print("WARNING", str, args);
    va_end(args);
}


/************************************************
 *
 ************************************************/
void Log::error(const char *str, ...)
{
    va_list args;
    va_start(args, str);
    print("ERROR", str, args);
    va_end(args);
}


/************************************************
 *
 ************************************************/
void Log::fatalError(const char *str, ...)
{
    va_list args;
    va_start(args, str);
    print("ERROR", str, args);
    va_end(args);
    exit(CUPS_BACKEND_FAILED);
}


