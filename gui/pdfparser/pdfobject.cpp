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
