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


#include "pdfxref.h"
#include <ctgmath>
#include <QDebug>

using namespace PDF;


/************************************************
 *
 ************************************************/
qint32 XRefTable::maxObjNum() const
{
    if (isEmpty())
        return 0;
    return (--constEnd()).key();
}


/************************************************
 *
 ************************************************/
XRefEntry XRefTable::addFreeObject(ObjNum objNum, GenNum genNum, ObjNum nextFreeObj)
{
    XRefEntry entry;
    entry.mObjNum = objNum;
    entry.mGenNum = genNum;
    entry.mType   = XRefEntry::Free;
    entry.mPos    = nextFreeObj;

    insert(objNum, entry);
    return entry;
}


/************************************************
 *
 ************************************************/
XRefEntry XRefTable::addUsedObject(ObjNum objNum, GenNum genNum, quint64 pos)
{
    XRefEntry entry;
    entry.mObjNum = objNum;
    entry.mGenNum = genNum;
    entry.mType   = XRefEntry::Used;
    entry.mPos    = pos;

    insert(objNum, entry);
    return entry;
}

/************************************************
 *
 ************************************************/
XRefEntry XRefTable::addCompressedObject(ObjNum objNum, ObjNum streamObjNum, quint32 streamIndex)
{
    XRefEntry entry;
    entry.mObjNum = objNum;
    entry.mGenNum = streamIndex;
    entry.mType   = XRefEntry::Compressed;
    entry.mPos    = streamObjNum;

    insert(objNum, entry);
    return entry;
}


/************************************************
 *
 ************************************************/
void XRefTable::updateFreeChain()
{
    auto prev = begin();
    for (auto i = ++(begin()) ; i != end(); ++i)
    {
        if (i.value().mType == XRefEntry::Free)
        {
            i.value().mPos = prev.value().mObjNum;
            prev = i;
        }
    }
}


/************************************************
 *
 ************************************************/
quint64 XRefEntry::pos() const
{
    return mType == Compressed ? 0 : mPos;
}


/************************************************
 *
 ************************************************/
ObjNum XRefEntry::objNum() const
{
    return mObjNum;
}


/************************************************
 *
 ************************************************/
GenNum XRefEntry::genNum() const
{
    return mType == Compressed ? 0 : mGenNum;
}


/************************************************
 *
 ************************************************/
ObjNum XRefEntry::streamObjNum() const
{
    return mType == Compressed ? mPos : 0;
}


/************************************************
 *
 ************************************************/
quint32 XRefEntry::streamIndex() const
{
    return mType == Compressed ? mGenNum : 0;
}


/************************************************
 *
 ************************************************/
QDebug operator<<(QDebug debug, const XRefEntry &xref)
{
    switch (xref.type())
    {
    case XRefEntry::Type::Free:
        debug.space()
                << xref.objNum()
                << xref.genNum()
                << "FREE";
        break;

    case XRefEntry::Type::Used:
        debug.space()
                << xref.objNum()
                << xref.genNum()
                << "Pos:" << xref.pos();
        break;

    case XRefEntry::Type::Compressed:
        debug.space()
                << xref.objNum()
                << xref.genNum()
                << "Stream:" << xref.streamObjNum()
                << ":" << xref.streamIndex();
        break;
    }

    return debug;
}


/************************************************
 *
 ************************************************/
QDebug operator<<(QDebug debug, const XRefTable &xrefTable)
{
    foreach (const XRefEntry &entry, xrefTable)
    {
        qDebug() << entry;
    }
    return debug;
}
