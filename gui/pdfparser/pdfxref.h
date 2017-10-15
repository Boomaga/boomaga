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


#ifndef PDFXREF_H
#define PDFXREF_H

#include <QMap>
namespace PdfParser {


struct XRefEntry {
    enum Type{
        Free,
        Used
    };

    XRefEntry():
        pos(0),
        objNum(0),
        genNum(0),
        type(Type::Free)
    {
    }

    XRefEntry(qint64  pos, quint32 objNum, quint16 genNum, Type type):
        pos(pos),
        objNum(objNum),
        genNum(genNum),
        type(type)
    {
    }

    qint64  pos;
    quint32 objNum;
    quint16 genNum;
    Type type;
};

class XRefTable: public QMap<quint32, XRefEntry>
{
};


} // namespace PdfParser
#endif // PDFXREF_H
