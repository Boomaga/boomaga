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

#include <QtGlobal>
#include <exception>
#include "pdfvalue.h"
#include "pdfxref.h"

class QFile;

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

/// The PDF::REader class provides a way to read PDF documents.
class Reader
{
public:
    /// Constructs a Reader object.
    Reader();

    ///Destroys the object and frees its resources, closing it if necessary.
    virtual ~Reader();

    /// Reads PDF document from the existing file, starting at position startPos until endPos.
    /// If endPos is 0 (the default), Reader reads all bytes starting at position startPos
    /// until the end of the file. If the fileName has no path or a relative path, the path
    /// used will be the application's current directory path at the time of the open() call.
    void open(const QString &fileName, quint64 startPos = 0, quint64 endPos = 0);

    /// Reads PDF document from first size bytes of the data.
    /// The bytes are not copied. The Reader will contain the data pointer. The caller
    /// guarantees that data will not be deleted or modified as long as this Reader exist.
    void open(const char * const data, quint64 size);

    /// Closes this reader for reading.
    void close();

    const XRefTable &xRefTable() const { return mXRefTable; }
    const Dict &trailerDict() const { return mTrailerDict; }
    Dict trailerDict() { return mTrailerDict; }

    Object getObject(const Link &link) const;
    Object getObject(uint objNum, quint16 genNum) const;
    Object getObject(const XRefEntry &xrefEntry) const;

    const Value find(const QString &path) const;

    quint32 pageCount();

    /// Constructs a QByteArray that uses len bytes from the data,
    /// starting at position pos. The bytes are not copied.
    /// The caller guarantees that reader will not be closed as long
    /// as this QByteArray and any copies of it exist.
    QByteArray rawData(quint64 pos, quint64 len) const;

protected:
    void   load();
    Value  readValue(quint64 *pos) const;

    qint64 readObject(quint64 start, Object *res) const;
    void readObjectFromStream(PDF::ObjNum objNum, Object *res, PDF::ObjNum streamObjNum, GenNum streamGenNum, quint32 stremIndex) const;
    qint64 readXRefTable(quint64 start, XRefTable *res, Dict *trailerDict) const;
    qint64 readXRefStream(qint64 start, XRefTable *xref, Dict *trailerDict) const;
private:
    QFile      *mFile;
    const char *mData;
    quint64     mSize;
    XRefTable   mXRefTable;
    Dict        mTrailerDict;
    int         mPagesCount;
    QTextCodec  *mTextCodec;

    class Cache;
    Cache       *mCache;
};

} // namespace PDF
#endif // PDFREADER_H
