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


#include <math.h>
#include <assert.h>
#include "pdfreader.h"
#include "pdfxref.h"
#include "pdfobject.h"
#include "pdferrors.h"
#include "pdfvalue.h"
#include <QFile>
#include <QTextCodec>
#include <QDebug>


namespace PDF {

struct ReaderData
{
public:

    ReaderData(const char *buf, const quint64 size, QTextCodec *textCodec):
        mData(buf),
        mSize(size),
        mTextCodec(textCodec)
    {

    }

    bool isDelim(quint64 pos) const;

    const char   *mData;
    const quint64 mSize;
    QTextCodec  *mTextCodec;

    bool compareStr(quint64 pos, const char *str) const;
    bool compareWord(quint64 pos, const char *str) const;
    quint64 skipSpace(quint64 pos) const;
    quint64 skipComment(quint64 pos) const;
    qint64 skipCRLF(quint64 pos) const;
    qint64 indexOf(const char *str, quint64 from) const;
    qint64 indexOfBack(const char *str, quint64 from) const;
    quint32 readUInt(quint64 *pos, bool *ok) const;
    double readNum(quint64 *pos, bool *ok) const;

    QString readNameString(quint64 *pos) const;
    qint64 readHexString(quint64 start, String *res) const;
    qint64 readLiteralString(qint64 start, String *res) const;

    qint64 readArray(quint64 start, Array *res) const;
    qint64 readDict(quint64 start, Dict *res) const;

    Value readValue(quint64 *pos) const;
    char operator[](quint64 index) const { return mData[index]; }
};

struct XRefStreamData
{
    struct Section {
        Section(PDF::ObjNum startObjNum = 0, quint32 count = 0):
            startObjNum(startObjNum),
            count(count)
        {
        }

        PDF::ObjNum startObjNum;
        quint32     count;
    };

    XRefStreamData(const char *buf, const quint64 size, const Dict &dict);
    quint64 readSection(quint64 pos, Section section, XRefTable *res);
    QVector<Section> &sections() {return mSections; }

private:
    inline quint64 readField(qint64 pos, int len) const;

    const char   *mData;
    const quint64 mSize;
    int mField1;
    int mField2;
    int mField3;
    int mEntryLen;
    QVector<Section> mSections;
};

class Reader::Cache{
public:
    Cache();
    ~Cache();

    const QByteArray getStream(PDF::ObjNum objNum, PDF::GenNum genNum) const;
    void  setStream(PDF::ObjNum objNum, PDF::GenNum genNum, QByteArray stream);

    void clear();

private:
    QHash<quint64, QByteArray> mStreams;
};


} // namespace PDF
using namespace PDF;


/************************************************
 *
 ************************************************/
Reader::Cache::Cache()
{
}


/************************************************
 *
 ************************************************/
Reader::Cache::~Cache()
{
}


/************************************************
 *
 ************************************************/
const QByteArray Reader::Cache::getStream(PDF::ObjNum objNum, PDF::GenNum genNum) const
{
    return mStreams.value((quint64(objNum) << 32) + genNum);
}


/************************************************
 *
 ************************************************/
void Reader::Cache::setStream(ObjNum objNum, GenNum genNum, QByteArray stream)
{
    mStreams.insert((quint64(objNum) << 32) + genNum, stream);
}


/************************************************
 *
 ************************************************/
void Reader::Cache::clear()
{
    mStreams.clear();
}



/************************************************
 *
 ************************************************/
XRefStreamData::XRefStreamData(const char *buf, const quint64 size, const Dict &dict):
    mData(buf),
    mSize(size),
    mField1(0),
    mField2(0),
    mField3(0),
    mEntryLen(0)
{
    Q_UNUSED(mSize)
    // W - An array of integers representing the size of the fields in a
    // single cross-reference entry.
    const Array &w = dict.value("W").asArray();
    if (!w.isValid())
        throw ReaderError("Incorrect XRef stream dictionary", 0);

    if (w.count()<3)
        throw ReaderError("Incorrect XRef stream dictionary", 0);

    for (int i=0; i<w.count(); ++i)
        mEntryLen += w.at(i).asNumber().value();

    mField1 = w.at(0).asNumber();
    mField2 = w.at(1).asNumber();
    mField3 = w.at(2).asNumber();


    if (!mEntryLen)
        throw ReaderError("Incorrect XRef stream dictionary", 0);

    // Index - An array containing a pair of integers for each subsection in
    // this section. The first integer is the first object number in the
    // subsection; the second integer is the number of entries in the subsection
    PDF::Array index = dict.value("Index").asArray();
    for (int s=0; s<index.count(); s+=2)
    {
        mSections << Section(
                    index.at(s).asNumber(),
                    index.at(s+1).asNumber());
    }

    // Default value: [0 Size].
    if (mSections.count() == 0)
        mSections << Section(
                     0,
                     dict.value("Size").asNumber());
}


