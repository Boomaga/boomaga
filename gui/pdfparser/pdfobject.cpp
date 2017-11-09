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


#include "pdfobject.h"
#include "pdftypes.h"
#include "pdfvalue.h"
#include <zlib.h>

#include <QDebug>

using namespace PDF;


/************************************************
 *
 ************************************************/
Object::Object(ObjNum objNum, GenNum genNum, const Value &value):
    mObjNum(objNum),
    mGenNum(genNum),
    mValue(value)
{
}


/************************************************
 *
 ************************************************/
Object::Object(const Object &other):
    mObjNum( other.mObjNum),
    mGenNum( other.mGenNum),
    mValue(  other.mValue),
    mStream( other.mStream)
{
}


/************************************************
 *
 ************************************************/
Object &Object::operator =(const Object &other)
{
    mObjNum  = other.mObjNum;
    mGenNum  = other.mGenNum;
    mValue   = other.mValue;
    mStream  = other.mStream;
    return *this;
}


/************************************************
 *
 ************************************************/
Object::~Object()
{

}


/************************************************
 *
 ************************************************/
void Object::setObjNum(ObjNum value)
{
    mObjNum = value;
}


/************************************************
 *
 ************************************************/
void Object::setGenNum(GenNum value)
{
    mGenNum = value;
}


/************************************************
 *
 ************************************************/
void Object::setValue(const Value &value)
{
    mValue = value;
}


/************************************************
 *
 ************************************************/
void Object::setStream(const QByteArray &value)
{
    mStream = value;
}


/************************************************
 *
 ************************************************/
QByteArray Object::decodedStream() const
{
    QStringList filters;
    const PDF::Value &v = dict().value("Filter");
    if (v.isName())
    {
        filters << v.asName().value();
    }
    else if (v.isArray())
    {
        const PDF::Array &arr = v.asArray();
        for (int i=0; i<arr.count(); ++i)
            filters << arr.at(i).asName().value();
    }

    QByteArray res = stream();
    foreach (const QString &filter, filters)
    {
        if (filter == "FlateDecode")
        {
            res = streamFlateDecode(res);
            continue;
        }

        if (filter == "ASCIIHexDecode"  ||
            filter == "ASCII85Decode"   ||
            filter == "LZWDecode"       ||
            filter == "FlateDecode"     ||
            filter == "RunLengthDecode" ||
            filter == "CCITTFaxDecode"  ||
            filter == "JBIG2Decode"     ||
            filter == "DCTDecode"       ||
            filter == "JPXDecode"       ||
            filter == "Crypt"           )
        {
            qWarning() << "Error: " << mObjNum << mGenNum << " obj: unsupported filter " << filter.toLocal8Bit();
            return QByteArray();
        }

        qWarning() << "Error: " << mObjNum << mGenNum << " obj: incorrect filter" << filter.toLocal8Bit();
        return QByteArray();
    }
    return res;
}


/************************************************
 *
 ************************************************/
QString Object::type() const
{
    return dict().value("Type").asName().value();
}


/************************************************
 *
 ************************************************/
QString Object::subType() const
{
    QString s = dict().value("Subtype").asName().value();
    if (s.isEmpty())
        return dict().value("S").asName().value();
    else
        return s;
}


/************************************************
 *
 ************************************************/
QByteArray Object::streamFlateDecode(const QByteArray &source) const
{
    QByteArray res;
    // More typical zlib compression ratios are on the order of 2:1 to 5:1.
    res.resize(source.size() * 2);

    while (true)
    {
        uchar * dest = (uchar*)res.data();
        uLongf destSize = res.size();

        int ret = uncompress(dest, &destSize, reinterpret_cast<const uchar*>(source.data()), source.length());
        switch (ret)
        {
        case Z_OK:
            res.resize(destSize);
            break;

        case Z_MEM_ERROR:
            qWarning("Z_MEM_ERROR: Not enough memory");
            return QByteArray();

        case Z_DATA_ERROR:
            qWarning("Z_DATA_ERROR: Input data is corrupted");
            return QByteArray();

        case Z_BUF_ERROR:
            res.resize(res.length() * 2);
            continue;
        }

        break;
    }


    return res;
}


/************************************************
 *
 ************************************************/
QDebug operator<<(QDebug dbg, const Object &obj)
{
    dbg.nospace() << obj.objNum() << " " << obj.genNum() << " obj\n";
    dbg.nospace() << obj.value();
    if (obj.stream().length())
        dbg.nospace() << "\n" << "stream length " << obj.stream().length();
    else
        dbg.nospace() << "\n" << "no stream";
    dbg.nospace() << "\nendobj\n";
    return dbg;
}
