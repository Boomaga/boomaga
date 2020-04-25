/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2013 Boomaga team https://github.com/Boomaga
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


#ifndef TMPPDFFILE_H
#define TMPPDFFILE_H

#include <QObject>
#include <QVector>
#include <QFile>
#include "boomagatypes.h"
#include "pdfparser/pdfxref.h"

class Sheet;
class Job;
class JobList;
struct JobFile;

namespace PDF {
    class Writer;
}

#include "job.h"

class TmpPdfFile: public QObject
{
    Q_OBJECT
    friend class PdfMerger;
public:
    explicit TmpPdfFile(QObject *parent = 0);
    virtual ~TmpPdfFile();

    JobList add(const JobList &jobs);
    void updateSheets(const QList<Sheet *> &sheets);

    QString fileName() const { return mFileName; }

    bool writeDocument(const QList<Sheet*> &sheets, QIODevice *out);
    bool isValid() const { return mValid; }

    QString errorString() const { return mErrorString; }

private:
    void getPageStream(QString *out, const Sheet *sheet) const;
    void writeSheets(QIODevice *out, const QList<Sheet *> &sheets) const;
    quint64 writeCatalog(PDF::Writer *writer, const QVector<PdfPageInfo> &pages);

    class Data;
    Data *mData;

    QString mFileName;
    QFile   mFile;


    qint32 mFirstFreeNum = 0;
    qint64 mOrigFileSize = 0;
    qint64 mOrigXrefPos  = 0;
    bool mValid = false;
    QString mErrorString;
};


#endif // TMPPDFFILE_H
