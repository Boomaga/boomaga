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


#ifndef PDFVALUE_H
#define PDFVALUE_H

#include <QExplicitlySharedDataPointer>
#include <QVector>
#include <QMap>
#include <QStringList>

namespace PdfParser {


class ValueData;

class Array;
class Bool;
class Dict;
class HexString;
class Link;
class LiteralString;
class Name;
class Null;
class Number;
class Object;

class Value {
    friend class Object;
    friend class Reader;
    friend class Writer;
    friend class ValueData;
public:
    enum class Type {
        Undefined = 0,
        Array,
        Bool,
        Dict,
        HexString,
        Link,
        LiteralString,
        Name,
        Null,
        Number
    };

    Value();
    Value(const Value &other);
    virtual ~Value();

    Value &operator =(const Value &other);

    /// Returns true if the values isn't empty; otherwise returns false.
    bool isValid();

    Type type() const;

    /// Returns true if the value contains a PDF array.
    inline bool isArray() const { return type() == Type::Array; }

    /// Returns true if the value contains a boolean.
    inline bool isBool()  const { return type() == Type::Bool; }

    /// Returns true if the value contains a PDF dictonary.
    inline bool isDict()  const { return type() == Type::Dict; }

    /// Returns true if the value contains a PDF hexadecimal string".
    inline bool isHexString() const { return type() == Type::HexString; }

    /// Returns true if the value contains a PDF link, like "01 02 R".
    inline bool isLink()  const { return type() == Type::Link; }

    /// Returns true if the value contains a PDF literal string".
    inline bool isLiteralString() const { return type() == Type::LiteralString; }

    /// Returns true if the value contains a PDF name.
    inline bool isName()  const { return type() == Type::Name; }

    /// Returns true if the value is PDF null.
    inline bool isNull()  const  { return type() == Type::Null; }

    /// Returns true if the value contains a PDF number.
    inline bool isNumber() const { return type() == Type::Number; }

    /// Returns true if the value is undefined. This can happen
    /// in certain error cases as e.g. accessing a non existing key in a PdfDict.
    inline bool isUndefined() const { return type() == Type::Undefined; }



    /// Converts the value to a PdfArray and returns it.
    /// If type() is not Array, an empty PdfArray will be returned.
    Array toArray(bool *ok = NULL) const;

    /// Converts the value to a PdfBool and returns it.
    /// If type() is not Bool, an empty PdfBool will be returned.
    Bool toBool(bool *ok = NULL) const;

    /// Converts the value to a PdfDict and returns it.
    /// If type() is not Dict, an empty PdfDict will be returned.
    Dict toDict(bool *ok = nullptr) const;

    /// Converts the value to a PdfHexString and returns it.
    /// If type() is not HexString, an empty PdfHexString will be returned.
    HexString toHexString(bool *ok = NULL) const;

    /// Converts the value to a PdfLink and returns it.
    /// If type() is not Link, an empty PdfLink will be returned.
    Link toLink(bool *ok = NULL) const;

    /// Converts the value to a PdfLiteralString and returns it.
    /// If type() is not LiteralString, an empty PdfLiteralString will be returned.
    LiteralString toLiteralString(bool *ok = NULL) const;

    /// Converts the value to a PdfName and returns it.
    /// If type() is not Name, an empty PdfName will be returned.
    Name toName(bool *ok = NULL) const;

    /// Converts the value to a PdfNull and returns it.
    /// If type() is not Null, an empty PdfNull will be returned.
    Null toNull(bool *ok = NULL) const;

    /// Converts the value to a PdfNum and returns it.
    /// If type() is not Num, an empty PdfNum will be returned.
    Number  toNumber(bool *ok = NULL) const;


protected:
    Value(ValueData *data);
    void setValid(bool value);
    QExplicitlySharedDataPointer<ValueData> d;
};


class Array: public Value
{
public:
    Array();
    Array(const Array &other);
    Array &operator =(const Array &other);

    const QVector<Value> values() const;
    QVector<Value> values();

    /// Inserts value at the end of the array.
    void append(const Value &value);


};


class Bool: public Value
{
public:
    Bool();
    Bool(const Bool &other);
    Bool &operator =(const Bool &other);

    bool value() const;
    void setValue(bool value);
};



class Dict: public Value
{
public:
    Dict();
    Dict(const Dict &other);
    Dict &operator =(const Dict &other);

    class iterator
    {
    };


    Value value(const QString &key) const;
    /// Inserts a new item with the key key and a value of value.
    /// If there is already an item with the key key, then that item's value is replaced with value.
    void insert(const QString &key, const Value &value);
    void insert(const QString &key, double value);

    QMap<QString, Value> values();


    /// Returns a list of all keys in this object. The list is sorted lexographically.
    QStringList keys() const;
};


class HexString: public Value
{
public:
    HexString();
    HexString(const QString &value);
    HexString(const HexString &other);
    HexString &operator =(const HexString &other);

    QByteArray value() const;
    void setValue(const QByteArray &value);
};


class Link: public Value
{
public:
    Link(quint32 objNum = 0, quint16 genNum = 0);
    Link(const Link &other);
    Link(const Object &obj);
    Link &operator =(const Link &other);
    Link &operator =(const Object &obj);

    int objNum() const;
    void setObjNum(int value);

    int genNum() const;
    void setGenNum(int value);


};


class LiteralString: public Value
{
public:
    LiteralString();
    LiteralString(const LiteralString &other);
    LiteralString &operator =(const LiteralString &other);

    QByteArray value() const;
    void setValue(const QByteArray &value);
};

class Name: public Value
{
public:
    Name();
    Name(const Name &other);
    Name &operator =(const Name &other);

    QString value() const;
    void setName(const QString &value);
};

class Null: public Value
{
public:
    Null();
    Null(const Null &other);
    Null &operator =(const Null &other);
};


class Number: public Value
{
public:
    Number(double value = 0.0);
    Number(const Number &other);
    Number &operator =(const Number &other);

    double value() const;
    void setValue(double value);
};



} // namespace PdfParser


#include "pdfdata.h"
#include "pdftypes.h"

#endif // PDFVALUE_H