/************************************************
 *
 ************************************************/
quint64 XRefStreamData::readSection(quint64 pos, XRefStreamData::Section section, XRefTable *res)
{
    quint64 end = section.count * mEntryLen;

    PDF::ObjNum objNum = section.startObjNum;
    for (; pos<end; pos+=mEntryLen)
    {
        int type = (mField1) ? readField(pos, mField1) : 1;

        switch (type)
        {

        // Type 0 entries define the linked list of free objects
        //  field 2 - The object number of the next free object.
        //  field 3 - The generation number.
        case 0:
            res->addFreeObject(objNum,
                               readField(pos + mField1 + mField2, mField3),
                               readField(pos + mField1,           mField2));
            break;

        // Type 1 entries define objects that are in use but are not compressed
        //  field 2 - The byte offset of the object
        //  field 3 - The generation number of the object.
        case 1:
            res->addUsedObject(objNum,
                               readField(pos + mField1 + mField2, mField3),
                               readField(pos + mField1,           mField2));
            break;

        // Type 2 entries define compressed objects.
        //  field 2 - The object number of the object stream.
        //  field 3 - The index of this object within the object stream.
        case 2:
            res->addCompressedObject(objNum,
                                     readField(pos + mField1,           mField2),
                                     readField(pos + mField1 + mField2, mField3));
            break;
        }

        ++objNum;
    }

    return end;
}


/************************************************
 *
 ************************************************/
quint64 XRefStreamData::readField(qint64 pos, int len) const
{
    quint64 res = 0;
    for (int i=0; i<len; ++i)
        res = (res << 8) + uchar(mData[pos++]);

    return res;
}


/************************************************
 *
 ************************************************/
bool ReaderData::isDelim(quint64 pos) const
{
    return isspace(mData[pos]) ||
            strchr("()<>[]{}/%", mData[pos]);
}


/************************************************
 *
 ************************************************/
bool ReaderData::compareStr(quint64 pos, const char *str) const
{
    size_t len = strlen(str);
    return (mSize - pos > len) && strncmp(mData + pos, str, len) == 0;
}


/************************************************
 *
 ************************************************/
bool ReaderData::compareWord(quint64 pos, const char *str) const
{
    size_t len = strlen(str);
    return (mSize - pos > len + 1) &&
            strncmp(mData + pos, str, len) == 0 &&
            isDelim(pos + len);
}


/************************************************
 *
 ************************************************/
quint64 ReaderData::skipSpace(quint64 pos) const
{
    while (pos < mSize)
    {
        while (pos < mSize && isspace(mData[pos]))
            pos++;

        if (mData[pos] != '%')
            return pos;

        pos = skipComment(pos);
    }

    return pos;
}


/************************************************
 *
 ************************************************/
quint64 ReaderData::skipComment(quint64 pos) const
{
    while (pos < mSize && mData[pos] != '\n' && mData[pos] != '\r')
        ++pos;

    return pos;
}


/************************************************
 *
 ************************************************/
qint64 ReaderData::skipCRLF(quint64 pos) const
{
    while (pos < mSize && (mData[pos] == '\n' || mData[pos] == '\r'))
        pos++;

    return pos;
}


/************************************************
 *
 ************************************************/
qint64 ReaderData::indexOf(const char *str, quint64 from) const
{
    size_t len = strlen(str);

    for (quint64 i = from; i<mSize-len; i++)
    {
        if (strncmp(mData + i, str, len) == 0)
            return i;
    }

    return -1;
}



/************************************************
 *
 ************************************************/
qint64 ReaderData::indexOfBack(const char *str, quint64 from) const
{
    size_t len = strlen(str);

    for (quint64 i = from - len + 1 ; i>0; i--)
    {
        if (strncmp(mData + i, str, len) == 0)
            return i;
    }

    return -1;
}


/************************************************
 *
 ************************************************/
