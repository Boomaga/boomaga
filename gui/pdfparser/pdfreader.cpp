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


#include "pdfreader.h"
#include "pdfxref.h"
#include "pdfdata.h"

#include "pdfobject.h"
#include "pdftypes.h"

#include "pdfvalue.h"
#include <QDebug>

using namespace PDF;

/************************************************
 *
 ************************************************/
inline bool isDelim(const char * const data, qint64 size, qint64 pos)
{
    Q_UNUSED(size)
    return isspace(data[pos]) ||
           strchr("()<>[]{}/%", data[pos]);
}


/************************************************
 *
 ************************************************/
Reader::Reader(const char * const data, quint64 size):
    mData2(data),
    mSize(size),
    mData(new Data(data, size))
{
}


/************************************************
 *
 ************************************************/
Reader::~Reader()
{
    delete mData;
}


/************************************************
 *
 ************************************************/
Value Reader::readValue(qint64 *pos) const
{
    char c = mData2[*pos];
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
        double n1 = mData->readNum(pos, &ok);
        if (!ok)
            throw ParseError(*pos, QString("Unexpected symbol '%1', expected a number.").arg(mData2[*pos]));

        if (n1 != quint64(n1))
            return Number(n1);

        qint64 p = *pos;
        p = mData->skipSpace(p);

        quint64 n2 = mData->readUInt(&p, &ok);
        if (!ok)
            return Number(n1);

        p = mData->skipSpace(p);

        if (mData2[p] != 'R')
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
        double n = mData->readNum(pos, &ok);
        if (!ok)
            throw ParseError(*pos, QString("Unexpected symbol '%1', expected a number.").arg(mData->at(*pos)));

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
        if (mData->at(*pos+1) == '<')
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
        if (mData->compareWord(*pos, "true"))
        {
            *pos += 4;
            return Bool(true);
        }

        if (mData->compareWord(*pos, "false"))
        {
            *pos += 5;
            return Bool(true);
        }

        throw ParseError(*pos, QString("Unexpected symbol '%1', expected a boolean.").arg(mData->at(*pos)));
    }

    // None ...........................
    case 'n':
    {
        if (!mData->compareWord(*pos, "null"))
            throw ParseError(*pos, QString("Invalid PDF null on pos %1").arg(*pos));

        *pos += 4;
        return Null();
    }

    }

    QByteArray d(mData->data() + *pos, qMin(mData->size() - *pos, 20ll));
    throw UnknownValueError(*pos, QString("Unknown object type on %1: '%2'").arg(*pos).arg(d.data()));
}


/************************************************
 *
 ************************************************/
