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
#include "pdfobject.h"
#include <QThreadStorage>
#include <QTextCodec>
#include <QDebug>

using namespace PDF;


//###############################################
// PDF Value
//###############################################
Value::Value():
    mType(Type::Undefined),
    mValid(false),
    mNumberValue(0),
    mLinkObjNum(0),
    mLinkGenNum(0),
    mBoolValue(false)
{
}


/************************************************
 *
 ************************************************/
Value::Value(Value::Type type):
    mType(type),
    mValid(false),
    mNumberValue(0),
    mLinkObjNum(0),
    mLinkGenNum(0),
    mBoolValue(false)
{
}


/************************************************
 *
 ************************************************/
Value::Value(const Value &other):
    mType(        other.mType),
    mValid(       other.mValid),
    mArrayValues( other.mArrayValues),
    mDictValues ( other.mDictValues),
    mStringValue( other.mStringValue),
    mNumberValue( other.mNumberValue),
    mLinkObjNum(  other.mLinkObjNum),
    mLinkGenNum(  other.mLinkGenNum),
    mBoolValue(   other.mBoolValue)
{

}


/************************************************
 *
 ************************************************/
void Value::setValid(bool value)
{
    mValid = value;
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
    mType        = other.mType;
    mValid       = other.mValid;

    mArrayValues = other.mArrayValues;
    mBoolValue   = other.mBoolValue;
    mDictValues  = other.mDictValues;
    mStringValue = other.mStringValue;
    mLinkObjNum  = other.mLinkObjNum;
    mLinkGenNum  = other.mLinkGenNum;
    mNumberValue = other.mNumberValue;

    return *this;
}


/************************************************
 *
 ************************************************/
const Array &Value::asArray(bool *ok) const
{
    return valueAs<Array>(Type::Array, ok);
}


/************************************************
 *
 ************************************************/
Array &Value::asArray(bool *ok)
{
    return valueAs<Array>(Type::Array, ok);
}


/************************************************
 *
 ************************************************/
const Bool &Value::asBool(bool *ok) const
{
    return valueAs<Bool>(Type::Bool, ok);
}


/************************************************
 *
 ************************************************/
Bool &Value::asBool(bool *ok)
{
    return valueAs<Bool>(Type::Bool, ok);
}


/************************************************
 *
 ************************************************/
const Dict &Value::asDict(bool *ok) const
{
    return valueAs<Dict>(Type::Dict, ok);
}


/************************************************
 *
 ************************************************/
Dict &Value::asDict(bool *ok)
{
    return valueAs<Dict>(Type::Dict, ok);
}


/************************************************
 *
 ************************************************/
const HexString &Value::asHexString(bool *ok) const
{
    return valueAs<HexString>(Type::HexString, ok);
}


/************************************************
 *
 ************************************************/
HexString &Value::asHexString(bool *ok)
{
    return valueAs<HexString>(Type::HexString, ok);
}


/************************************************
 *
 ************************************************/
const Link &Value::asLink(bool *ok) const
{
    return valueAs<Link>(Type::Link, ok);
}


/************************************************
 *
 ************************************************/
Link &Value::asLink(bool *ok)
{
    return valueAs<Link>(Type::Link, ok);
}


/************************************************
 *
 ************************************************/
const LiteralString &Value::asLiteralString(bool *ok) const
{
    return valueAs<LiteralString>(Type::LiteralString, ok);
}


/************************************************
 *
 ************************************************/
LiteralString &Value::asLiteralString(bool *ok)
{
    return valueAs<LiteralString>(Type::LiteralString, ok);
}


/************************************************
 *
 ************************************************/
const Name &Value::asName(bool *ok) const
{
    return valueAs<Name>(Type::Name, ok);
}


/************************************************
 *
 ************************************************/
Name &Value::asName(bool *ok)
{
    return valueAs<Name>(Type::Name, ok);
}


/************************************************
 *
 ************************************************/
const Null &Value::asNull(bool *ok) const
{
    return valueAs<Null>(Type::Null, ok);
}


/************************************************
 *
 ************************************************/
Null &Value::asNull(bool *ok)
{
    return valueAs<Null>(Type::Null, ok);
}


/************************************************
 *
 ************************************************/
const Number &Value::asNumber(bool *ok) const
{
    return valueAs<Number>(Type::Number, ok);
}


/************************************************
 *
 ************************************************/
Number &Value::asNumber(bool *ok)
{
    return valueAs<Number>(Type::Number, ok);
}


