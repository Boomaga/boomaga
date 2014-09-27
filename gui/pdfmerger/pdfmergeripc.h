/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2014 Boomaga team https://github.com/Boomaga
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


#ifndef PDFMERGERIPC_H
#define PDFMERGERIPC_H

#include <QObject>
#include <QString>
#include <QTextStream>
#include <QRectF>

class QProcess;

class PdfPageInfo
{
public:
    PdfPageInfo():
        objNum(-1),
        rotate(0)
    {
    }

    uint    objNum;
    QRectF  mediaBox;
    QRectF  cropBox;
    int    rotate;
};


class PdfMergerIPCReader: public QObject
{
    Q_OBJECT
public:
    PdfMergerIPCReader(QProcess *process, QObject *parent = 0);

signals:
    void allPagesCount(int pageCount);
    void pageInfo(int fileNum, int pageNum, const PdfPageInfo &pageInfo);
    void xRefInfo(qint64 xrefPos, qint32 freeNum);
    void progress(int pageNum, int total);
    void error(const QString &message);


private slots:
    void mergerOutputReady();
    void mergerStdErrReady();

private:
    QProcess *mProcess;
    QString mBuf;
    int mPageCount;
};


class PdfMergerIPCWriter: public QObject
{
    Q_OBJECT
public:
    PdfMergerIPCWriter(QObject *parent = 0);
    ~PdfMergerIPCWriter();

    void writeAllPagesCount(int pageCount);
    void writePageInfo(int fileNum, int pageNum, const PdfPageInfo &pageInfo);
    void writeXRefInfo(qint64 xrefPos, qint32 freeNum);
    void writeProgressStatus(int pageNum);
    void writeError(const QString &message);
    void writeWarning(const QString &message);
    void writeDebug(const QString &message);

private:
    QTextStream mStdOut;
};

#endif // PDFMERGERIPC_H
