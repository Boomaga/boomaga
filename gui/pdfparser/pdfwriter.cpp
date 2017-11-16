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


#include "pdfwriter.h"
#include "pdfobject.h"
#include "pdfvalue.h"
#include "pdfxref.h"
#include <climits>
#include <cmath>

#include <QUuid>
#include <QDebug>

using namespace PDF;


/************************************************
 *
 ************************************************/
Writer::Writer():
    mDevice(nullptr),
    mXRefPos(0),
    mBuf{0}
{
    mXRefTable.addFreeObject(0, 65535, 0);
}


/************************************************
 *
 ************************************************/
Writer::Writer(QIODevice *device):
    mDevice(device),
    mXRefPos(0),
    mBuf{0}
{
    mXRefTable.addFreeObject(0, 65535, 0);
}


/************************************************
 *
 ************************************************/
Writer::~Writer()
{
}


/************************************************
 *
 ************************************************/
void Writer::setDevice(QIODevice *device)
{
    mDevice = device;
}


/************************************************
 *
 ************************************************/
uint sPrintUint(char *s, quint64 value)
{
    uint len = value ? log10(value) + 1 : 1;
    s[len] = '\0';
    int i(len);
    quint64 n = value;
    do
    {
        s[--i] = (n % 10 ) + '0';
        n /= 10;
    } while(n);

    return len;
}


/************************************************
 *
 ************************************************/
uint sPrintInt(char *s, qint64 value)
{
    if (value<0)
    {
        s[0] = '-';
        return sPrintUint(s+1, -value) + 1;
    }

    return sPrintUint(s, value);
}


/************************************************
 *
 ************************************************/
uint sPrintDouble(char *s, double value)
{
    int n = sprintf(s, "%f", value);
    if (n < 0)
        return 0;

    for (--n; n>0 && s[n] == '0'; --n)
        s[n] = 0;

    if (s[n] == '.' || s[n] == ',')
    {
        s[n] = 0;
        return n;
    }

    char *c = strchr(s, ',');
    if (c)
        c[0] = '.';

    s[n+1] = 0;
    return n+1;
}


/************************************************
 *
 ************************************************/
void Writer::writeValue(const Value &value)
{
    switch (value.type())
    {

    //.....................................................
    case Value::Type::Array:
    {
        mDevice->write("[");
        foreach (const Value v, value.asArray().values())
        {
            writeValue(v);
            mDevice->write(" ");
        }
        mDevice->write("]");

        break;
    }


    //.....................................................
    case Value::Type::Bool:
    {
        if (value.asBool().value())
            mDevice->write("true");
        else
            mDevice->write("false");

        break;
    }


    //.....................................................
    case Value::Type::Dict:
    {
        const QMap<QString, Value> &values = value.asDict().values();
        write("<<\n");
        QMap<QString, Value>::const_iterator i;
        for (i = values.constBegin(); i != values.constEnd(); ++i)
        {
            write('/');
            write(i.key());
            write(' ');
            writeValue(i.value());
            write('\n');
        }
        write(">>");

        break;
    }


    //.....................................................
    case Value::Type::Link:
    {
        const Link link = value.asLink();
        write(link.objNum());
        mDevice->write(" ");
        write(link.genNum());
        mDevice->write(" R");

        break;
    }


    //.....................................................
    case Value::Type::Name:
        mDevice->write("/");
        mDevice->write(value.asName().value().toLatin1());
        break;


    //.....................................................
    case Value::Type::Null:
        mDevice->write("null");
        break;


    //.....................................................
    case Value::Type::Number:
    {
        write(value.asNumber().value());
        break;
    }


    //.....................................................
    case Value::Type::String:
    {
        const String &s = value.asString();
        if (s.encodingType() == String::HexEncoded)
        {
            write('<');
            mDevice->write(s.value().toUtf8().toHex());
            write('>');
        }
        else
        {
            mDevice->write("(");
            writeLiteralString(s);
            mDevice->write(")");
        }
        break;
    }

    //.....................................................
    default:
        throw Error(QString("Unknown object type: '%1'").arg(int(value.type())));
    }
}


/************************************************
 *
 ************************************************/
void Writer::writePDFHeader(int majorVersion, int minorVersion)
{
    mDevice->write(QString("%PDF-%1.%2\n").arg(majorVersion).arg(minorVersion).toLatin1());
    //is recommended that the header line be immediately followed by
    // a comment line containing at least four binary characters—that is,
    // characters whose codes are 128 or greater.
    // PDF Reference, 3.4.1 File Header
    mDevice->write("%\xE2\xE3\xCF\xD3\n");
}


