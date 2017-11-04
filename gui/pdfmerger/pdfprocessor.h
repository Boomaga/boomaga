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


#ifndef PDFPROCESSOR_H
#define PDFPROCESSOR_H

#include "pdfmergeripc.h"
#include <QVector>
#include <QString>
#include <QFile>
#include <QSet>
#include "pdfparser/pdfvalue.h"
#include "pdfparser/pdfreader.h"

namespace  PDF {
    class Writer;
    class Object;
    class Dict;
}

class PdfProcessor: public QObject
{
    Q_OBJECT
public:
    PdfProcessor(const QString &fileName, qint64 startPos = 0, qint64 endPos = 0);
    ~PdfProcessor();

    void open();

    quint32 pageCount();

    void run(PDF::Writer *writer, quint32 objNumOffset);

    const QVector<PdfPageInfo> &pageInfo() const { return mPageInfo; }
signals:
    void pageReady();

private:
    QString mFileName;
    qint64 mStartPos;
    qint64 mEndPos;
    PDF::Reader mReader;
    quint32 mObjNumOffset;
    PDF::Writer *mWriter;
    QVector<PdfPageInfo> mPageInfo;
    QSet<PDF::ObjNum> mProcessedObjects;

    int walkPageTree(int pageNum, const PDF::Object &page, const PDF::Dict &inherited);
    PDF::ObjNum writeContentAsXObject(const PDF::Link &contentLink, const PDF::Dict &pageDict, const PDF::Dict &inherited);
    PDF::Object &addOffset(PDF::Object &obj);
    void offsetValue(PDF::Value &value);
};

#endif // PDFPROCESSOR_H
