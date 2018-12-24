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


#ifndef COMMON_H
#define COMMON_H
#include <string>

static const int CUPS_BACKEND_OK      = 0;
static const int CUPS_BACKEND_FAILED  = 1;

class Log
{
public:
    static void debug(const char *str, ...);
    static void info(const char *str, ...);
    static void warn(const char *str, ...);
    static void error(const char *str, ...);
    static void fatalError(const char *str, ...);

    static std::string prefix();
    static void setPrefix(const std::string &prefix);
};


#endif // COMMON_H