/************************************************
 * Each entry is exactly 20 bytes long, including the
 * end-of-line marker. There are two kinds of cross-reference entries:
 * one for objects that are in use and another for objects that have
 * been deleted and therefore are free. Both types of entries have
 * similar basic formats, distinguished by the keyword n
 * (for an in-use entry) or f (for a free entry).
 *
 * The format of an in-use entry is
 *     nnnnnnnnnn ggggg n eol
 * where
 *     nnnnnnnnnn is a 10-digit byte offset
 *     ggggg is a 5-digit generation number
 *     n is a literal keyword identifying this as an in-use entry
 * eol is a 2-character end-of-line sequence
 *
 * The cross-reference entry for a free object has essentially
 * the same format, except that the keyword is f instead of n
 * and the interpretation of the first item is different:
 *     nnnnnnnnnn ggggg f eol
 * where
 *     nnnnnnnnnn is the 10-digit object number of the next free object
 *     ggggg is a 5-digit generation number
 *     f is a literal keyword identifying this as a free entry
 *     eol is a 2-character end-of-line sequence
 *
 * The free entries in the cross-reference table form a linked list,
 * with each free entry containing the object number of the next.
 * The first entry in the table (object number 0) is always free and has
 * a generation number of 65,535; it is the head of the linked list of
 * free objects. The last free entry (the tail of the linked list) links
 * back to object number 0.
 ************************************************/
void Writer::writeXrefTable()
{
    // Restore free entries chain.
    mXRefTable.updateFreeChain();

    mXRefPos = mDevice->pos();
    mDevice->write("xref\n");
    auto start = mXRefTable.constBegin();

    while (start != mXRefTable.constEnd())
    {
        auto it(start);
        PDF::ObjNum prev = start.value().objNum();
        for (; it != mXRefTable.constEnd(); ++it)
        {
            if (it.value().objNum() - prev > 1)
                break;

            prev = it.value().objNum();
        }

        writeXrefSection(start, prev - start.value().objNum() + 1);
        start = it;
    }
}


/************************************************
 *
 ************************************************/
