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


#ifndef PDFWRITER_H
#define PDFWRITER_H

#include <QIODevice>
#include "pdfvalue.h"
#include "pdfxref.h"

namespace PDF {

class Object;

class Writer
{
public:

    ///Constructs a stream writer.
    /// \sa setDevice().
    Writer();

    /// Constructs a stream writer that writes into device;
    Writer(QIODevice *device);

    /// Destroys the writer.
    ~Writer();

    /// Returns the current device associated with the Writer, or 0 if no device has been assigned.
    /// \sa setDevice().
    QIODevice *device() const { return mDevice; }

    /// Sets the current device to device. If you want the stream to write
    /// into a QByteArray, you can create a QBuffer device.
    /// \sa device().
    void setDevice(QIODevice *device);


    /// Writes a PDF document header identifying the version of the PDF
    /// specification to which the file conforms.
    /// This also writes the comment line containing four binary characters. This ensures
    /// proper behavior of file transfer applications that inspect data near the beginning
    /// of a file to determine whether to treat the file’s contents as text or as binary.
    /// sa\ writeEndDocument().
    void writePDFHeader(int majorVersion = 1, int minorVersion = 7);


    void writeObject(const Object &object);

    void writeXrefTable();

    /// Write PDF trailer
    ///  root - The catalog dictionary for the PDF document.
    void writeTrailer(const Link &root);

    /// Write PDF trailer
    ///  root - The catalog dictionary for the PDF document.
    ///  info - The document’s information dictionary.
    void writeTrailer(const Link &root, const Link &info);
    void writeTrailer(const Dict &trailerDict);

    const XRefTable xRefTable() const { return mXRefTable; }

    void writeComment(const QString &comment);

protected:
    void writeValue(const Value &value);
    void writeXrefSection(const XRefTable::const_iterator &start, quint32 count);
    void writeLiteralString(const String &value);

    void write(const char value);
    void write(const char* value);
    void write(const QString &value);
    void write(double value);
    void write(quint64 value);
    void write(quint32 value);
    void write(quint16 value);
    void write(qint64 value);
    void write(qint32 value);
    void write(qint16 value);

private:
    QIODevice *mDevice;
    XRefTable mXRefTable;
    qint64 mXRefPos;

    char mBuf[1024*1024];
};


} // namespace PDF



#endif // PDFWRITER_H
