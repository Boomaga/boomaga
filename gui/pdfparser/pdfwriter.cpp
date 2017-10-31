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

#include <QCryptographicHash>
#include <QDebug>

using namespace PDF;


/************************************************
 *
 ************************************************/
Writer::Writer():
    mDevice(nullptr),
    mXRefPos(0)
{
    mXRefTable.insert(0, XRefEntry(0, 0, 65535, XRefEntry::Free));
}


/************************************************
 *
 ************************************************/
Writer::Writer(QIODevice *device):
    mDevice(device),
    mXRefPos(0)
{
    mXRefTable.insert(0, XRefEntry(0, 0, 65535, XRefEntry::Free));
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
QIODevice &operator<<(QIODevice &out, const char *value)
{
    out.write(value);
    return out;
}


/************************************************
 *
 ************************************************/
QIODevice &operator<<(QIODevice &out, const QString &value)
{
    out.write(value.toLatin1());
    return out;
}


/************************************************
 *
 ************************************************/
QIODevice &operator<<(QIODevice &out, const double value)
{
    out.write(QString::number(qint64(value)).toLatin1());
    if (value - qint64(value) > 0)
    {
        out.write(".");
        out.write(QString::number(value-qint64(value)).toLatin1().data() + 2);
    }

    return out;
}


/************************************************
 *
 ************************************************/
QIODevice &operator<<(QIODevice &out, const quint64 value)
{
    const int BUF_SIZE(64);
    char buf[BUF_SIZE+1];
    buf[BUF_SIZE]=0;
    int i(BUF_SIZE);

    quint64 n = value;
    do
    {
        buf[--i] = (n % 10 ) + '0';
        n /= 10;
    } while(n);

    out.write(buf + i, BUF_SIZE - i);
    return out;
}


/************************************************
 *
 ************************************************/
QIODevice &operator<<(QIODevice &out, const quint32 value)
{
    return operator<<(out, quint64(value));
}


/************************************************
 *
 ************************************************/
QIODevice &operator<<(QIODevice &out, const quint16 value)
{
    return operator<<(out, quint64(value));
}


/************************************************
 *
 ************************************************/
QIODevice &operator<<(QIODevice &out, const qint64 value)
{
    const int BUF_SIZE(64);
    char buf[BUF_SIZE+1];
    buf[BUF_SIZE]=0;
    int i(BUF_SIZE);
    quint64 n(value);
    bool neg = false;
    if (value < 0)
    {
        neg = true;
        n= -n;
    }

    do
    {
        buf[--i] = (n % 10 ) + '0';
        n /= 10;
    } while(n);

    if (neg)
        buf[--i] = '-';

    out.write(buf + i, BUF_SIZE - i);
    return out;
}


/************************************************
 *
 ************************************************/
QIODevice &operator<<(QIODevice &out, const qint16 value)
{
    return operator<<(out, qint64(value));
}


/************************************************
 *
 ************************************************/
QIODevice &operator<<(QIODevice &out, const qint32 value)
{
    return operator<<(out, qint64(value));
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
        * mDevice << "[";
        foreach (const Value v, value.asArray().values())
        {
            writeValue(v);
            *mDevice << " ";
        }
        *mDevice << "]";
        break;
    }


    //.....................................................
    case Value::Type::Bool:
    {
        if (value.asBool().value())
            *mDevice << "true";
        else
            *mDevice << "false";

        break;
    }


    //.....................................................
    case Value::Type::Dict:
    {
        const QMap<QString, Value> &values = value.asDict().values();
        *mDevice << "<<\n";
        QMap<QString, Value>::const_iterator i;
        for (i = values.constBegin(); i != values.constEnd(); ++i)
        {
            *mDevice << "/";
            *mDevice << i.key();
            *mDevice << " ";
            writeValue(i.value());
            *mDevice << "\n";
        }
        *mDevice << ">>";
        break;
    }

    //.....................................................
    case Value::Type::HexString:
        mDevice->write("<");
        mDevice->write(value.asHexString().value());
        mDevice->write(">");
        break;


    //.....................................................
    case Value::Type::Link:
    {
        const Link link = value.asLink();
        *mDevice << link.objNum();
        *mDevice << " ";
        *mDevice << link.genNum();
        *mDevice << " R";
        break;
    }


    //.....................................................
    case Value::Type::LiteralString:
        mDevice->write("(");
        mDevice->write(value.asHexString().value());
        mDevice->write(")");
        break;


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
        *mDevice << value.asNumber().value();
        break;
    }


    //.....................................................
    default:
        throw UnknownValueError(mDevice->pos(), QString("Unknown object type: '%1'").arg(int(value.type())));
    }
}


