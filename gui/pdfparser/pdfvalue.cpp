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


#include "pdfvalue.h"
#include "pdfvalue_p.h"
#include "pdfobject.h"

#include <QDebug>

using namespace PdfParser;


//###############################################
// PDF Value
//###############################################
Value::Value()
{
    d = new ValueData(Type::Undefined);
}


/************************************************
 *
 ************************************************/
Value::Value(const Value &other):
    d(other.d)
{

}


/************************************************
 *
 ************************************************/
Value::Value(ValueData *data):
    d(data)
{

}


/************************************************
 *
 ************************************************/
void Value::setValid(bool value)
{
    d->mValid = value;
}


/************************************************
 *
 ************************************************/
Value::~Value()
{

}


/************************************************
 *
 ************************************************/
Value &Value::operator =(const Value &other)
{
    d = other.d;
    return *this;
}


/************************************************
 *
 ************************************************/
bool Value::isValid()
{
    return d->mValid;
}


/************************************************
 *
 ************************************************/
Value::Type Value::type() const
{
    return d->mType;
}



#define CONVERT_TO_PDF_TYPE(TYPE) \
    if (is ## TYPE())\
    {\
        if (ok) *ok = true;\
        return TYPE(*(static_cast<const TYPE*>(this)));\
    }\
    \
    if (ok) *ok = false;\
    return TYPE();\


/************************************************
 *
 ************************************************/
Array         Value::toArray(bool *ok)         const { CONVERT_TO_PDF_TYPE(Array)         }
Bool          Value::toBool(bool *ok)          const { CONVERT_TO_PDF_TYPE(Bool)          }
Dict          Value::toDict(bool *ok)          const { CONVERT_TO_PDF_TYPE(Dict)          }
HexString     Value::toHexString(bool *ok)     const { CONVERT_TO_PDF_TYPE(HexString)     }
Link          Value::toLink(bool *ok)          const { CONVERT_TO_PDF_TYPE(Link)          }
LiteralString Value::toLiteralString(bool *ok) const { CONVERT_TO_PDF_TYPE(LiteralString) }
Name          Value::toName(bool *ok)          const { CONVERT_TO_PDF_TYPE(Name)          }
Null          Value::toNull(bool *ok)          const { CONVERT_TO_PDF_TYPE(Null)          }
Number        Value::toNumber(bool *ok)        const { CONVERT_TO_PDF_TYPE(Number)        }


/************************************************
 *
 ************************************************/
PdfParser::ValueData *ValueData::privateData(const Value &other) const
{
    return other.d.data();
}


//###############################################
// PDF Array
//###############################################
Array::Array():
    Value(new ArrayData())
{

}

/************************************************
 *
 ************************************************/
Array::Array(const Array &other):
    Value(other)
{
}


/************************************************
 *
 ************************************************/
Array &Array::operator =(const Array &other)
{
    d = other.d;
    return *this;
}


/************************************************
 *
 ************************************************/
const QVector<Value> Array::values() const
{
    return d->as<ArrayData>()->mValues;
}


/************************************************
 *
 ************************************************/
QVector<Value> Array::values()
{
    return d->as<ArrayData>()->mValues;

}


/************************************************
 *
 ************************************************/
void Array::append(const Value &value)
{
    d->as<ArrayData>()->mValues.append(value);
}


//###############################################
// PDF Bool
//###############################################
Bool::Bool():
    Value(new BoolData())
{

}


/************************************************
 *
 ************************************************/
Bool::Bool(const Bool &other):
    Value(other)
{

}

/************************************************
 *
 ************************************************/
Bool &Bool::operator =(const Bool &other)
{
    d = other.d;
    return *this;
}

/************************************************
 *
 ************************************************/
bool Bool::value() const
{
    return d->as<BoolData>()->mValue;
}


/************************************************
 *
 ************************************************/
void Bool::setValue(bool value)
{
    d->as<BoolData>()->mValue = value;
}


//###############################################
// PDF Dictionary
//###############################################
Dict::Dict():
    Value(new DictData())
{

}


/************************************************
 *
 ************************************************/
Dict::Dict(const Dict &other):
    Value(other)
{
}


/************************************************
 *
 ************************************************/
Dict &Dict::operator =(const Dict &other)
{
    d = other.d;
    return *this;
}


/************************************************
 *
 ************************************************/
QMap<QString, Value> Dict::values()
{
    return d->as<DictData>()->mValues;
}


/************************************************
 *
 ************************************************/
Value Dict::value(const QString &key) const
{
    return d->as<DictData>()->mValues.value(key);
}


/************************************************
 *
 ************************************************/
void Dict::insert(const QString &key, const Value &value)
{
    d->as<DictData>()->mValues.insert(key, value);
}


/************************************************
 *
 ************************************************/
void Dict::insert(const QString &key, double value)
{
    d->as<DictData>()->mValues.insert(key, Number(value));
}


/************************************************
 *
 ************************************************/
QStringList Dict::keys() const
{
    QStringList res = d->as<DictData>()->mValues.keys();
    res.sort();
    return res;
}


//###############################################
// PDF Hexadecimal String
//###############################################
HexString::HexString():
    Value(new HexStringData())
{

}


/************************************************
 *
 ************************************************/
HexString::HexString(const QString &value):
    Value(new HexStringData())
{
    d->as<HexStringData>()->mValue = value.toLocal8Bit().toHex();
}


/************************************************
 *
 ************************************************/
HexString::HexString(const HexString &other):
    Value(other)
{

}


/************************************************
 *
 ************************************************/
HexString &HexString::operator =(const HexString &other)
{
    d = other.d;
    return *this;
}


/************************************************
 *
 ************************************************/
QByteArray HexString::value() const
{
    return d->as<HexStringData>()->mValue;
}

/************************************************
 *
 ************************************************/
void HexString::setValue(const QByteArray &value)
{
    d->as<HexStringData>()->mValue = value;
}


//###############################################
// PDF Literal String
//###############################################
Link::Link(quint32 objNum, quint16 genNum):
    Value(new LinkData(objNum, genNum))
{
}


/************************************************
 *
 ************************************************/
Link::Link(const Link &other):
    Value(other)
{
}


/************************************************
 *
 ************************************************/
Link::Link(const Object &obj):
    Value(new LinkData(obj.objNum(), obj.genNum()))
{

}


/************************************************
 *
 ************************************************/
Link &Link::operator =(const Link &other)
{
    d = other.d;
    return *this;
}


/************************************************
 *
 ************************************************/
Link &Link::operator =(const Object &obj)
{
    setObjNum(obj.objNum());
    setGenNum(obj.genNum());
    return *this;
}


/************************************************
 *
 ************************************************/
int Link::objNum() const
{
    return d->as<LinkData>()->mObjNum;
}


/************************************************
 *
 ************************************************/
void Link::setObjNum(int value)
{
    d->as<LinkData>()->mObjNum = value;
}


/************************************************
 *
 ************************************************/
int Link::genNum() const
{
    return d->as<LinkData>()->mGenNum;
}

void Link::setGenNum(int value)
{
    d->as<LinkData>()->mGenNum = value;
}


//###############################################
// PDF Literal String
//###############################################
LiteralString::LiteralString():
    Value(new LiteralStringData())
{

}


/************************************************
 *
 ************************************************/
LiteralString::LiteralString(const LiteralString &other):
    Value(other)
{

}


/************************************************
 *
 ************************************************/
LiteralString &LiteralString::operator =(const LiteralString &other)
{
    d = other.d;
    return *this;
}


/************************************************
 *
 ************************************************/
QByteArray LiteralString::value() const
{
    return d->as<LiteralStringData>()->mValue;
}

/************************************************
 *
 ************************************************/
void LiteralString::setValue(const QByteArray &value)
{
    d->as<LiteralStringData>()->mValue = value;
}


//###############################################
// PDF Name
//###############################################
Name::Name():
    Value(new NameData())
{
}


/************************************************
 *
 ************************************************/
Name::Name(const Name &other):
    Value(other)
{
}


/************************************************
 *
 ************************************************/
Name &Name::operator =(const Name &other)
{
    d = other.d;
    return *this;
}


/************************************************
 *
 ************************************************/
QString Name::value() const
{
    return d->as<NameData>()->mValue;
}


/************************************************
 *
 ************************************************/
void Name::setName(const QString &value)
{
    d->as<NameData>()->mValue = value;
}


//###############################################
// PDF Null
//###############################################
Null::Null():
    Value(new NullData())
{

}


/************************************************
 *
 ************************************************/
Null::Null(const Null &other):
    Value(other)
{

}


/************************************************
 *
 ************************************************/
Null &Null::operator =(const Null &other)
{
    d = other.d;
    return *this;
}


//###############################################
// PDF Number
//###############################################
Number::Number(double value):
    Value(new NumberData(value))
{

}


/************************************************
 *
 ************************************************/
Number::Number(const Number &other):
    Value(other)
{
}


/************************************************
 *
 ************************************************/
Number &Number::operator =(const Number &other)
{
    d = other.d;
    return *this;
}


/************************************************
 *
 ************************************************/
double Number::value() const
{
    return d->as<NumberData>()->mValue;
}


/************************************************
 *
 ************************************************/
void Number::setValue(double value)
{
    d->as<NumberData>()->mValue = value;
}