void Writer::writeXrefSection(const XRefTable::const_iterator &start, quint32 count)
{
    write(start.value().objNum());
    write(' ');
    write(count);
    write('\n');

    const uint bufSize = 1024 * 20;
    char buf[bufSize];

    XRefTable::const_iterator it = start;
    for (quint32 i = 0; i<count;)
    {
        uint pos = 0;
        for (; pos < bufSize-20 && i<count; ++i)
        {
            const XRefEntry &entry = it.value();
            buf[pos +  0] = (entry.pos() / 1000000000) % 10 + '0';
            buf[pos +  1] = (entry.pos() / 100000000) % 10 + '0';
            buf[pos +  2] = (entry.pos() / 10000000) % 10 + '0';
            buf[pos +  3] = (entry.pos() / 1000000) % 10 + '0';
            buf[pos +  4] = (entry.pos() / 100000) % 10 + '0';
            buf[pos +  5] = (entry.pos() / 10000) % 10 + '0';
            buf[pos +  6] = (entry.pos() / 1000) % 10 + '0';
            buf[pos +  7] = (entry.pos() / 100) % 10 + '0';
            buf[pos +  8] = (entry.pos() / 10) % 10 + '0';
            buf[pos +  9] = (entry.pos() / 1) % 10 + '0';

            buf[pos + 10] = ' ';

            buf[pos + 11] = (entry.genNum() / 10000) % 10 + '0';
            buf[pos + 12] = (entry.genNum() / 1000) % 10 + '0';
            buf[pos + 13] = (entry.genNum() / 100) % 10 + '0';
            buf[pos + 14] = (entry.genNum() / 10) % 10 + '0';
            buf[pos + 15] = (entry.genNum() / 1) % 10 + '0';

            buf[pos + 16] = ' ';
            buf[pos + 17] = (entry.type() == PDF::XRefEntry::Used ? 'n' : 'f');
            buf[pos + 18] = ' ';
            buf[pos + 19] = '\n';

            pos+=20;
            ++it;
        }
        mDevice->write(buf, pos);
    }
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
void Writer::writeLiteralString(const PDF::String &value)
{
    char oct[5] = "\\000";

    foreach (const char c, value.value().toUtf8())
    {
        switch (c)
        {
        // Line feed (LF) - write as is.
        case '\n':
            mDevice->write("\n");
            continue;

        // Carriage return (CR) - write as is.
        case '\r':
            mDevice->write("\r");
            continue;

        // Horizontal tab (HT) - write as is.
        case '\t':
            mDevice->write("\t");
            continue;

        // Backspace (BS) - write escaped
        case '\b':
            mDevice->write("\\b");
            continue;

        // Form feed (FF) - write escaped
        case '\f':
            mDevice->write("\\f");
            continue;

        // Left parenthesis - write escaped
        case '(':
            mDevice->write("\\(");
            continue;

        // Right parenthesis - write escaped
        case ')':
            mDevice->write("\\)");
            continue;

        // Backslash - write escaped
        case '\\':
            mDevice->write("\\\\");
            continue;
        }

        // ASCII - write as is
        if (uchar(c) >= ' ' && uchar(c) <= '~')
        {
            mDevice->write(&c, 1);
            continue;
        }

        // Write as octal character code (\ddd)
        oct[1] = (uchar(c) / 64) % 8 + '0';
        oct[2] = (uchar(c) /  8) % 8 + '0';
        oct[3] =  uchar(c)       % 8 + '0';
        mDevice->write(oct, 4);
    }
}


/************************************************
 *
 ************************************************/
void Writer::write(const char value)
{
    mDevice->write(&value, 1);
}


/************************************************
 *
 ************************************************/
void Writer::write(const char *value)
{
    mDevice->write(value);
}


/************************************************
 *
 ************************************************/
void Writer::write(const QString &value)
{
    mDevice->write(value.toLocal8Bit());
}


/************************************************
 *
 ************************************************/
void Writer::write(double value)
{
    uint len = sPrintDouble(mBuf, value);
    mDevice->write(mBuf, len);
}


/************************************************
 *
 ************************************************/
void Writer::write(quint64 value)
{
    uint len = sPrintUint(mBuf, value);
    mDevice->write(mBuf, len);
}


/************************************************
 *
 ************************************************/
void Writer::write(quint32 value)
{
    uint len = sPrintUint(mBuf, value);
    mDevice->write(mBuf, len);
}


/************************************************
 *
 ************************************************/
void Writer::write(quint16 value)
{
    uint len = sPrintUint(mBuf, value);
    mDevice->write(mBuf, len);
}


/************************************************
 *
 ************************************************/
void Writer::write(qint64 value)
{
    uint len = sPrintInt(mBuf, value);
    mDevice->write(mBuf, len);
}


/************************************************
 *
 ************************************************/
void Writer::write(qint32 value)
{
    uint len = sPrintInt(mBuf, value);
    mDevice->write(mBuf, len);
}


/************************************************
 *
 ************************************************/
void Writer::write(qint16 value)
{
    uint len = sPrintInt(mBuf, value);
    mDevice->write(mBuf, len);
}


/************************************************
 *
 ************************************************/
void Writer::writeTrailer(const Link &root)
{
    writeTrailer(root, Link());
}


/************************************************
 *
 ************************************************/
void Writer::writeTrailer(const Link &root, const Link &info)
{
    Dict trailerDict;

    // Start - The total number of entries in the file’s cross-reference table,
    // as defined by the combination of the original section and all update sections.
    // Equivalently, this value is 1 greater than the highest object number used in the file.
    trailerDict.insert("Size", mXRefTable.maxObjNum() + 1);

    // Root - (Required; must be an indirect reference) The catalog dictionary for the
    // PDF document contained in the file (see Section 3.6.1, “Document Catalog”).
    trailerDict.insert("Root", root);

    // Info - (Optional; must be an indirect reference) The document’s information
    // dictionary (see Section 10.2.1, “Document Information Dictionary”).
    if (info.objNum())
        trailerDict.insert("Info", info);

    // ID - (Optional, but strongly recommended; PDF 1.1) An array of two byte-strings
    // constituting a file identifier (see Section 10.3, “File Identifiers”) for the file.
    // The two bytestrings should be direct objects and should be unencrypted.
    // Although this entry is optional, its absence might prevent the file from
    // functioning in some workflows that depend on files being uniquely identified.
    String uuid;
    uuid.setEncodingType(PDF::String::HexEncoded);
    uuid.setValue(QUuid::createUuid().toString());

    Array id;
    id.append(uuid);
    id.append(uuid);
    trailerDict.insert("ID", id);

    writeTrailer(trailerDict);
}


/************************************************
 *
 ************************************************/
void Writer::writeTrailer(const Dict &trailerDict)
{
    mDevice->write("\ntrailer\n");
    writeValue(trailerDict);
    mDevice->write(QString("\nstartxref\n%1\n%%EOF\n").arg(mXRefPos).toLatin1());
}


/************************************************
 *
 ************************************************/
void Writer::writeComment(const QString &comment)
{
    write("\n%");
    QString s = comment;
    write(s.replace("\n", "\n%"));
    write('\n');
}


/************************************************
 *
 ************************************************/
void Writer::writeObject(const Object &object)
{
    write('\n');
    mXRefTable.addUsedObject(object.objNum(), object.genNum(), mDevice->pos());

    write(object.objNum());
    write(' ');
    write(object.genNum());
    write(" obj\n");
    writeValue(object.value());

    if (object.stream().length())
    {
        write("\nstream\n");
        mDevice->write(object.stream());
        write("\nendstream");
    }

    write("\nendobj\n");
}
