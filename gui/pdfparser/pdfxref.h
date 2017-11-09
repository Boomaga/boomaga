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

#include "pdfvalue.h"

#include <QMap>
namespace PDF {


class XRefEntry
{
friend class XRefTable;
public:
    enum Type{
        Free,
        Used,
        Compressed
    };

    XRefEntry():
        mPos(0),
        mObjNum(0),
        mGenNum(0),
        mType(Type::Free)
    {
    }

    Type type() const { return mType; }
    quint64 pos() const;
    PDF::ObjNum objNum() const;
    PDF::GenNum genNum() const;
    PDF::ObjNum streamObjNum() const;
    quint32     streamIndex() const;

private:
    qint64      mPos;
    PDF::ObjNum mObjNum;
    PDF::GenNum mGenNum;
    Type        mType;
};


class XRefTable: public QMap<PDF::ObjNum, XRefEntry>
{
public:
    qint32 maxObjNum() const;

    XRefEntry addFreeObject(PDF::ObjNum objNum, PDF::GenNum genNum, PDF::ObjNum nextFreeObj = 0);
    XRefEntry addUsedObject(PDF::ObjNum objNum, PDF::GenNum genNum, quint64 pos);
    XRefEntry addCompressedObject(PDF::ObjNum objNum, PDF::ObjNum streamObjNum, quint32 streamIndex);

    // Restore free entries chain.
    void updateFreeChain();
};

} // namespace PDF

QDebug operator<<(QDebug debug, const PDF::XRefEntry &xref);
QDebug operator<<(QDebug debug, const PDF::XRefTable &xrefTable);
#endif // PDFXREF_H