qint64 Reader::readArray(qint64 start, Array *res) const
{
    qint64 pos = start + 1;

    while (pos < mData->size())
    {
        pos = mData->skipSpace(pos);

        if (pos == mData->size())
            throw ParseError(start);

        if (mData->at(pos) == ']')
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

    while (pos < mData->size() - 1)
    {
        pos = mData->skipSpace(pos);

        if (mData->at(pos)     == '>' &&
            mData->at(pos + 1) == '>' )
        {
            res->setValid(true);
            return pos += 2;        // skip ">>" mark
        }

        QString name = readNameString(&pos);
        pos = mData->skipSpace(pos);
        res->insert(name, readValue(&pos));
    }

    throw ParseError(start, "The closing dictionary marker '>>' was not found.");
}


/************************************************
 *
 ************************************************/
qint64 Reader::readHexString(qint64 start, HexString *res) const
{
    for (qint64 pos = start+1; pos < mData->size(); ++pos)
    {
        if (isxdigit(mData->at(pos)) || isspace(mData->at(pos)))
            continue;

        if (mData->at(pos) == '>')
        {
            res->setValid(true);
            res->setValue(QByteArray(mData->data() + start + 1, pos - start - 1));
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
    for (qint64 pos = start+1; pos < mData->size(); ++pos)
    {
        if (mData->at(pos) == '\\')
        {
            esc = !esc;
        }
        else if (mData->at(pos) == ')' && !esc)
        {
            res->setValid(true);
            res->setValue(QByteArray(mData->data() + start + 1, pos - start - 1));
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
    res->setObjNum(mData->readUInt(&pos, &ok));
    if (!ok)
        throw ParseError(pos);

    pos = mData->skipSpace(pos);
    res->setGenNum(mData->readUInt(&pos, &ok));
    if (!ok)
        throw ParseError(pos);

    pos = mData->indexOf("obj", pos) + 3;
    pos = mData->skipSpace(pos);

    res->setValue(readValue(&pos));
    pos = mData->skipSpace(pos);

    if (mData->compareWord(pos, "stream"))
    {
        pos = mData->skipCRLF(pos + strlen("stream"));

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
            throw ParseError(pos, QString("Incorrect stream length in object at %1.").arg(mData->at(start)));
        }

        res->setStream(QByteArray::fromRawData(mData->data() + pos, len));
        pos = mData->skipSpace(pos + len);
        if (mData->compareWord(pos, "endstream"))
            pos += strlen("endstream");
    }

    return pos;
}


/************************************************
 *
 ************************************************/
qint64 Reader::readXRefTable(qint64 pos, XRefTable *res) const
{
    pos = mData->skipSpace(pos);
    if (!mData->compareWord(pos, "xref"))
        throw ParseError(pos, "Incorrect XRef. Expected 'xref'.");
    pos +=4;

    pos = mData->skipSpace(pos);

    // read XRef table ..........................
    const char* data = mData->data();
    do {
        bool ok;
        uint startObjNum = mData->readUInt(&pos, &ok);
        if (!ok)
            throw ParseError(pos, "Incorrect XRef. Can't read object number of the first object.");

        uint cnt = mData->readUInt(&pos, &ok);
        if (!ok)
            throw ParseError(pos, "Incorrect XRef. Can't read number of entries.");
        pos = mData->skipSpace(pos);

        for (uint i=0; i<cnt; ++i)
        {
            if (!res->contains(startObjNum + i))
            {
                if (data[pos + 17] == 'n')
                {
                    res->insert(startObjNum + i,
                                XRefEntry(
                                    strtoull(data + pos,     nullptr, 10),
                                    startObjNum + i,
                                    strtoul(data + pos + 11, nullptr, 10),
                                    XRefEntry::Used));
                }
                else
                {
                    res->insert(startObjNum + i,
                                XRefEntry(
                                    0,
                                    startObjNum + i,
                                    strtoul(data + pos + 11, nullptr, 10),
                                    XRefEntry::Free));

                }
            }

            pos += 20;
        }

        pos = mData->skipSpace(pos);
    } while (!mData->compareStr(pos, "trailer"));

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
    if (mData2[*pos] != '/')
        throw ParseError(*pos, QString("Invalid PDF name on pos %1").arg(*pos));

    qint64 start = *pos;
    for (++(*pos); *pos < mSize; ++(*pos))
    {
        if (isDelim(mData2, mSize, *pos))
        {
            return QString::fromLocal8Bit(mData2 + start + 1, *pos - start - 1);
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
    if (mData->indexOf("%PDF-") != 0)
        throw HeaderError(0);

    bool ok;
    // Get xref table position ..................
    qint64 startXRef = mData->indexOfBack("startxref", mData->size() - 1);
    if (startXRef < 0)
        throw ParseError(0, "Incorrect trailer, the marker 'startxref' was not found.");

    qint64 pos = startXRef + strlen("startxref");
    quint64 xrefPos = mData->readUInt(&pos, &ok);
    if (!ok)
        throw ParseError(pos, "Error in trailer, can't read xref position.");

    // Read xref table ..........................
    pos = readXRefTable(xrefPos, &mXRefTable);
    pos = mData->skipSpace(pos+strlen("trailer"));
    readDict(pos, &mTrailerDict);

    qint64 parentXrefPos = mTrailerDict.value("Prev").toNumber().value();

    while (parentXrefPos)
    {
        pos = readXRefTable(parentXrefPos, &mXRefTable);
        pos = mData->skipSpace(pos+strlen("trailer"));
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


