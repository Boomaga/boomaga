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


#include "math.h"
#include "pdfreader.h"
#include "pdfxref.h"
#include "pdfobject.h"
#include "pdftypes.h"
#include "pdfvalue.h"
#include <QDebug>

using namespace PDF;


/************************************************
 *
 ************************************************/
Reader::Reader(const char * const data, quint64 size):
    mData(data),
    mSize(size)
{
}


/************************************************
 *
 ************************************************/
Reader::~Reader()
{

}


/************************************************
 *
 ************************************************/
Value Reader::readValue(qint64 *pos) const
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
            throw ParseError(*pos, QString("Unexpected symbol '%1', expected a number.").arg(mData[*pos]));

        if (n1 != quint64(n1))
            return Number(n1);

        qint64 p = *pos;
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
            throw ParseError(*pos, QString("Unexpected symbol '%1', expected a number.").arg(mData[*pos]));

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
            HexString res;
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
        LiteralString res;
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
            return Bool(true);
        }

        throw ParseError(*pos, QString("Unexpected symbol '%1', expected a boolean.").arg(mData[*pos]));
    }

    // None ...........................
    case 'n':
    {
        if (!compareWord(*pos, "null"))
            throw ParseError(*pos, QString("Invalid PDF null on pos %1").arg(*pos));

        *pos += 4;
        return Null();
    }

    }

    QByteArray d(mData + *pos, qMin(mSize - *pos, 20ll));
    throw UnknownValueError(*pos, QString("Unknown object type on %1: '%2'").arg(*pos).arg(d.data()));
}


/************************************************
 *
 ************************************************/
qint64 Reader::readArray(qint64 start, Array *res) const
{
    qint64 pos = start + 1;

    while (pos < mSize)
    {
        pos = skipSpace(pos);

        if (pos == mSize)
            throw ParseError(start);

        if (mData[pos] == ']')
        {
            res->setValid(true);
            return pos + 1;
        }

        res->append(readValue(&pos));
    }

    throw ParseError(start, "The closing array marker ']' was not found.");
}


/************************************************
 *
 ************************************************/
qint64 Reader::readDict(qint64 start, Dict *res) const
{
    qint64 pos = start + 2;         // skip "<<" mark

    while (pos < mSize - 1)
    {
        pos = skipSpace(pos);

        if (mData[pos]     == '>' &&
            mData[pos + 1] == '>' )
        {
            res->setValid(true);
            return pos += 2;        // skip ">>" mark
        }

        QString name = readNameString(&pos);
        pos = skipSpace(pos);
        res->insert(name, readValue(&pos));
    }

    throw ParseError(start, "The closing dictionary marker '>>' was not found.");
}


/************************************************
 *
 ************************************************/
qint64 Reader::readHexString(qint64 start, HexString *res) const
{
    for (qint64 pos = start+1; pos < mSize; ++pos)
    {
        if (isxdigit(mData[pos]) || isspace(mData[pos]))
            continue;

        if (mData[pos] == '>')
        {
            res->setValid(true);
            res->setValue(QByteArray(mData + start + 1, pos - start - 1));
            return pos + 1;
        }

        throw ParseError(pos, QString("Invalid PDF hexadecimal string on pos %1").arg(pos));
    }

    throw ParseError(start, "The closing hexadecimal string marker '>' was not found.");
}


/************************************************
 *
 ************************************************/
qint64 Reader::readLiteralString(qint64 start, LiteralString *res) const
{
    bool esc = false;
    for (qint64 pos = start+1; pos < mSize; ++pos)
    {
        if (mData[pos] == '\\')
        {
            esc = !esc;
        }
        else if (mData[pos] == ')' && !esc)
        {
            res->setValid(true);
            res->setValue(QByteArray(mData + start + 1, pos - start - 1));
            return pos + 1;
        }
        else
        {
            esc = false;
        }
    }

    throw ParseError(start, "The closing literal string marker ')' was not found.");
}


/************************************************
 *
 ************************************************/
qint64 Reader::readObject(qint64 start, Object *res) const
{
    qint64 pos = start;

    bool ok;
    res->setObjNum(readUInt(&pos, &ok));
    if (!ok)
        throw ParseError(pos);

    pos = skipSpace(pos);
    res->setGenNum(readUInt(&pos, &ok));
    if (!ok)
        throw ParseError(pos);

    pos = indexOf("obj", pos) + 3;
    pos = skipSpace(pos);

    res->setValue(readValue(&pos));
    pos = skipSpace(pos);

    if (compareWord(pos, "stream"))
    {
        pos = skipCRLF(pos + strlen("stream"));

        qint64 len = 0;
        Value v = res->dict().value("Length");
        switch (v.type()) {
        case Value::Type::Number:
            len = v.toNumber().value();
            break;

        case Value::Type::Link:
            len = getObject(v.toLink().objNum(), v.toLink().genNum()).value().toNumber().value();
            break;

        default:
            throw ParseError(pos, QString("Incorrect stream length in object at %1.").arg(mData[start]));
        }

        res->setStream(QByteArray::fromRawData(mData + pos, len));
        pos = skipSpace(pos + len);
        if (compareWord(pos, "endstream"))
            pos += strlen("endstream");
    }

    return pos;
}


/************************************************
 *
 ************************************************/
