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


#include <assert.h>
#include "pdfvalue.h"
#include "pdfvalue_p.h"
#include "pdfobject.h"

#include <QDebug>

using namespace PDF;


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
Value::Value(Value::Type type)
{
    d = new ValueData(type);
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
bool Value::isValid() const
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


/************************************************
 *
 ************************************************/
template <typename T>
const T &valueAs(const Value *value, Value::Type type, bool *ok)
{
    if (value->type() == type)
    {
        if (ok) *ok = true;
        return *(static_cast<const T*>(value));
    }
    else
    {
        if (ok) *ok = false;
        return ValueData::emptyValue<T>();
    }
}


/************************************************
 *
 ************************************************/
template <typename T>
T &valueAs(Value *value, Value::Type type, bool *ok)
{
    if (value->type() == type)
    {
        if (ok) *ok = true;
        return *(static_cast<T*>(value));
    }
    else
    {
        if (ok) *ok = false;
        return ValueData::emptyValue<T>();
    }
}


/************************************************
 *
 ************************************************/
const Array &Value::toArray(bool *ok) const
{
    return valueAs<Array>(this, Type::Array, ok);
}


/************************************************
 *
 ************************************************/
Array &Value::toArray(bool *ok)
{
    return valueAs<Array>(this, Type::Array, ok);
}


/************************************************
 *
 ************************************************/
const Bool &Value::toBool(bool *ok) const
{
    return valueAs<Bool>(this, Type::Bool, ok);
}


/************************************************
 *
 ************************************************/
Bool &Value::toBool(bool *ok)
{
    return valueAs<Bool>(this, Type::Bool, ok);
}


/************************************************
 *
 ************************************************/
const Dict &Value::toDict(bool *ok) const
{
    return valueAs<Dict>(this, Type::Dict, ok);
}


/************************************************
 *
 ************************************************/
Dict &Value::toDict(bool *ok)
{
    return valueAs<Dict>(this, Type::Dict, ok);
}


/************************************************
 *
 ************************************************/
const HexString &Value::toHexString(bool *ok) const
{
    return valueAs<HexString>(this, Type::HexString, ok);
}


/************************************************
 *
 ************************************************/
HexString &Value::toHexString(bool *ok)
{
    return valueAs<HexString>(this, Type::HexString, ok);
}


/************************************************
 *
 ************************************************/
const Link &Value::toLink(bool *ok) const
{
    return valueAs<Link>(this, Type::Link, ok);
}


/************************************************
 *
 ************************************************/
Link &Value::toLink(bool *ok)
{
    return valueAs<Link>(this, Type::Link, ok);
}


/************************************************
 *
 ************************************************/
const LiteralString &Value::toLiteralString(bool *ok) const
{
    return valueAs<LiteralString>(this, Type::LiteralString, ok);
}


/************************************************
 *
 ************************************************/
LiteralString &Value::toLiteralString(bool *ok)
{
    return valueAs<LiteralString>(this, Type::LiteralString, ok);
}


/************************************************
 *
 ************************************************/
const Name &Value::toName(bool *ok) const
{
    return valueAs<Name>(this, Type::Name, ok);
}


/************************************************
 *
 ************************************************/
Name &Value::toName(bool *ok)
{
    return valueAs<Name>(this, Type::Name, ok);
}


/************************************************
 *
 ************************************************/
const Null &Value::toNull(bool *ok) const
{
    return valueAs<Null>(this, Type::Null, ok);
}


/************************************************
 *
 ************************************************/
Null &Value::toNull(bool *ok)
{
    return valueAs<Null>(this, Type::Null, ok);
}


/************************************************
 *
 ************************************************/
const Number &Value::toNumber(bool *ok) const
{
    return valueAs<Number>(this, Type::Number, ok);
}


/************************************************
 *
 ************************************************/
Number &Value::toNumber(bool *ok)
{
    return valueAs<Number>(this, Type::Number, ok);
}


/************************************************
 *
 ************************************************/
bool Value::operator==(const Value &other) const
{
    if (d->mType != other.d->mType)
        return false;

    switch (d->mType)
    {
    case Type::Undefined:       return true;
    case Type::Array:           return d->mArrayValues == other.d->mArrayValues;
    case Type::Bool:            return d->mBoolValue   == other.d->mBoolValue;
    case Type::Dict:            return d->mDictValues  == other.d->mDictValues;
    case Type::HexString:       return d->mStringValue == other.d->mStringValue;
    case Type::Link:            return d->mLinkObjNum  == other.d->mLinkObjNum && d->mLinkGenNum == other.d->mLinkGenNum;
    case Type::LiteralString:   return d->mStringValue == other.d->mStringValue;
    case Type::Name:            return d->mStringValue == other.d->mStringValue;
    case Type::Null:            return true;
    case Type::Number:          return d->mNumberValue == other.d->mNumberValue;
    }
    return false;
}


/************************************************
 *
 ************************************************/
bool Value::operator!=(const Value &other) const
{
    return ! operator ==(other);
}


//###############################################
// PDF Array
//###############################################
Array::Array():
    Value(Type::Array)
{
    d->mValid = true;
}


/************************************************
 * Typically, the array takes the form
 *   [ llx lly urx ur y ]
 *  specifying the
 *   - lower-left x,
 *   - lower-left y,
 *   - upper-right x,
 *   - and upper-right y
 * coordinates of the rectangle, in that order.
 ************************************************/
Array::Array(const QRectF &rect):
    Value(Type::Array)
{
    d->mValid = true;
    d->mArrayValues.resize(4);
    d->mArrayValues[0] = PDF::Number(rect.left());
    d->mArrayValues[1] = PDF::Number(rect.bottom());
    d->mArrayValues[2] = PDF::Number(rect.right());
    d->mArrayValues[3] = PDF::Number(rect.top());
}


/************************************************
 *
 ************************************************/
Array::Array(const Array &other):
    Value(other)
{
    d->mValid = true;
}


/************************************************
 *
 ************************************************/
Array &Array::operator =(const Array &other)
{
    d = other.d;
    d->mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
const QVector<Value> Array::values() const
{
    assert(d->mType == Type::Array);
    return d->mArrayValues;
}


/************************************************
 *
 ************************************************/
QVector<Value> &Array::values()
{
    assert(d->mType == Type::Array);
    return d->mArrayValues;
}


/************************************************
 *
 ************************************************/
void Array::append(const Value &value)
{
    assert(d->mType == Type::Array);
    d->mArrayValues.append(value);
}


/************************************************
 *
 ************************************************/
int Array::count(const Value &value) const
{
    return d->mArrayValues.count(value);
}


/************************************************
 *
 ************************************************/
void Array::remove(int i)
{
    assert(d->mType == Type::Array);
    d->mArrayValues.remove(i);
}


/************************************************
 *
 ************************************************/
Array &Array::operator<<(const Value &value)
{
    assert(d->mType == Type::Array);
    d->mArrayValues.operator <<(value);
    return *this;
}


//###############################################
// PDF Bool
//###############################################
Bool::Bool(bool value):
    Value(Type::Bool)
{
    d->mBoolValue = value;
    d->mValid = true;
}


/************************************************
 *
 ************************************************/
Bool::Bool(const Bool &other):
    Value(other)
{
    d->mValid = true;
}


/************************************************
 *
 ************************************************/
Bool &Bool::operator =(const Bool &other)
{
    d = other.d;
    d->mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
bool Bool::value() const
{
    assert(d->mType == Type::Bool);
    return d->mBoolValue;
}


/************************************************
 *
 ************************************************/
void Bool::setValue(bool value)
{
    assert(d->mType == Type::Bool);
    if (isValid())
        d->mBoolValue = value;
}


//###############################################
// PDF Dictionary
//###############################################
Dict::Dict():
    Value(Type::Dict)
{
    d->mValid = true;
}


/************************************************
 *
 ************************************************/
Dict::Dict(const Dict &other):
    Value(other)
{
     d->mValid = true;
}


/************************************************
 *
 ************************************************/
Dict &Dict::operator =(const Dict &other)
{
    d = other.d;
    d->mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
QMap<QString, Value> Dict::values()
{
    assert(d->mType == Type::Dict);
    return d->mDictValues;
}


/************************************************
 *
 ************************************************/
const QMap<QString, Value> &Dict::values() const
{
    assert(d->mType == Type::Dict);
    return d->mDictValues;
}


/************************************************
 *
 ************************************************/
int Dict::size() const
{
    assert(d->mType == Type::Dict);
    return d->mDictValues.size();
}


/************************************************
 *
 ************************************************/
bool Dict::isEmpty() const
{
    assert(d->mType == Type::Dict);
    return d->mDictValues.isEmpty();
}


/************************************************
 *
 ************************************************/
bool Dict::contains(const QString &key) const
{
    assert(d->mType == Type::Dict);
    return d->mDictValues.contains(key);
}


/************************************************
 *
 ************************************************/
const Value Dict::value(const QString &key, const Value &defaultValue) const
{
    assert(d->mType == Type::Dict);
    return d->mDictValues.value(key, defaultValue);
}


/************************************************
 *
 ************************************************/
Value &Dict::operator[](const QString &key)
{
    assert(d->mType == Type::Dict);
    return d->mDictValues[key];
}


/************************************************
 *
 ************************************************/
const Value Dict::operator[](const QString &key) const
{
    assert(d->mType == Type::Dict);
    return d->mDictValues[key];
}


/************************************************
 *
 ************************************************/
void Dict::insert(const QString &key, const Value &value)
{
    if (isValid())
    {
        assert(d->mType == Type::Dict);
        d->mDictValues.insert(key, value);
    }
}


/************************************************
 *
 ************************************************/
void Dict::insert(const QString &key, double value)
{
    insert(key, Number(value));
}


/************************************************
 *
 ************************************************/
QStringList Dict::keys() const
{
    assert(d->mType == Type::Dict);
    QStringList res = d->mDictValues.keys();
    res.sort();
    return res;
}


//###############################################
// PDF Hexadecimal String
//###############################################
HexString::HexString(const QString &value):
    Value(Type::HexString)
{
    d->mValid = true;
    d->mStringValue = value.toLocal8Bit().toHex();
}


/************************************************
 *
 ************************************************/
HexString::HexString(const HexString &other):
    Value(other)
{
    d->mValid = true;
}


/************************************************
 *
 ************************************************/
HexString &HexString::operator =(const HexString &other)
{
    d = other.d;
    d->mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
QByteArray HexString::value() const
{
    assert(d->mType == Type::HexString);
    return d->mStringValue;
}


/************************************************
 *
 ************************************************/
void HexString::setValue(const QByteArray &value)
{
    assert(d->mType == Type::HexString);
    if (d->mValid)
        d->mStringValue = value;
}


//###############################################
// PDF Literal String
//###############################################
Link::Link(quint32 objNum, quint16 genNum):
    Value(Type::Link)
{
    d->mLinkObjNum = objNum;
    d->mLinkGenNum = genNum;
    d->mValid = true;
}



/************************************************
 *
 ************************************************/
Link::Link(const Object &obj):
    Value(Type::Link)
{
    d->mLinkObjNum = obj.objNum();
    d->mLinkGenNum = obj.genNum();
    d->mValid = true;
}


/************************************************
 *
 ************************************************/
Link::Link(const Link &other):
    Value(other)
{
    d->mValid = true;
}


/************************************************
 *
 ************************************************/
Link &Link::operator =(const Link &other)
{
    d = other.d;
    d->mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
Link &Link::operator =(const Object &obj)
{
    setObjNum(obj.objNum());
    setGenNum(obj.genNum());
    d->mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
quint32 Link::objNum() const
{
    assert(d->mType == Type::Link);
    return d->mLinkObjNum;
}


/************************************************
 *
 ************************************************/
void Link::setObjNum(quint32 value)
{
    assert(d->mType == Type::Link);
    if (d->mValid)
        d->mLinkObjNum = value;
}


/************************************************
 *
 ************************************************/
quint16 Link::genNum() const
{
    assert(d->mType == Type::Link);
    return d->mLinkGenNum;
}


/************************************************
 *
 ************************************************/
void Link::setGenNum(quint16 value)
{
    assert(d->mType == Type::Link);
    if (d->mValid)
        d->mLinkGenNum = value;
}


//###############################################
// PDF Literal String
//###############################################
LiteralString::LiteralString(const QString &value):
    Value(Type::LiteralString)
{
    d->mValid = true;
    d->mStringValue = value.toLocal8Bit();
}


/************************************************
 *
 ************************************************/
LiteralString::LiteralString(const LiteralString &other):
    Value(other)
{
    d->mValid = true;
}


/************************************************
 *
 ************************************************/
LiteralString &LiteralString::operator =(const LiteralString &other)
{
    d = other.d;
    d->mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
QByteArray LiteralString::value() const
{
    assert(d->mType == Type::LiteralString);
    return d->mStringValue;
}


/************************************************
 *
 ************************************************/
void LiteralString::setValue(const QByteArray &value)
{
    assert(d->mType == Type::LiteralString);
    if (d->mValid)
        d->mStringValue = value;
}


//###############################################
// PDF Name
//###############################################
Name::Name(const QString &name):
    Value(Type::Name)
{
    d->mValid = true;
    d->mStringValue = name.toLatin1();
}


/************************************************
 *
 ************************************************/
Name::Name(const Name &other):
    Value(other)
{
    d->mValid = true;
}


/************************************************
 *
 ************************************************/
Name &Name::operator =(const Name &other)
{
    d = other.d;
    d->mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
QString Name::value() const
{
    assert(d->mType == Type::Name);
    return d->mStringValue;
}


/************************************************
 *
 ************************************************/
void Name::setValue(const QString &value)
{
    assert(d->mType == Type::Name);
    if (d->mValid)
        d->mStringValue = value.toLatin1();
}


//###############################################
// PDF Null
//###############################################
Null::Null():
    Value(Type::Null)
{
    d->mValid = true;
}


/************************************************
 *
 ************************************************/
Null::Null(const Null &other):
    Value(other)
{
    d->mValid = true;
}


/************************************************
 *
 ************************************************/
Null &Null::operator =(const Null &other)
{
    d = other.d;
    d->mValid = true;
    return *this;
}


//###############################################
// PDF Number
//###############################################
Number::Number(double value):
    Value(Value::Type::Number)
{
    d->mNumberValue = value;
    d->mValid = true;
}


/************************************************
 *
 ************************************************/
Number::Number(const Number &other):
    Value(other)
{
    d->mValid = true;
}


/************************************************
 *
 ************************************************/
Number &Number::operator =(const Number &other)
{
    d = other.d;
    d->mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
double Number::value() const
{
    return d->mNumberValue;
}


/************************************************
 *
 ************************************************/
void Number::setValue(double value)
{
    if (isValid())
        d->mNumberValue = value;
}


#define DEBUG_INDENT 2
/************************************************
 *
 ************************************************/
void debugValue(QDebug dbg, const Value &value, int indent = 0)
{
    //QString indentStr = QString("%1").arg("", indent, ' ');

    if (!value.isValid())
    {
        dbg.space() << "INVALID";
    }

    switch (value.type())
    {

    case Value::Type::Undefined:
        dbg.space() << "Undefined PDF value";
        break;

    case Value::Type::Array:
        dbg.nospace() << "[ ";
        foreach (const Value v, value.toArray().values())
        {
            dbg.nospace() <<  v << " ";
        }
        dbg.nospace() << "]";
        break;


    case Value::Type::Dict:
    {
        const QMap<QString, Value> &values = value.toDict().values();
        dbg.nospace() << " <<\n";
        for (auto i = values.constBegin(); i != values.constEnd(); ++i)
        {
            QString s = QString("   %1/%2 ").arg("", indent, ' ').arg(i.key());
            dbg.nospace() << s.toLocal8Bit().data();
            debugValue(dbg, i.value(),  s.length());
            dbg.nospace() << "\n";
        }
        dbg.nospace() << QString(" %1>>").arg("", indent, ' ').toLatin1().data();
        break;
    }

    // Trivial types
    case Value::Type::Bool:
        dbg.space() << (value.toBool().value() ? "true" : "false");
        break;

    case Value::Type::HexString:
        dbg.space() << value.toHexString().value();
        break;

    case Value::Type::Link:
        dbg.space() << value.toLink().objNum() << value.toLink().genNum() << "R";
        break;

    case Value::Type::LiteralString:
        dbg.space() << value.toLiteralString().value();
        break;

    case Value::Type::Name:
        dbg.space() << QString("/" + value.toName().value()).toLocal8Bit().data();
        break;

    case Value::Type::Null:
        dbg.space() << "null";
        break;

    case Value::Type::Number:
        dbg.nospace() << value.toNumber().value();
        break;
    }
}


/************************************************
 *
 ************************************************/
QDebug operator<<(QDebug dbg, const Value &value)
{
    debugValue(dbg, value, 0);
    return dbg;
}
