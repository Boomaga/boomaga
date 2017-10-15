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


#ifndef PDFVALUE_P_H
#define PDFVALUE_P_H

#include "pdfvalue.h"
#include <QSharedData>
#include <QIODevice>

using namespace PdfParser;

//###############################################
// PDF Value
//###############################################
class PdfParser::ValueData: public QSharedData
{
public:
    ValueData(Value::Type type):
        mType(type),
        mValid(false)
    {
    }

    ValueData(const ValueData &other):
        QSharedData(other),
        mType(other.mType),
        mValid(other.mValid)
    {
    }

    ValueData &operator =(const ValueData &other)
    {
        mValid = other.mValid;
        mType  = other.mType;
        return *this;
    }

    virtual ~ValueData()
    {
    }

    ValueData *privateData(const Value &other) const;

    template <typename T>
    T  *as() {
        return static_cast<T*>(this);
    }

    template <typename T>
    const T  *as() const {
        return static_cast<const T*>(this);
    }

    Value::Type mType;
    bool mValid;
};


//###############################################
// PDF Array
//###############################################
class ArrayData: public ValueData
{
public:
    ArrayData():
        ValueData(Value::Type::Array)
    {
    }

    ArrayData(const ArrayData &other):
        ValueData(other),
        mValues(other.mValues)
    {
    }

    ArrayData &operator =(const ArrayData &other)
    {
        mValid  = other.mValid;
        mType   = other.mType;
        mValues = other.mValues;
        return *this;
    }

    QVector<Value> mValues;
};


//###############################################
// PDF Bool
//###############################################
class BoolData: public ValueData
{
public:
    BoolData():
        ValueData(Value::Type::Bool),
        mValue(false)
    {
    }

    BoolData(const BoolData &other):
        ValueData(other),
        mValue(other.mValue)
    {
    }

    BoolData &operator =(const BoolData &other)
    {
        mValid = other.mValid;
        mType  = other.mType;
        mValue = other.mValue;
        return *this;
    }

    bool mValue;
};


//###############################################
// PDF Dictionary
//###############################################
class DictData: public ValueData
{
public:
    DictData():
        ValueData(Value::Type::Dict)
    {
    }

    DictData(const DictData &other):
        ValueData(other),
        mValues(other.mValues)
    {
    }

    DictData &operator =(const DictData &other)
    {
        mValid  = other.mValid;
        mType   = other.mType;
        mValues = other.mValues;
        return *this;
    }

    QMap<QString, Value> mValues;
};


//###############################################
// PDF Literal String
//###############################################
class LinkData: public ValueData
{
public:
    LinkData(quint32 objNum = 0, quint16 genNum = 0):
      ValueData(Value::Type::Link),
      mObjNum(0),
      mGenNum(0)
    {
    }

    LinkData(const LinkData &other):
        ValueData(other),
        mObjNum(other.mObjNum),
        mGenNum(other.mGenNum)
    {
    }


    LinkData &operator =(const LinkData &other)
    {
        mValid  = other.mValid;
        mType   = other.mType;
        mObjNum = other.mObjNum;
        mGenNum = other.mGenNum;
        return *this;
    }

    quint32 mObjNum;
    quint16 mGenNum;
};


//###############################################
// PDF Hexadecimal String
//###############################################
class HexStringData: public ValueData
{
public:
    HexStringData():
        ValueData(Value::Type::HexString)
    {
    }

    HexStringData(const HexStringData &other):
        ValueData(other),
        mValue(other.mValue)
    {
    }

    HexStringData &operator =(const HexStringData &other)
    {
        mValid = other.mValid;
        mType  = other.mType;
        mValue = other.mValue;
        return *this;
    }

    QByteArray mValue;
};


class LiteralStringData: public ValueData
{
public:
    LiteralStringData():
        ValueData(Value::Type::LiteralString)
    {
    }

    LiteralStringData(const LiteralStringData &other):
        ValueData(other),
        mValue(other.mValue)
    {
    }

    LiteralStringData &operator =(const LiteralStringData &other)
    {
        mValid = other.mValid;
        mType  = other.mType;
        mValue = other.mValue;
        return *this;
    }

    QByteArray mValue;
};

class NameData: public ValueData
{
public:
    NameData():
        ValueData(Value::Type::Name)
    {
    }

    NameData(const NameData &other):
        ValueData(other),
        mValue(other.mValue)
    {
    }


    NameData &operator =(const NameData &other)
    {
        mValid = other.mValid;
        mType  = other.mType;
        mValue = other.mValue;
        return *this;
    }

    QString mValue;
};

class NullData: public ValueData
{
public:
    NullData():
        ValueData(Value::Type::Null)
    {
    }

    NullData(const NullData &other):
        ValueData(other)
    {
    }

    NullData &operator =(const NullData &other)
    {
        mValid = other.mValid;
        mType  = other.mType;
        return *this;
    }
};


class NumberData: public ValueData
{
public:
    NumberData(double value = 0):
        ValueData(Value::Type::Number),
        mValue(value)
    {
    }

    NumberData(const NumberData &other):
        ValueData(other),
        mValue(other.mValue)
    {
    }

    NumberData &operator =(const NumberData &other)
    {
        mValid = other.mValid;
        mType  = other.mType;
        mValue = other.mValue;
        return *this;
    }

    double mValue;
};


#endif // PDFVALUE_P_H