/************************************************
 *
 ************************************************/
void Writer::writePDFHeader(int majorVersion, int minorVersion)
{
    mDevice->write(QString("%PDF-%1.%2\n").arg(majorVersion).arg(minorVersion).toLatin1());
    //is recommended that the header line be immediately followed by a comment line containing
    // at least four binary characters—that is, characters whose codes are 128 or greater.
    // PDF Reference, 3.4.1 File Header
    mDevice->write("%\xE2\xE3\xCF\xD3\n");
}


/************************************************
 *
 ************************************************/
void Writer::writeXrefTable()
{
    mXRefPos = mDevice->pos();
    mDevice->write("xref\n");
    auto start = mXRefTable.constBegin();

    while (start != mXRefTable.constEnd())
    {
        auto it(start);
        PDF::ObjNum prev = start.value().objNum;
        for (; it != mXRefTable.constEnd(); ++it)
        {
            if (it.value().objNum - prev > 1)
                break;

            prev = it.value().objNum;
        }

        writeXrefSection(start, prev - start.value().objNum + 1);
        start = it;
    }
}


/************************************************
 *
 ************************************************/
void Writer::writeXrefSection(const XRefTable::const_iterator &start, quint32 count)
{
    *mDevice << start.value().objNum;
    *mDevice << " ";
    *mDevice << count;
    *mDevice << "\n";

    const uint bufSize = 1024 * 20;
    char buf[bufSize];

    XRefTable::const_iterator it = start;
    for (quint32 i = 0; i<count;)
    {
        uint pos = 0;
        for (; pos < bufSize && i<count; ++i)
        {
            const XRefEntry &entry = it.value();
            buf[pos +  0] = (entry.pos / 1000000000) % 10 + '0';
            buf[pos +  1] = (entry.pos / 100000000) % 10 + '0';
            buf[pos +  2] = (entry.pos / 10000000) % 10 + '0';
            buf[pos +  3] = (entry.pos / 1000000) % 10 + '0';
            buf[pos +  4] = (entry.pos / 100000) % 10 + '0';
            buf[pos +  5] = (entry.pos / 10000) % 10 + '0';
            buf[pos +  6] = (entry.pos / 1000) % 10 + '0';
            buf[pos +  7] = (entry.pos / 100) % 10 + '0';
            buf[pos +  8] = (entry.pos / 10) % 10 + '0';
            buf[pos +  9] = (entry.pos / 1) % 10 + '0';

            buf[pos + 10] = ' ';

            buf[pos + 11] = (entry.genNum / 10000) % 10 + '0';
            buf[pos + 12] = (entry.genNum / 1000) % 10 + '0';
            buf[pos + 13] = (entry.genNum / 100) % 10 + '0';
            buf[pos + 14] = (entry.genNum / 10) % 10 + '0';
            buf[pos + 15] = (entry.genNum / 1) % 10 + '0';

            buf[pos + 16] = ' ';
            buf[pos + 17] = 'n';
            buf[pos + 18] = ' ';
            buf[pos + 19] = '\n';

            pos+=20;
            ++it;
        }
        mDevice->write(buf, pos);
    }
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
    QCryptographicHash idHash(QCryptographicHash::Md5);
    foreach (const XRefEntry &xref, mXRefTable)
    {
        idHash.addData(QString("%1 %2 %3").arg(xref.pos).arg(xref.objNum).arg(xref.genNum).toLatin1());
    }

    Array id;
    id.append(HexString(idHash.result()));

    idHash.addData("PDF parser");
    id.append(HexString(HexString(idHash.result())));

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
    *mDevice << "\n%";
    QString s = comment;
    *mDevice << s.replace("\n", "\n%");
    *mDevice << "\n";
}


/************************************************
 *
 ************************************************/
void Writer::writeObject(const Object &object)
{
    *mDevice << "\n";
    mXRefTable.insert(object.objNum(), XRefEntry(mDevice->pos(), object.objNum(), object.genNum(), XRefEntry::Used));

    *mDevice << object.objNum();
    *mDevice << " ";
    *mDevice << object.genNum();
    *mDevice << " obj\n";

    writeValue(object.value());

    if (object.stream().length())
    {
        *mDevice << "\nstream\n";
        mDevice->write(object.stream());
        *mDevice << "\nendstream";
    }

    *mDevice << "\nendobj\n";
}
