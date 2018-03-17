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


#ifndef PDFMERGER_H
#define PDFMERGER_H

#include <QObject>
#include "pdfparser/pdfreader.h"
#include "pdfmergeripc.h"
#include <QString>
#include <QList>
#include <QIODevice>

namespace  PDF {
    class Reader;
    class Writer;
    class Object;
}

class PdfMerger: public QObject
{
    Q_OBJECT
public:
    PdfMerger(QObject *parent = 0);
    ~PdfMerger();

    void addSourceFile(const QString &fileName, qint64 startPos = 0, qint64 endPos = 0);
    void run(const QString &outFileName);
    void run(QIODevice *outDevice);

signals:
    void error(const QString &message);

private:
    struct SourceFile {
        QString fileName;
        qint64 startPos;
        qint64 endPos;
    };

    struct PageInfo;
    QList<SourceFile> mSources;
};

#endif // PDFMERGER_H