qint64 Reader::readXRefTable(qint64 pos, XRefTable *res) const
{
    pos = skipSpace(pos);
    if (!compareWord(pos, "xref"))
        throw ParseError(pos, "Incorrect XRef. Expected 'xref'.");
    pos +=4;

    pos = skipSpace(pos);

    // read XRef table ..........................
    do {
        bool ok;
        uint startObjNum = readUInt(&pos, &ok);
        if (!ok)
            throw ParseError(pos, "Incorrect XRef. Can't read object number of the first object.");

        uint cnt = readUInt(&pos, &ok);
        if (!ok)
            throw ParseError(pos, "Incorrect XRef. Can't read number of entries.");
        pos = skipSpace(pos);

        for (uint i=0; i<cnt; ++i)
        {
            if (!res->contains(startObjNum + i))
            {
                if (mData[pos + 17] == 'n')
                {
                    res->insert(startObjNum + i,
                                XRefEntry(
                                    strtoull(mData + pos,     nullptr, 10),
                                    startObjNum + i,
                                    strtoul(mData + pos + 11, nullptr, 10),
                                    XRefEntry::Used));
                }
                else
                {
                    res->insert(startObjNum + i,
                                XRefEntry(
                                    0,
                                    startObjNum + i,
                                    strtoul(mData + pos + 11, nullptr, 10),
                                    XRefEntry::Free));

                }
            }

            pos += 20;
        }

        pos = skipSpace(pos);
    } while (!compareStr(pos, "trailer"));

    return pos;
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
    Object obj;
    if (mXRefTable.value(objNum).pos)
        readObject(mXRefTable.value(objNum).pos, &obj);
    return obj;
}


/************************************************
 *
 ************************************************/
const Value Reader::find(const QString &path) const
{
    QStringList objects = path.split('/', QString::SkipEmptyParts);
    QString val = objects.takeLast();

    Dict dict = trailerDict();
    foreach (const QString &obj, objects)
    {
        dict = getObject(dict.value(obj).toLink()).dict();
    }
    return dict.value(val);
}


/************************************************
 *
 ************************************************/
QString Reader::readNameString(qint64 *pos) const
{
    if (mData[*pos] != '/')
        throw ParseError(*pos, QString("Invalid PDF name on pos %1").arg(*pos));

    qint64 start = *pos;
    for (++(*pos); *pos < mSize; ++(*pos))
    {
        if (isDelim(*pos))
        {
            return QString::fromLocal8Bit(mData + start + 1, *pos - start - 1);
        }
    }

    throw ParseError(start);
}


/************************************************
 *
 ************************************************/
void Reader::open()
{
    // Check header ...................................
    if (indexOf("%PDF-") != 0)
        throw HeaderError(0);

    bool ok;
    // Get xref table position ..................
    qint64 startXRef = indexOfBack("startxref", mSize - 1);
    if (startXRef < 0)
        throw ParseError(0, "Incorrect trailer, the marker 'startxref' was not found.");

    qint64 pos = startXRef + strlen("startxref");
    quint64 xrefPos = readUInt(&pos, &ok);
    if (!ok)
        throw ParseError(pos, "Error in trailer, can't read xref position.");

    // Read xref table ..........................
    pos = readXRefTable(xrefPos, &mXRefTable);
    pos = skipSpace(pos+strlen("trailer"));
    readDict(pos, &mTrailerDict);

    qint64 parentXrefPos = mTrailerDict.value("Prev").toNumber().value();

    while (parentXrefPos)
    {
        pos = readXRefTable(parentXrefPos, &mXRefTable);
        pos = skipSpace(pos+strlen("trailer"));
        Dict dict;
        readDict(pos, &dict);
        parentXrefPos = dict.value("Prev").toNumber().value();
    }
}

/************************************************
 *
 ************************************************/
void Reader::load()
{
    foreach (const XRefEntry &entry, mXRefTable)
    {
        if (entry.type == XRefEntry::Used)
        {
            Object obj = Object();
            readObject(entry.pos, &obj);
        }
    }
}


/************************************************
 *
 ************************************************/
bool Reader::isDelim(qint64 pos) const
{
    return isspace(mData[pos]) ||
            strchr("()<>[]{}/%", mData[pos]);
}


/************************************************
 *
 ************************************************/
qint64 Reader::skipSpace(qint64 pos) const
{
    while (pos < mSize && isspace(mData[pos]))
        pos++;

    return pos;
}


/************************************************
 *
 ************************************************/
qint64 Reader::indexOf(const char *str, qint64 from) const
{
    qint64 len = strlen(str);

    for (qint64 i = from; i<mSize-len; i++)
    {
        if (strncmp(mData + i, str, len) == 0)
            return i;
    }

    return -1;
}


/************************************************
 *
 ************************************************/
qint64 Reader::indexOfBack(const char *str, qint64 from) const
{
    qint64 len = strlen(str);

    for (qint64 i = from - len + 1 ; i>0; i--)
    {
        if (strncmp(mData + i, str, len) == 0)
            return i;
    }

    return -1;
}


/************************************************
 *
 ************************************************/
qint64 Reader::skipCRLF(qint64 pos) const
{
    for (; pos >= 0; ++pos)
    {
        if (mData[pos] != '\n' && mData[pos] != '\r')
            return pos;
    }

    return 0;
}


/************************************************
 *
 ************************************************/
quint32 Reader::readUInt(qint64 *pos, bool *ok) const
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
double Reader::readNum(qint64 *pos, bool *ok) const
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
bool Reader::compareStr(qint64 pos, const char *str) const
{
    int len = strlen(str);
    return (mSize - pos > len) && strncmp(mData + pos, str, len) == 0;
}


/************************************************
 *
 ************************************************/
bool Reader::compareWord(qint64 pos, const char *str) const
{
    int len = strlen(str);
    return (mSize - pos > len + 1) &&
            strncmp(mData + pos, str, len) == 0 &&
            isDelim(pos + len);
}