quint32 ReaderData::readUInt(quint64 *pos, bool *ok) const
{
    char *end;
    quint32 res = strtoul(mData + *pos, &end, 10);
    *ok = end != mData + *pos;
    *pos = end - mData;
    return res;
}


/************************************************
 *
 ************************************************/
double ReaderData::readNum(quint64 *pos, bool *ok) const
{
    const char * str = mData + *pos;
    int sign = 1;
    if (str[0] == '-')
    {
        ++str;
        sign = -1;
    }

    char *end;
    double res = strtoll(str, &end, 10);
    if (end[0] == '.')
    {
        ++end;
        if (isdigit(end[0]))
        {
            str = end;
            long long fract = strtoll(str, &end, 10);
            if (str < end)
                res += fract / pow(10.0, end - str);
        }
    }
    *ok = end != mData + *pos;
    *pos = end - mData;
    return res * sign;
}


/************************************************
 *
 ************************************************/
QString ReaderData::readNameString(quint64 *pos) const
{
    if (mData[*pos] != '/')
        throw ReaderError("Invalid PDF name, starting marker '/' was not found", *pos);

    quint64 start = *pos;
    for (++(*pos); *pos < mSize; ++(*pos))
    {
        if (isDelim(*pos))
        {
            return QString::fromLocal8Bit(mData + start + 1, *pos - start - 1);
        }
    }

    throw ReaderError("Invalid PDF name on pos", start);
}


/************************************************
 * Strings may be written in hexadecimal form, which is useful for
 * including arbitrary binary data in a PDF file. A hexadecimal
 * string is written as a sequence of hexadecimal digits (0–9 and
 * either A –F or a–f ) enclosed within angle brackets (< and >):
 *    < 4E6F762073686D6F7A206B6120706F702E >
 *
 * Each pair of hexadecimal digits defines one byte of the string.
 * White-space characters (such as space, tab, carriage return,
 * line feed, and form feed) are ignored.
 *
 * If the final digit of a hexadecimal string is missing—that is,
 * if there is an odd number of digits—the final digit is assumed
 * to be 0. For example:
 *     < 901FA3 >
 * is a 3-byte string consisting of the characters whose hexadecimal
 * codes are 90, 1F, and A3, but
 *    < 901FA >
 * is a 3-byte string containing the characters whose hexadecimal
 * codes are 90, 1F , and A0.
 ************************************************/
