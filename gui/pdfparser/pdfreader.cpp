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
Reader::Reader(const char * const data, quint64 size):
    mData(new Data(data, size)),
    mHandler(nullptr)
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
bool Reader::canReadArray(qint64 pos) const
{
    return mData->at(pos) == '[';
}


/************************************************
 *
 ************************************************/
bool Reader::canReadBool(qint64 pos) const
{
    return mData->compareWord(pos, "true") ||
           mData->compareWord(pos, "false");
}


/************************************************
 *
 ************************************************/
bool Reader::canReadDict(qint64 pos) const
{
    return mData->at(pos)   == '<' &&
           mData->at(pos+1) == '<';
}


/************************************************
 *
 ************************************************/
bool Reader::canReadHexString(qint64 pos) const
{
    return mData->at(pos)   == '<' &&
           mData->at(pos+1) != '<';
}


/************************************************
 *
 ************************************************/
bool Reader::canReadLink(qint64 pos) const
{
    bool ok;
    mData->readUInt(&pos, &ok);
    if (!ok)
        return false;

    pos = mData->skipSpace(pos);

    mData->readUInt(&pos, &ok);
    if (!ok)
        return false;

    pos = mData->skipSpace(pos);

    return mData->at(pos) == 'R';
}


/************************************************
 *
 ************************************************/
bool Reader::canReadLiteralString(qint64 pos) const
{
    return mData->at(pos) == '(';
}


/************************************************
 *
 ************************************************/
bool Reader::canReadName(qint64 pos) const
{
    return mData->at(pos) == '/';
}


/************************************************
 *
 ************************************************/
bool Reader::canReadNull(qint64 pos) const
{
    return mData->compareWord(pos, "null");
}


/************************************************
 *
 ************************************************/
bool Reader::canReadNumber(qint64 pos) const
{
    bool ok;
    mData->readNum(&pos, &ok);
    return ok;
}


/************************************************
 *
 ************************************************/
Value Reader::readValue(qint64 *pos) const
{
    if (canReadDict(*pos))
    {
        Dict res;
        *pos = readDict(*pos, &res);
        return res;
    }

    if (canReadArray(*pos))
    {
        Array res;
        *pos = readArray(*pos, &res);
        return res;
    }

    if (canReadName(*pos))
    {
        Name res;
        *pos = readName(*pos, &res);
        return res;
    }

    if (canReadLink(*pos))
    {
        Link res;
        *pos = readLink(*pos, &res);
        return res;
    }

    if (canReadNumber(*pos))
    {
        Number res;
        *pos = readNumber(*pos, &res);
        return res;
    }

    if (canReadHexString(*pos))
    {
        HexString res;
        *pos = readHexString(*pos, &res);
        return res;
    }

    if (canReadBool(*pos))
    {
        Bool res;
        *pos = readBool(*pos, &res);
        return res;
    }

    if (canReadLiteralString(*pos))
    {
        LiteralString res;
        *pos = readLiteralString(*pos, &res);
        return res;
    }

    if (canReadNull(*pos))
    {
        Null res;
        *pos = readNull(*pos, &res);
        return res;
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
qint64 Reader::readBool(qint64 start, Bool *res) const
{
    if (mData->compareWord(start, "true"))
    {
        res->setValid(true);
        res->setValue(true);
        return start + 4;
    }

    if (mData->compareWord(start, "false"))
    {
        res->setValid(true);
        res->setValue(false);
        return start + 5;
    }

    throw ParseError(start, QString("Unexpected symbol '%1', expected a boolean.").arg(mData->at(start)));
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

        if (!canReadName(pos))
        {
            throw ParseError(pos, QString("Invalid PDF name on pos %1").arg(pos));
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
qint64 Reader::readLink(qint64 pos, Link *res) const
{
    bool ok;
    res->setObjNum(mData->readUInt(&pos, &ok));
    if (!ok)
        throw ParseError(pos, QString("Unexpected symbol '%1', expected a number.").arg(mData->at(pos)));

    pos = mData->skipSpace(pos);

    res->setGenNum(mData->readUInt(&pos, &ok));
    if (!ok)
        throw ParseError(pos, QString("Unexpected symbol '%1', expected a number.").arg(mData->at(pos)));

    pos = mData->skipSpace(pos);
    res->setValid(true);
    return pos + 1;
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
qint64 Reader::readName(qint64 start, Name *res) const
{
    res->setValid(true);
    res->setValue(readNameString(&start));
    return start;
}


/************************************************
 *
 ************************************************/
qint64 Reader::readNull(qint64 start, Null *res) const
{
    if (mData->compareWord(start, "null"))
    {
        res->setValid(true);
        return start + 4;
    }

    throw ParseError(start, QString("Invalid PDF null on pos %1").arg(start));
}


/************************************************
 *
 ************************************************/
qint64 Reader::readNumber(qint64 pos, Number *res) const
{
    bool ok;
    res->setValue(mData->readNum(&pos, &ok));
    if (!ok)
        throw ParseError(pos, QString("Unexpected symbol '%1', expected a number.").arg(mData->at(pos)));

    res->setValid(true);
    return pos;
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
    Object obj;
    if (mXRefTable.value(objNum).pos)
        readObject(mXRefTable.value(objNum).pos, &obj);
    return obj;
}


/************************************************
 *
 ************************************************/
QString Reader::readNameString(qint64 *pos) const
{
    qint64 start = *pos;
    for (++(*pos); *pos < mData->size(); ++(*pos))
    {
        if (mData->isDelim(*pos))
        {
            return QString::fromLocal8Bit(mData->data() + start + 1, *pos - start - 1);
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
//    // Check header ...................................
//    if (mData->indexOf("%PDF-") != 0)
//        throw HeaderError(0);

//    bool ok;
//    // Get xref table position ..................
//    qint64 startXRef = mData->indexOfBack("startxref", mData->size() - 1);
//    if (startXRef < 0)
//        throw ParseError(0, "Incorrect trailer, the marker 'startxref' was not found.");

//    qint64 pos = startXRef + strlen("startxref");
//    quint64 xrefPos = mData->readUInt(&pos, &ok);
//    if (!ok)
//        throw ParseError(pos, "Error in trailer, can't read xref position.");

//    // Read xref table ..........................
//    pos = readXRefTable(xrefPos, &mXRefTable);
//    pos = mData->skipSpace(pos+strlen("trailer"));
//    readDict(pos, &mTrailerDict);

//    qint64 parentXrefPos = mTrailerDict.value("Prev").toNumber().value();

//    while (parentXrefPos)
//    {
//        pos = readXRefTable(parentXrefPos, &mXRefTable);
//        pos = mData->skipSpace(pos+strlen("trailer"));
//        Dict dict;
//        readDict(pos, &dict);
//        parentXrefPos = dict.value("Prev").toNumber().value();
//    }

//    mHandler->trailerReady(mXRefTable, mTrailerDict);

    foreach (const XRefEntry &entry, mXRefTable)
    {
        if (entry.type == XRefEntry::Used)
        {
            Object obj = Object();
            readObject(entry.pos, &obj);
            mHandler->objectReady(obj);
        }
    }
}


/************************************************
 *
 ************************************************/
void Reader::setHandler(ReaderHandler *handler)
{
    mHandler = handler;
}
