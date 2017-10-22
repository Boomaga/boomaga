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


#ifndef PDFTYPES_H
#define PDFTYPES_H

#include <QtGlobal>
#include <QString>

namespace PDF {

enum class ObjecType
{
    Dictionary,
};


#define PDF_ERROR(NAME)  class NAME: public Error { public:  NAME(qint64 pos, const QString description = ""): Error(pos, description)  {}}

class Error: public std::exception
{
public:
    Error(qint64 pos, const QString description = ""): std::exception(), mPos(pos), mDescription(description) {}
    qint64 pos() const { return mPos; }
    QString description() const { return mDescription; }
private:
    qint64 mPos;
    QString mDescription;
};

PDF_ERROR(HeaderError);
PDF_ERROR(ParseError);
PDF_ERROR(UnknownValueError);

} // namespace PDF

#endif // PDFTYPES_H
