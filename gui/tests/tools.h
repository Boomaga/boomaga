/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2017 Boomaga team https://github.com/Boomaga
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

#ifndef TOOLS_H
#define TOOLS_H

#ifdef __GNUG__
#include <memory>
#include <cxxabi.h>

QString exceptionName(const std::exception &e)
{
    int     status;
    char   *realname = abi::__cxa_demangle(typeid(e).name(), 0, 0, &status);
    if (status == 0)
    {
        QString res = QString::fromLatin1(realname);
        free(realname);
        return res;
    }
    return typeid(e).name() ;
}

#else

QString exceptionName(const std::exception &e) {
    return typeid(e).name();
}

#endif

#define FAIL_EXCEPTION(E) QFAIL(QString("Exception %1 at pos: %2: %3").arg(exceptionName(E)).arg(E.pos()).arg(E.description()).toLocal8Bit())

#endif //TOOLS_H