/************************************************
 *
 ************************************************/
bool Value::operator==(const Value &other) const
{
    if (mType != other.mType)
        return false;

    switch (mType)
    {
    case Type::Undefined:       return true;
    case Type::Array:           return mArrayValues == other.mArrayValues;
    case Type::Bool:            return mBoolValue   == other.mBoolValue;
    case Type::Dict:            return mDictValues  == other.mDictValues;
    case Type::HexString:       return mStringValue == other.mStringValue;
    case Type::Link:            return mLinkObjNum  == other.mLinkObjNum && mLinkGenNum == other.mLinkGenNum;
    case Type::LiteralString:   return mStringValue == other.mStringValue;
    case Type::Name:            return mStringValue == other.mStringValue;
    case Type::Null:            return true;
    case Type::Number:          return mNumberValue == other.mNumberValue;
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
    mValid = true;
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
    mValid = true;
    mArrayValues.resize(4);
    mArrayValues[0] = PDF::Number(rect.left());
    mArrayValues[1] = PDF::Number(rect.bottom());
    mArrayValues[2] = PDF::Number(rect.right());
    mArrayValues[3] = PDF::Number(rect.top());
}


/************************************************
 *
 ************************************************/
Array::Array(const Array &other):
    Value(other)
{
    mValid = true;
}


/************************************************
 *
 ************************************************/
Array &Array::operator =(const Array &other)
{
    Value::operator =(other);
    mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
const QVector<Value> Array::values() const
{
    assert(mType == Type::Array);
    return mArrayValues;
}


/************************************************
 *
 ************************************************/
QVector<Value> &Array::values()
{
    assert(mType == Type::Array);
    return mArrayValues;
}


/************************************************
 *
 ************************************************/
void Array::append(const Value &value)
{
    assert(mType == Type::Array);
    mArrayValues.append(value);
}


/************************************************
 *
 ************************************************/
int Array::count(const Value &value) const
{
    return mArrayValues.count(value);
}


/************************************************
 *
 ************************************************/
void Array::remove(int i)
{
    assert(mType == Type::Array);
    mArrayValues.remove(i);
}


/************************************************
 *
 ************************************************/
Array &Array::operator<<(const Value &value)
{
    assert(mType == Type::Array);
    mArrayValues.operator <<(value);
    return *this;
}


//###############################################
// PDF Bool
//###############################################
Bool::Bool(bool value):
    Value(Type::Bool)
{
    mBoolValue = value;
    mValid = true;
}


/************************************************
 *
 ************************************************/
Bool::Bool(const Bool &other):
    Value(other)
{
    mValid = true;
}


/************************************************
 *
 ************************************************/
Bool &Bool::operator =(const Bool &other)
{
    Value::operator =(other);
    mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
bool Bool::value() const
{
    assert(mType == Type::Bool);
    return mBoolValue;
}


/************************************************
 *
 ************************************************/
void Bool::setValue(bool value)
{
    assert(mType == Type::Bool);
    if (isValid())
        mBoolValue = value;
}


//###############################################
// PDF Dictionary
//###############################################
Dict::Dict():
    Value(Type::Dict)
{
    mValid = true;
}


/************************************************
 *
 ************************************************/
Dict::Dict(const Dict &other):
    Value(other)
{
     mValid = true;
}


/************************************************
 *
 ************************************************/
Dict &Dict::operator =(const Dict &other)
{
    Value::operator =(other);
    mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
QMap<QString, Value> Dict::values()
{
    assert(mType == Type::Dict);
    return mDictValues;
}


/************************************************
 *
 ************************************************/
const QMap<QString, Value> &Dict::values() const
{
    assert(mType == Type::Dict);
    return mDictValues;
}


/************************************************
 *
 ************************************************/
int Dict::size() const
{
    assert(mType == Type::Dict);
    return mDictValues.size();
}


/************************************************
 *
 ************************************************/
bool Dict::isEmpty() const
{
    assert(mType == Type::Dict);
    return mDictValues.isEmpty();
}


/************************************************
 *
 ************************************************/
bool Dict::contains(const QString &key) const
{
    assert(mType == Type::Dict);
    return mDictValues.contains(key);
}


/************************************************
 *
 ************************************************/
const Value Dict::value(const QString &key, const Value &defaultValue) const
{
    assert(mType == Type::Dict);
    return mDictValues.value(key, defaultValue);
}


/************************************************
 *
 ************************************************/
Value &Dict::operator[](const QString &key)
{
    assert(mType == Type::Dict);
    return mDictValues[key];
}


/************************************************
 *
 ************************************************/
const Value Dict::operator[](const QString &key) const
{
    assert(mType == Type::Dict);
    return mDictValues[key];
}


/************************************************
 *
 ************************************************/
void Dict::insert(const QString &key, const Value &value)
{
    if (isValid())
    {
        assert(mType == Type::Dict);
        mDictValues.insert(key, value);
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
    assert(mType == Type::Dict);
    QStringList res = mDictValues.keys();
    res.sort();
    return res;
}


//###############################################
// PDF Hexadecimal String
//###############################################
HexString::HexString(const QString &value):
    Value(Type::HexString)
{
    mValid = true;
    mStringValue = value.toLocal8Bit().toHex();
}


/************************************************
 *
 ************************************************/
HexString::HexString(const HexString &other):
    Value(other)
{
    mValid = true;
}


/************************************************
 *
 ************************************************/
HexString &HexString::operator =(const HexString &other)
{
    Value::operator =(other);
    mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
QByteArray HexString::value() const
{
    assert(mType == Type::HexString);
    return mStringValue;
}


/************************************************
 *
 ************************************************/
void HexString::setValue(const QByteArray &value)
{
    assert(mType == Type::HexString);
    if (mValid)
        mStringValue = value;
}


/************************************************
 * A hexadecimal string is written as a sequence of
 * hexadecimal digits (0–9 and either A –F or a–f )
 * enclosed within angle brackets (< and >):
 *    < 4E6F762073686D6F7A206B6120706F702E >
 ************************************************/
QString HexString::toString() const
{
    QByteArray data;
    data.reserve(mStringValue.length() + 1);

    bool first = true;
    char r = 0;
    foreach (auto c, mStringValue)
    {
        switch (c)
        {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        {
            if (first)
                r = c - '0';
            else
                data.append(r * 16 + c -'0');

            first = !first;
            break;
        }

        case 'A': case 'B': case 'C':
        case 'D': case 'E': case 'F':
        {
            if (first)
                r = c - 'A' + 10;
            else
                data.append(r * 16 + c -'A' + 10);

            first = !first;
            break;
        }

        case 'a': case 'b': case 'c':
        case 'd': case 'e': case 'f':
        {
            if (first)
                r = c - 'a' + 10;
            else
                data.append(r * 16 + c -'a' + 10);

            first = !first;
            break;
        }
        }
    }

    if (!first)
        data.append(r * 16);

    return QTextCodec::codecForUtfText(data, QTextCodec::codecForName("UTF-8"))->toUnicode(data);
}


//###############################################
// PDF Literal String
//###############################################
Link::Link(quint32 objNum, quint16 genNum):
    Value(Type::Link)
{
    mLinkObjNum = objNum;
    mLinkGenNum = genNum;
    mValid = true;
}



/************************************************
 *
 ************************************************/
Link::Link(const Object &obj):
    Value(Type::Link)
{
    mLinkObjNum = obj.objNum();
    mLinkGenNum = obj.genNum();
    mValid = true;
}


/************************************************
 *
 ************************************************/
Link::Link(const Link &other):
    Value(other)
{
    mValid = true;
}


/************************************************
 *
 ************************************************/
Link &Link::operator =(const Link &other)
{
    Value::operator =(other);
    mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
Link &Link::operator =(const Object &obj)
{
    mLinkObjNum = obj.objNum();
    mLinkGenNum = obj.genNum();
    mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
quint32 Link::objNum() const
{
    assert(mType == Type::Link);
    return mLinkObjNum;
}


/************************************************
 *
 ************************************************/
void Link::setObjNum(quint32 value)
{
    assert(mType == Type::Link);
    if (mValid)
        mLinkObjNum = value;
}


/************************************************
 *
 ************************************************/
quint16 Link::genNum() const
{
    assert(mType == Type::Link);
    return mLinkGenNum;
}


/************************************************
 *
 ************************************************/
void Link::setGenNum(quint16 value)
{
    assert(mType == Type::Link);
    if (mValid)
        mLinkGenNum = value;
}


//###############################################
// PDF Literal String
//###############################################
LiteralString::LiteralString(const QString &value):
    Value(Type::LiteralString)
{
    mValid = true;
    mStringValue = value.toLocal8Bit();
}


/************************************************
 *
 ************************************************/
LiteralString::LiteralString(const LiteralString &other):
    Value(other)
{
    mValid = true;
}


/************************************************
 *
 ************************************************/
LiteralString &LiteralString::operator =(const LiteralString &other)
{
    Value::operator =(other);
    mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
QByteArray LiteralString::value() const
{
    assert(mType == Type::LiteralString);
    return mStringValue;
}


/************************************************
 *
 ************************************************/
void LiteralString::setValue(const QByteArray &value)
{
    assert(mType == Type::LiteralString);
    if (mValid)
        mStringValue = value;
}


/************************************************
 * Literal Strings
 *
 * A literal string is written as an arbitrary number of characters enclosed
 * in parentheses. Any characters may appear in a string except unbalanced
 * parentheses and the backslash, which must be treated specially.
 * Balanced pairs of parentheses within a string require no special treatment.
 *
 * The following are valid literal strings:
 *  ( This is a string )
 *  ( Strings may contain newlines
 *  and such. )
 *  ( Strings may contain balanced parentheses ( ) and
 *  special characters ( * ! & } ^ % and so on ) . )
 *  ( The following is an empty string . )
 *  ( )
 *  ( It has zero ( 0 ) length . )
 *
 * Within a literal string, the backslash (\) is used as an escape character
 * for various purposes, such as to include newline characters, nonprinting
 * ASCII characters, unbalanced parentheses, or the backslash character
 * itself in the string. The character immediately following the backslash
 * determines its precise interpretation (see Table 3.2). If the character
 * following the backslash is not one of those shown in the table, the
 * backslash is ignored.
 *
 * SEQUENCE     MEANING
 *  \n          Line feed (LF)
 *  \r          Carriage return (CR)
 *  \t          Horizontal tab (HT)
 *  \b          Backspace (BS)
 *  \f          Form feed (FF)
 *  \(          Left parenthesis
 *  \)          Right parenthesis
 *  \\          Backslash
 *  \ddd        Character code ddd (octal)
 *
 * If a string is too long to be conveniently placed on a single line,
 * it may be split across multiple lines by using the backslash character at
 * the end of a line to indicate that the string continues on the following
 * line. The backslash and the end-of-line marker following it are not
 * considered part of the string. For example:
 *  ( These \
 *  two strings \
 *  are the same . )
 *  ( These two strings are the same . )
 *
 * If an end-of-line marker appears within a literal string without a
 * preceding backslash, the result is equivalent to \n (regardless of
 * whether the end-of-line marker was a carriage return, a line feed,
 * or both). For example:
 *  ( This string has an end−of−line at the end of it .
 *  )
 *  ( So does this one .\n )
 *
 * The \ddd escape sequence provides a way to represent characters outside
 * the printable ASCII character set. For example:
 *  ( This string contains \245two octal characters\307 . )
 * The number ddd may consist of one, two, or three octal digits, with
 * high-order overflow ignored. It is required that three octal digits be
 * used, with leading zeros as needed, if the next character of the string
 * is also a digit. For example, the literal
 *  ( \0053 )
 * denotes a string containing two characters, \005 (Control-E) followed
 * by the digit 3, whereas both
 *  ( \053 )
 * and
 *  ( \53 )
 * denote strings containing the single character \053, a plus sign (+).
 ************************************************/
QString LiteralString::toString() const
{
    QByteArray res;
    res.reserve(mStringValue.length());

    bool esc = false;
    for (int i=0; i<mStringValue.length(); ++i)
    {
        char c = mStringValue.at(i);
        switch (c)
        {

        // Backslash .......................
        case '\\':
            esc = !esc;
            if (!esc)
                res.append(c);
            break;

        // Line feed (LF) ..................
        case 'n':
            res.append(esc ? '\n' : 'n');
            esc = false;
            break;

        // Carriage return (CR) ............
        case 'r':
            res.append(esc ? '\r' : 'r');
            esc = false;
            break;

        // Horizontal tab (HT) .............
        case 't':
            res.append(esc ? '\t' : 't');
            esc = false;
            break;

        // Backspace (BS) ..................
        case 'b':
            res.append(esc ? '\b' : 'b');
            esc = false;
            break;

        // Form feed (FF) ..................
        case 'f':
            res.append(esc ? '\f' : 'f');
            esc = false;
            break;

        // Character code ddd (octal) ......
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
            if (esc)
            {
                esc = false;
                char n = c-'0';
                int l = qMin(i+3, mStringValue.length());
                for(++i; i<l; ++i)
                {
                    c = mStringValue.at(i);
                    if (c < '0' || c > '7' )
                        break;

                    n = n * 8 + c - '0';
                }
                res.append(n);
                --i;
            }
            else
            {
                res.append(c);
            }
            break;

        case '\n':
            if (esc)
            {
                if (i+1<mStringValue.length() && mStringValue.at(i+1) == '\r')
                    ++i;
            }
            else
            {
                res.append('\n');
            }
            esc = false;
            break;

        case '\r':
            if (esc)
            {
                if (i+1<mStringValue.length() && mStringValue.at(i+1) == '\n')
                    ++i;
            }
            else
            {
                res.append('\r');
            }
            esc = false;
            break;

        default:
            esc = false;
            res.append(c);
            break;
        }
    }

    return QString::fromLocal8Bit(res);
}


//###############################################
// PDF Name
//###############################################
Name::Name(const QString &name):
    Value(Type::Name)
{
    mValid = true;
    mStringValue = name.toLatin1();
}


/************************************************
 *
 ************************************************/
Name::Name(const Name &other):
    Value(other)
{
    mValid = true;
}


/************************************************
 *
 ************************************************/
Name &Name::operator =(const Name &other)
{
    Value::operator =(other);
    mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
QString Name::value() const
{
    assert(mType == Type::Name);
    return mStringValue;
}


/************************************************
 *
 ************************************************/
void Name::setValue(const QString &value)
{
    assert(mType == Type::Name);
    if (mValid)
        mStringValue = value.toLatin1();
}


//###############################################
// PDF Null
//###############################################
Null::Null():
    Value(Type::Null)
{
    mValid = true;
}


/************************************************
 *
 ************************************************/
Null::Null(const Null &other):
    Value(other)
{
    mValid = true;
}


/************************************************
 *
 ************************************************/
Null &Null::operator =(const Null &other)
{
    Value::operator =(other);
    mValid = true;
    return *this;
}


//###############################################
// PDF Number
//###############################################
Number::Number(double value):
    Value(Value::Type::Number)
{
    mNumberValue = value;
    mValid = true;
}


/************************************************
 *
 ************************************************/
Number::Number(const Number &other):
    Value(other)
{
    mValid = true;
}


/************************************************
 *
 ************************************************/
Number &Number::operator =(const Number &other)
{
    Value::operator =(other);
    mValid = true;
    return *this;
}


/************************************************
 *
 ************************************************/
double Number::value() const
{
    return mNumberValue;
}


/************************************************
 *
 ************************************************/
void Number::setValue(double value)
{
    if (isValid())
        mNumberValue = value;
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
        foreach (const Value v, value.asArray().values())
        {
            dbg.nospace() <<  v << " ";
        }
        dbg.nospace() << "]";
        break;


    case Value::Type::Dict:
    {
        const QMap<QString, Value> &values = value.asDict().values();
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
        dbg.space() << (value.asBool().value() ? "true" : "false");
        break;

    case Value::Type::HexString:
        dbg.space() << value.asHexString().value();
        break;

    case Value::Type::Link:
        dbg.space() << value.asLink().objNum() << value.asLink().genNum() << "R";
        break;

    case Value::Type::LiteralString:
        dbg.space() << value.asLiteralString().value();
        break;

    case Value::Type::Name:
        dbg.space() << QString("/" + value.asName().value()).toLocal8Bit().data();
        break;

    case Value::Type::Null:
        dbg.space() << "null";
        break;

    case Value::Type::Number:
        dbg.nospace() << value.asNumber().value();
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


/************************************************
 *
 ************************************************/
template <typename T>
const T &Value::valueAs(Value::Type type, bool *ok) const
{
    if (mType == type)
    {
        if (ok) *ok = true;
        return *(static_cast<const T*>(this));
    }
    else
    {
        if (ok) *ok = false;
        static QThreadStorage<T> stor;
        stor.localData().setValid(false);
        return stor.localData();
    }
}


/************************************************
 *
 ************************************************/
template <typename T>
T &Value::valueAs(Value::Type type, bool *ok)
{
    if (mType == type)
    {
        if (ok) *ok = true;
        return *(static_cast<T*>(this));
    }
    else
    {
        if (ok) *ok = false;
        static QThreadStorage<T> stor;
        stor.localData().setValid(false);
        return stor.localData();
    }
}
