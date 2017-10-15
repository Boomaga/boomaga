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
#include "pdfdata.h"
#include "pdfvalue.h"
#include "pdfvalue_p.h"

#include <QDebug>
#include <QSharedData>

using namespace PdfParser;

class PdfParser::ObjectData: public QSharedData
{
public:
    ObjectData():
        QSharedData(),
        mObjNum(0),
        mGenNum(0)
    {
    }

    ObjectData(const ObjectData &other):
        QSharedData(other),
        mObjNum(other.mObjNum),
        mGenNum(other.mGenNum),
        mType(other.mType),
        mSubType(other.mSubType),
        mValue(other.mValue)
    {
    }

    virtual ~ObjectData()
    {
    }

    ObjectData &operator =(const ObjectData &other)
    {
        mObjNum  = other.mObjNum;
        mGenNum  = other.mGenNum;
        mType    = other.mType;
        mSubType = other.mSubType;
        mValue   = other.mValue;
        return *this;
    }

    quint32 mObjNum;
    quint16 mGenNum;
    QString mType;
    QString mSubType;
    QString mError;
    Value mValue;
    QByteArray mStream;
};


/************************************************
 *
 ************************************************/
Object::Object()
{
    d = new ObjectData;
}


/************************************************
 *
 ************************************************/
Object::Object(const Object &other):
    d(other.d)
{
}


/************************************************
 *
 ************************************************/
Object &Object::operator =(const Object &other)
{
    d = other.d;
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
quint32 Object::objNum() const
{
    return d->mObjNum;
}


/************************************************
 *
 ************************************************/
void Object::setObjNum(quint32 value)
{
    d->mObjNum = value;
}


/************************************************
 *
 ************************************************/
quint16 Object::genNum() const
{
    return d->mGenNum;
}


/************************************************
 *
 ************************************************/
void Object::setGenNum(quint16 value)
{
    d->mGenNum = value;
}


/************************************************
 *
 ************************************************/
Dict Object::dictionary() const
{
    return d->mValue.toDict();
}


/************************************************
 *
 ************************************************/
Value Object::value() const
{
    return d->mValue;
}


/************************************************
 *
 ************************************************/
void Object::setValue(const Value &value)
{
    d->mValue = value;
}


/************************************************
 *
 ************************************************/
QByteArray Object::stream() const
{
    return d->mStream;
}


/************************************************
 *
 ************************************************/
void Object::setStream(const QByteArray &value)
{
    d->mStream = value;
}
