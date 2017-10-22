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


#ifndef PDFREADER_H
#define PDFREADER_H

//#include <stddef.h>
#include <QtGlobal>
#include <exception>
#include "pdfvalue.h"
#include "pdfxref.h"

namespace PDF {

class Object;

/**
    \brief The ContentHandler class provides an interface to report the logical content of PDF object.

    If the application needs to be informed of basic parsing events, it can implement this interface and
    activate it using Reader::setHandler(). The reader can then report basic document-related events
    like the end of PDF object through this interface.
*/

class ReaderHandler
{
public:
    virtual void trailerReady(const XRefTable &xRefTable, const Dict &trailerDict) = 0;
    virtual void objectReady(const Object &object) = 0;
};

/**
    \brief The Reader class provides an implementation of a simple PDF parser.

    This PDF reader is suitable for a wide range of applications. It is able to parse well-formed PDF and
    can report the elements to a content handler.

    The easiest pattern of use for this class is to create a reader instance, define an input source,
    specify the handler to be used by the reader, and parse the data.
*/

class Data;


class Reader
{
public:
    Reader(const char * const data, quint64 size);
    virtual ~Reader();

    void open();
    void load();


    /// Returns the content handler or 0 if none was set.
    /// \sa setHandler().
    ReaderHandler *handler() const { return mHandler; }

    /// Sets the content handler to handler.
    /// \sa handler().
    void setHandler(ReaderHandler *handler);

    const XRefTable &xRefTable() { return mXRefTable; }
    //const Dict &trailerDict() { return mTrailerDict; }
    Dict trailerDict() { return mTrailerDict; }

    Object getObject(const Link &link) const;
    Object getObject(uint objNum, quint16 genNum) const;

protected:
    bool canReadArray(qint64 pos) const;
    bool canReadBool(qint64 pos) const;
    bool canReadDict(qint64 pos) const;
    bool canReadHexString(qint64 pos) const;
    bool canReadLink(qint64 pos) const;
    bool canReadLiteralString(qint64 pos) const;
    bool canReadName(qint64 pos) const;
    bool canReadNull(qint64 pos) const;
    bool canReadNumber(qint64 pos) const;

    Value readValue(qint64 *pos) const;
    qint64 readArray(qint64 start, Array *res) const;
    qint64 readBool(qint64 start, Bool *res) const;
    qint64 readDict(qint64 start, Dict *res) const;
    qint64 readHexString(qint64 start, HexString *res) const;
    qint64 readLink(qint64 start, Link *res) const;
    qint64 readLiteralString(qint64 start, LiteralString *res) const;
    qint64 readName(qint64 start, Name *res) const;
    qint64 readNull(qint64 start, Null *res) const;
    qint64 readNumber(qint64 start, Number *res) const;
    qint64 readObject(qint64 start, Object *res) const;
    qint64 readXRefTable(qint64 start, XRefTable *res) const;


    QString readNameString(qint64 *pos) const;

private:
    const Data    *mData;
    XRefTable     mXRefTable;
    Dict          mTrailerDict;
    ReaderHandler *mHandler;
};

} // namespace PDF
#endif // PDFREADER_H