qint64 ReaderData::readHexString(quint64 start, String *res) const
{
    QByteArray string;
    string.reserve(1024);

    bool first = true;
    char r = 0;
    for (quint64 pos = start+1; pos < mSize; ++pos)
    {
        char c = mData[pos];
        switch (c)
        {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        {
            if (first)
                r = c - '0';
            else
                string.append(r * 16 + c -'0');

            first = !first;
            break;
        }

        case 'A': case 'B': case 'C':
        case 'D': case 'E': case 'F':
        {
            if (first)
                r = c - 'A' + 10;
            else
                string.append(r * 16 + c -'A' + 10);

            first = !first;
            break;
        }

        case 'a': case 'b': case 'c':
        case 'd': case 'e': case 'f':
        {
            if (first)
                r = c - 'a' + 10;
            else
                string.append(r * 16 + c -'a' + 10);

            first = !first;
            break;
        }

        case ' ':  case '\t': case '\n':
        case '\v': case '\f': case '\r':
            break;

        case '>':
        {
            if (!first)
                string.append(r * 16);

            res->setValue(QTextCodec::codecForUtfText(string, mTextCodec)->toUnicode(string));
            res->setEncodingType(String::HexEncoded);
            return pos + 1;
        }

        default:
            throw ReaderError("Invalid PDF hexadecimal string.", pos);
        }
    }

    throw ReaderError("The closing hexadecimal string marker '>' was not found.", start);
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
qint64 ReaderData::readLiteralString(qint64 start, String *res) const
{
    QByteArray data;
    data.reserve(1024);

    int level = 1;
    bool esc = false;
    for (quint64 i = start+1; i < mSize; ++i)
    {
        char c = mData[i];

        switch (c)
        {

        // Backslash .......................
        case '\\':
            esc = !esc;
            if (!esc)
                data.append(c);
            break;

        // Line feed (LF) ..................
        case 'n':
            data.append(esc ? '\n' : 'n');
            esc = false;
            break;

        // Carriage return (CR) ............
        case 'r':
            data.append(esc ? '\r' : 'r');
            esc = false;
            break;

        // Horizontal tab (HT) .............
        case 't':
            data.append(esc ? '\t' : 't');
            esc = false;
            break;

        // Backspace (BS) ..................
        case 'b':
            data.append(esc ? '\b' : 'b');
            esc = false;
            break;

        // Form feed (FF) ..................
        case 'f':
            data.append(esc ? '\f' : 'f');
            esc = false;
            break;

        // Character code ddd (octal) ......
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
            if (esc)
            {
                esc = false;
                char n = c-'0';
                uint l = qMin(i+3, mSize);
                for(++i; i<l; ++i)
                {
                    c = mData[i];
                    if (c < '0' || c > '7' )
                        break;

                    n = n * 8 + c - '0';
                }
                data.append(n);
                --i;
            }
            else
            {
                data.append(c);
            }
            break;

        case '\n':
            if (esc)
            {
                if (i+1<mSize && mData[i+1] == '\r')
                    ++i;
            }
            else
            {
                data.append('\n');
            }
            esc = false;
            break;

        case '\r':
            if (esc)
            {
                if (i+1<mSize && mData[i+1] == '\n')
                    ++i;
            }
            else
            {
                data.append('\r');
            }
            esc = false;
            break;

        case '(':
            if (!esc)
                ++level;
            data.append(c);
            esc = false;
            break;

        case ')':
            if (!esc)
            {
                --level;

                if (level == 0)
                {
                    res->setValue(QTextCodec::codecForUtfText(data, mTextCodec)->toUnicode(data));
                    res->setEncodingType(String::LiteralEncoded);
                    return i + 1;
                }
            }
            esc = false;
            data.append(c);
            break;

        default:
            esc = false;
            data.append(c);
            break;
        }
    }

    throw ReaderError("The closing literal string marker ')' was not found.", start);
}


/************************************************
 *
 ************************************************/
qint64 ReaderData::readArray(quint64 start, Array *res) const
{
    quint64 pos = skipSpace(start + 1);

    while (pos < mSize)
    {
        if (mData[pos] == ']')
        {
            res->setValid(true);
            return pos + 1;
        }

        res->append(readValue(&pos));
        pos = skipSpace(pos);
    }

    throw ReaderError("The closing array marker ']' was not found.", start);

}


/************************************************
 *
 ************************************************/
qint64 ReaderData::readDict(quint64 start, Dict *res) const
{
    quint64 pos = skipSpace(start + 2);  // skip "<<" mark

    while (pos < mSize - 1)
    {
        if (mData[pos]     == '>' &&
            mData[pos + 1] == '>' )
        {
            res->setValid(true);
            return pos += 2;        // skip ">>" mark
        }

        QString name = readNameString(&pos);
        pos = skipSpace(pos);
        res->insert(name, readValue(&pos));

        pos = skipSpace(pos);
    }

    throw ReaderError("The closing dictionary marker '>>' was not found.", start);
}


/************************************************
 *
 ************************************************/
Value ReaderData::readValue(quint64 *pos) const
{
    char c = mData[*pos];
    switch (c) {
    // Link or Number .................
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {
        bool ok;
        double n1 = readNum(pos, &ok);
        if (!ok)
            throw ReaderError(QString("Unexpected symbol '%1', expected a number.").arg(mData[*pos]), *pos);

        if (n1 != quint64(n1))
            return Number(n1);

        quint64 p = *pos;
        p = skipSpace(p);

        quint64 n2 = readUInt(&p, &ok);
        if (!ok)
            return Number(n1);

        p = skipSpace(p);

        if (mData[p] != 'R')
            return Number(n1);

        *pos = p+1;
        return Link(n1, n2);
    }
    // Float number ...................
    case '-':
    case '+':
    case '.':
    {
        bool ok;
        double n = readNum(pos, &ok);
        if (!ok)
            throw ReaderError(QString("Unexpected symbol '%1', expected a number.").arg(mData[*pos]), *pos);

        return Number(n);
    }


    // Array ..........................
    case '[':
    {
        Array res;
        *pos = readArray(*pos, &res);
        return res;

    }

    // Dict or HexString ..............
    case '<':
    {
        if (mData[*pos+1] == '<')
        {
            Dict res;
            *pos = readDict(*pos, &res);
            return res;
        }
        else
        {
            String res;
            *pos = readHexString(*pos, &res);
            return res;
        }
    }

    // Name ...........................
    case '/':
    {
        return Name(readNameString(pos));
    }

    //LiteralString ...................
    case '(':
    {
        String res;
        *pos = readLiteralString(*pos, &res);
        return res;
    }

    // Bool ...........................
    case 't':
    case 'f':
    {
        if (compareWord(*pos, "true"))
        {
            *pos += 4;
            return Bool(true);
        }

        if (compareWord(*pos, "false"))
        {
            *pos += 5;
            return Bool(false);
        }

        throw ReaderError(QString("Unexpected symbol '%1', expected a boolean.").arg(mData[*pos]), *pos);
    }

    // None ...........................
    case 'n':
    {
        if (!compareWord(*pos, "null"))
            throw ReaderError("Invalid PDF null", *pos);

        *pos += 4;
        return Null();
    }

    // Comment ........................
    case '%':
    {
        while (*pos < mSize && mData[*pos] != '\n' && mData[*pos] != '\r')
            ++(*pos);

        *pos = skipSpace(*pos);
        return readValue(pos);
    }
    }

    QByteArray d(mData + *pos, qMin(mSize - *pos, 20ull));
    throw ReaderError(QString("Unknown object type '%1'").arg(d.data()), *pos);
}



/************************************************
 *
 ************************************************/
Reader::Reader():
    mFile(nullptr),
    mData(nullptr),
    mSize(0),
    mPagesCount(-1),
    mTextCodec(QTextCodec::codecForName("UTF-8")),
    mCache(new Cache())
{

}


/************************************************
 *
 ************************************************/
Reader::~Reader()
{
    close();

    delete mFile;
    delete mCache;
}


/************************************************
 *
 ************************************************/
Value Reader::readValue(quint64 *pos) const
{
    return ReaderData(mData, mSize, mTextCodec).readValue(pos);
}


/************************************************
 *
 ************************************************/
qint64 Reader::readObject(quint64 start, Object *res) const
{
    ReaderData data(mData, mSize, mTextCodec);
    quint64 pos = start;

    bool ok;
    res->setObjNum(data.readUInt(&pos, &ok));
    if (!ok)
        throw ReaderError("Can't read objNum", pos);

    pos = data.skipSpace(pos);
    res->setGenNum(data.readUInt(&pos, &ok));
    if (!ok)
        throw ReaderError("Can't read genNum", pos);

    pos = data.indexOf("obj", pos) + 3;
    pos = data.skipSpace(pos);

    res->setValue(data.readValue(&pos));
    pos = data.skipSpace(pos);

    if (data.compareWord(pos, "stream"))
    {
        pos = data.skipCRLF(pos + strlen("stream"));

        qint64 len = 0;
        Value v = res->dict().value("Length");
        switch (v.type()) {
        case Value::Type::Number:
            len = v.asNumber().value();
            break;

        case Value::Type::Link:
            len = getObject(v.asLink().objNum(), v.asLink().genNum()).value().asNumber().value();
            break;

        default:
            throw ReaderError(QString("Incorrect stream length in object at %1.").arg(data.mData[start]), pos);
        }

        res->setStream(QByteArray::fromRawData(data.mData + pos, len));
        pos = data.skipSpace(pos + len);
        if (data.compareWord(pos, "endstream"))
            pos += strlen("endstream");
    }

    pos = data.skipSpace(pos);
    pos += strlen("endobj");
    res->mPos = start;
    res->mLen = pos - start;
    return pos;
}


/************************************************
 * Additional entries specific to an object stream dictionary
 *
 * KEY  TYPE DESCRIPTION
 * Type name    (Required) The type of PDF object that this dictionary
 *              describes; must be ObjStm for an object stream.
 *
 * N    int     (Required) The number of compressed objects in the stream.
 *
 * First int    (Required) The byte offset (in the decoded stream)
 *              of the first compressed object.
 *
 * Extends 	stream  (Optional) A reference to an object stream, of which
 *                  the current object stream is considered an extension.
 *                  Both streams are considered part of a collection of object
 *                  streams (see below). A given collection consists of a set
 *                  of streams whose Extends links form a directed acyclic graph.
 *
 ************************************************/
void Reader::readObjectFromStream(ObjNum objNum, Object *res, ObjNum streamObjNum, GenNum streamGenNum, quint32 stremIndex) const
{
    Q_UNUSED(stremIndex)

    const Object &streamObj = getObject(streamObjNum, streamGenNum);

    QByteArray stream = mCache->getStream(streamObjNum, streamGenNum);
    if (stream.isEmpty())
    {
        stream = streamObj.decodedStream();
        mCache->setStream(streamObjNum, streamGenNum, stream);
    }

    ReaderData data(stream.data(), stream.size(), mTextCodec);

    // The number of compressed objects in the stream.
    uint cnt = streamObj.dict().value("N").asNumber().value();

    quint32 offset = 0;
    bool found = false;

    quint64 pos = 0;
    for (uint i=0; i<cnt; ++i)
    {
        bool ok;
        ObjNum num = data.readUInt(&pos, &ok);
        pos = data.skipSpace(pos);
        offset = data.readUInt(&pos, &ok);

        if (num == objNum)
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        res->setObjNum(objNum);
        res->setGenNum(0);
        // The byte offset (in the decoded stream) of the first compressed object.
        uint firstOffset = streamObj.dict().value("First").asNumber();
        pos = offset + firstOffset;

        pos = data.skipSpace(pos);
        res->setValue(data.readValue(&pos));
    }
    else
    {
        const Link &extends = streamObj.dict().value("Extends").asLink();
        if (extends.isValid())
        {
            readObjectFromStream(objNum, res, extends.objNum(), extends.genNum(), 0);
        }
    }
}


/************************************************
 *
 ************************************************/
qint64 Reader::readXRefTable(quint64 pos, XRefTable *res, Dict *trailerDict) const
{
    ReaderData data(mData, mSize, mTextCodec);
    pos = data.skipSpace(pos);

    if (!data.compareWord(pos, "xref"))
        throw ReaderError("Incorrect XRef. Expected 'xref'.", pos);

    pos +=4;
    pos = data.skipSpace(pos);

    // read XRef table ..........................
    do {
        bool ok;
        ObjNum startObjNum = data.readUInt(&pos, &ok);
        if (!ok)
            throw ReaderError("Incorrect XRef. Can't read object number of the first object.", pos);

        uint cnt = data.readUInt(&pos, &ok);
        if (!ok)
            throw ReaderError("Incorrect XRef. Can't read number of entries.", pos);
        pos = data.skipSpace(pos);

        for (uint i=0; i<cnt; ++i)
        {
            if (!res->contains(startObjNum + i))
            {
                if (data[pos + 17] == 'n')
                {
                    res->addUsedObject(startObjNum + i,
                                       strtoul(data.mData + pos + 11, nullptr, 10),
                                       strtoull(data.mData + pos,     nullptr, 10));
                }
                else
                {
                    res->addFreeObject(startObjNum + i,
                                       strtoul(mData + pos + 11, nullptr, 10),
                                       strtoul(mData + pos,      nullptr, 10));
                }
            }

            pos += 20;
        }

        pos = data.skipSpace(pos);
    } while (!data.compareStr(pos, "t"));

    if (!data.compareWord(pos, "trailer"))
        throw ReaderError("Incorrect XRef. Expected 'trailer'.", pos);

    pos = data.skipSpace(pos+strlen("trailer"));
    return data.readDict(pos, trailerDict);
}


/************************************************
 *
 ************************************************/
qint64 Reader::readXRefStream(qint64 start, XRefTable *xref, Dict *trailerDict) const
{
    Object obj;
    quint64 res = readObject(start, &obj);
    *trailerDict = obj.dict();

    QByteArray ba = obj.decodedStream();

    XRefStreamData data(ba.data(), ba.size(), obj.dict());
    quint64 pos = 0;
    foreach (const XRefStreamData::Section &section, data.sections())
    {
        pos = data.readSection(pos, section, xref);
    }
    return res;
}


/************************************************
 *
 ************************************************/
Object Reader::getObject(const Link &link) const
{
    return getObject(link.objNum(), link.genNum());
}


/************************************************
 *
 ************************************************/
Object Reader::getObject(uint objNum, quint16 genNum) const
{
    Q_UNUSED(genNum)

    XRefTable::const_iterator it = mXRefTable.find(objNum);
    if (it == mXRefTable.end())
        return PDF::Object();

    if (it.value().type() == XRefEntry::Compressed)
    {
        PDF::Object res;
        readObjectFromStream(objNum, &res, it.value().streamObjNum(), 0, it.value().streamIndex());
        return res;
    }
    else if (it.value().pos())
    {
        PDF::Object res;
        readObject(it.value().pos(), &res);
        return res;
    }

    return PDF::Object();
}


/************************************************
 *
 ************************************************/
Object Reader::getObject(const XRefEntry &xrefEntry) const
{
    return getObject(xrefEntry.objNum(), xrefEntry.genNum());
}


/************************************************
 *
 ************************************************/
const Value Reader::find(const QString &path) const
{
    QStringList objects = path.split('/', QString::SkipEmptyParts);
    if (objects.first() == "Trailer")
        objects.removeFirst();
    QString val = objects.takeLast();

    Dict dict = trailerDict();
    foreach (const QString &obj, objects)
    {
        dict = getObject(dict.value(obj).asLink()).dict();
    }
    return dict.value(val);
}


/************************************************
 *
 ************************************************/
quint32 Reader::pageCount()
{
    if (mPagesCount < 0)
        mPagesCount = find("/Root/Pages/Count").asNumber().value();

    return mPagesCount;
}


/************************************************
 *
 ************************************************/
QByteArray Reader::rawData(quint64 pos, quint64 len) const
{
    return QByteArray::fromRawData(mData + pos, len);
}


/************************************************
 *
 ************************************************/
void Reader::open(const QString &fileName, quint64 startPos, quint64 endPos)
{
    mFile = new QFile(fileName);

    if(!mFile->open(QFile::ReadOnly))
        throw Error(QString("I can't open file \"%1\":%2").arg(fileName).arg(mFile->errorString()));

    int start = startPos;
    int end   = endPos ? endPos : mFile->size();

    if (end < start)
        throw Error(QString("Invalid request for %1, the start position (%2) is greater than the end (%3) one.")
            .arg(fileName)
            .arg(startPos)
            .arg(endPos));

    mFile->seek(start);
    mSize  =  end - start;
    mData  = reinterpret_cast<const char*>(mFile->map(start, mSize));
    load();
}


/************************************************
 *
 ************************************************/
void Reader::open(const char * const data, quint64 size)
{
    mData = data;
    mSize = size;
    load();
}


/************************************************
 *
 ************************************************/
void Reader::close()
{
    mCache->clear();

    if (mFile && mData)
        mFile->unmap(const_cast<uchar*>(reinterpret_cast<const uchar*>(mData)));
    mData = nullptr;
    mSize = 0;
}


/************************************************
 *
 ************************************************/
void Reader::load()
{
    mXRefTable.clear();
    mTrailerDict.clear();
    ReaderData data(mData, mSize, mTextCodec);

    // Check header ...................................
    if (!data.compareStr(0, "%PDF-"))
        throw ReaderError("Incorrect header, the marker '%PDF-' was not found.", 0);

    bool ok;
    // Get xref table position ..................
    qint64 startXRef = data.indexOfBack("startxref", mSize - 1);
    if (startXRef < 0)
        throw ReaderError("Incorrect trailer, the marker 'startxref' was not found.", mSize);

    quint64 pos = startXRef + strlen("startxref");
    quint64 xrefPos = data.readUInt(&pos, &ok);
    if (!ok)
        throw ReaderError("Error in trailer, can't read xref position.", pos);

    // Read xref table ..........................
    xrefPos = data.skipSpace(xrefPos);
    if (data[xrefPos] == 'x')
        readXRefTable(xrefPos, &mXRefTable, &mTrailerDict);

    else if (data[xrefPos] >= '0' && data[xrefPos] <= '9')
        readXRefStream(xrefPos, &mXRefTable, &mTrailerDict);

    else
        throw ReaderError("Error in trailer, unknown xref type.", xrefPos);


    qint64 parentXrefPos = mTrailerDict.value("Prev").asNumber().value();
    while (parentXrefPos)
    {
        Dict dict;
        parentXrefPos = data.skipSpace(parentXrefPos);
        if (data[parentXrefPos] == 'x')
            readXRefTable(parentXrefPos, &mXRefTable, &dict);

        else if (data[parentXrefPos] >= '0' && data[parentXrefPos] <= '9')
            readXRefStream(parentXrefPos, &mXRefTable, &dict);

        else
            throw ReaderError("Error in trailer, unknown xref type.", parentXrefPos);

        parentXrefPos = dict.value("Prev").asNumber().value();
    }

    assert(mTrailerDict.value("Root").isLink());
    assert(mTrailerDict.value("Size").isNumber());
}
