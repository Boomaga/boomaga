/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)GPL2+
 *
 *
 * Copyright: 2012-2013 Boomaga team https://github.com/Boomaga
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

 // This code based on poppler pdfunite.cc


#ifndef PDFMERGER_H
#define PDFMERGER_H

#include "pdfmergeripc.h"

#include <QList>
#include <QVector>
#include <QRectF>
#include <poppler/PDFDoc.h>

class PdfMergerPageInfo;

class PdfMerger
{
public:
    PdfMerger();
    ~PdfMerger();
    int majorVer() const { return mMajorVer; }
    int minorVer() const { return mMinorVer; }

    PDFDoc *addFile(const QString &fileName, qint64 startPos, qint64 endPos);
    bool run(const QString &outFileName);

    qint64 xrefPos() const { return mXrefPos; }

private:
    XRef mXRef;
    QList<PDFDoc*> mDocs;
    QVector<PdfMergerPageInfo*> mOrigPages;
    int mMajorVer;
    int mMinorVer;
    FileOutStream *mStream;

    Guint  mNextFreeNum;
    qint64 mXrefPos;
    bool writePageAsXObject(PdfMergerPageInfo *pageInfo);
    void writeStreamAsXObject(PdfMergerPageInfo *pageInfo, Stream *stream);
    void copyStream(Stream *stream, PdfMergerPageInfo *pageInfo);
    bool writeDictValue(Dict *dict, const char *key, Guint numOffset);
    QString getDocumentMetaInfo(PDFDoc *doc, const char *tag);
};

#endif // PDFMERGER_H
