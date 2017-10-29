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
#include <QRectF>
#include <QDebug>

namespace PDF {

typedef quint32 ObjNum;
typedef quint16 GenNum;

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
    bool isValid() const;

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
    const Array &toArray(bool *ok = NULL) const;
          Array &toArray(bool *ok = NULL);

    /// Converts the value to a PdfBool and returns it.
    /// If type() is not Bool, an empty PdfBool will be returned.
    const Bool &toBool(bool *ok = NULL) const;
          Bool &toBool(bool *ok = NULL);

    /// Converts the value to a PdfDict and returns it.
    /// If type() is not Dict, an empty PdfDict will be returned.
    const Dict &toDict(bool *ok = nullptr) const;
          Dict &toDict(bool *ok = nullptr);

    /// Converts the value to a PdfHexString and returns it.
    /// If type() is not HexString, an empty PdfHexString will be returned.
    const HexString &toHexString(bool *ok = NULL) const;
          HexString &toHexString(bool *ok = NULL);

    /// Converts the value to a PdfLink and returns it.
    /// If type() is not Link, an empty PdfLink will be returned.
    const Link &toLink(bool *ok = NULL) const;
          Link &toLink(bool *ok = NULL);

    /// Converts the value to a PdfLiteralString and returns it.
    /// If type() is not LiteralString, an empty PdfLiteralString will be returned.
    const LiteralString &toLiteralString(bool *ok = NULL) const;
          LiteralString &toLiteralString(bool *ok = NULL);

    /// Converts the value to a PdfName and returns it.
    /// If type() is not Name, an empty PdfName will be returned.
    const Name &toName(bool *ok = NULL) const;
          Name &toName(bool *ok = NULL);

    /// Converts the value to a PdfNull and returns it.
    /// If type() is not Null, an empty PdfNull will be returned.
    const Null &toNull(bool *ok = NULL) const;
          Null &toNull(bool *ok = NULL);

    /// Converts the value to a PdfNum and returns it.
    /// If type() is not Num, an empty PdfNum will be returned.
    const Number  &toNumber(bool *ok = NULL) const;
          Number  &toNumber(bool *ok = NULL);


    /// Returns true if the value is equal to other.
    bool operator==(const Value &other) const;

    /// Returns true if the value is not equal to other.
    bool operator!=(const Value &other) const;

protected:
    Value(Type type);
    Value(ValueData *data);
    void setValid(bool value);
    QSharedDataPointer<ValueData> d;
};


class Array: public Value
{
public:
    Array();
    Array(const QRectF &rect);
    Array(const Array &other);
    Array &operator =(const Array &other);

    const QVector<Value> values() const;
    QVector<Value> &values();

    /// Inserts value at the end of the array.
    void append(const Value &value);

    /// Returns the value at index position i as a modifiable reference.
    /// i must be a valid index position in the vector (i.e., 0 <= i < size()).
    Value &operator[](int i) { return values()[i]; }

    /// This is an overloaded function.
    /// Same as at(i).
    const Value &operator[](int i) const { return values()[i]; }

    /// Returns the item at index position i in the vector.
    /// i must be a valid index position in the vector (i.e., 0 <= i < size()).
    /// \sa value() and operator[]().
    const Value &at(int i) const { return values().at(i); }

    /// Returns the number of occurrences of value in the vector.
    /// This function requires the value type to have an implementation of operator==().
    // \sa contains() and indexOf().
    int count(const Value &value) const;

    /// This is an overloaded function.
    /// Same as size().
    int count() const { return values().count(); }


    /// Returns the number of items in the vector.
    /// \sa isEmpty() and resize().
    int size() const { return values().size(); }

    /// Removes the element at index position i.
    void remove(int i);

    /// Inserts value at the end of the array and returns a reference to this array.
    /// \sa operator+=() and append().
    Array &operator<<(const Value &value);


};


class Bool: public Value
{
public:
    Bool(bool value = false);
    Bool(const Bool &other);
    Bool &operator =(const Bool &other);

    /// Returns true if the value is not equal to other.
    bool operator!=(const Bool &other) const { return value() != other.value(); }

    /// Returns true if the value is not equal to other.
    bool operator!=(bool other) const { return value() != other; }


    /// Returns true if the value is equal to other.
    bool operator==(const Bool &other) const { return value() == other.value(); }

    /// Returns true if the value is equal to other.
    bool operator==(bool other) const { return value() == other; }

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

    /// Returns the value associated with the key key.
    /// If the dictionary contains no item with key key, the function returns defaultValue.
    /// If no defaultValue is specified, the function returns a Value with type Undefined.
    /// \sa key(), values(), contains(), and operator[]().
    const Value value(const QString &key, const Value &defaultValue = Value()) const;


    /// Returns the value associated with the key key as a modifiable reference.
    /// If the dictionary contains no item with key key, the function inserts a Value with type Undefined
    /// into the dictionary with key key, and returns a reference to it.
    /// \sa insert() and value().
    Value &operator[](const QString &key);

    /// This is an overloaded function.
    /// Same as value().
    const Value operator[](const QString &key) const;

    /// Inserts a new item with the key key and a value of value.
    /// If there is already an item with the key key, then that item's value is replaced with value.
    void insert(const QString &key, const Value &value);
    void insert(const QString &key, double value);

    QMap<QString, Value> values();
    const QMap<QString, Value> &values() const;

    /// Same as size().
    /// \sa size().
    int count() const { return size(); }

    /// Returns the number of (key, value) pairs in the dictionary.
    /// \sa isEmpty() and count().
    int size() const;

    /// Returns true if the dictionary contains no items; otherwise returns false.
    /// \sa size().
    bool isEmpty() const;

    /// Returns true if the dictionary contains an item with key key; otherwise returns false.
    /// \sa count().
    bool contains(const QString &key) const;

    /// Returns a list of all keys in this object. The list is sorted lexographically.
    QStringList keys() const;
};


class HexString: public Value
{
public:
    HexString(const QString &value = "");
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

    quint32 objNum() const;
    void setObjNum(quint32 value);

    quint16 genNum() const;
    void setGenNum(quint16 value);
};


class LiteralString: public Value
{
public:
    LiteralString(const QString &value = "");
    LiteralString(const LiteralString &other);
    LiteralString &operator =(const LiteralString &other);

    QByteArray value() const;
    void setValue(const QByteArray &value);
};


class Name: public Value
{
public:
    Name(const char *name = "");
    Name(const Name &other);
    Name &operator =(const Name &other);

    QString value() const;
    void setValue(const QString &value);
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

} // namespace PDF

QDebug operator<<(QDebug debug, const PDF::Value &value);

#include "pdfdata.h"
#include "pdftypes.h"

#endif // PDFVALUE_H
