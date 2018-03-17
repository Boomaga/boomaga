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


#include "pdfmergeripc.h"

#include <QProcess>
#include <QDebug>


/************************************************
 *
 * ***********************************************/
QString rectToString(const QRectF &rect)
{

    return QString("%1,%2,%3,%4")
            .arg(rect.left())
            .arg(rect.top())
            .arg(rect.width())
            .arg(rect.height());

}

/************************************************
 *
 * ***********************************************/
QRectF strToRect(const QString &str, bool *ok = 0)
{
    if (ok)
        *ok = false;

    QStringList items = str.split(",");

    if (items.count() < 4)
        return QRectF();

    bool ok1, ok2, ok3, ok4;
    QRectF rect = QRectF(items.at(0).toDouble(&ok1),
                         items.at(1).toDouble(&ok2),
                         items.at(2).toDouble(&ok3),
                         items.at(3).toDouble(&ok4));

    if (ok1 && ok2 && ok3 && ok4)
    {
        if (ok)
            *ok = true;

        return rect;
    }

    return QRectF();
}




/************************************************
 *
 * ***********************************************/
PdfMergerIPCReader::PdfMergerIPCReader(QProcess *process, QObject *parent):
    QObject(parent),
    mProcess(process),
    mPageCount(0)
{
    connect(mProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(mergerOutputReady()));

    connect(mProcess, SIGNAL(readyReadStandardError()),
            this, SLOT(mergerStdErrReady()));
}



/************************************************
 *
 * ***********************************************/
void PdfMergerIPCReader::mergerOutputReady()
{
    mBuf += QString::fromLocal8Bit(mProcess->readAllStandardOutput());
    QString line;
    int i;
    for (i=0; i<mBuf.length(); ++i)
    {
        QChar c = mBuf.at(i);
        if (c != '\n')
        {
            line += c;
            continue;
        }


        QStringList data = line.split(':', QString::KeepEmptyParts);

        //***************************************
        if (data.at(0) == "S")
        {
            emit progress(data.at(1).toInt(), mPageCount);
        }


        //***************************************
        else if (data.at(0) == "P")
        {
            PdfPageInfo info;
            foreach(QString s, data.at(3).split('_', QString::SkipEmptyParts))
                info.xObjNums << s.toInt();

            info.cropBox = strToRect(data.at(4));
            info.mediaBox = strToRect(data.at(5));
            info.rotate = data.at(6).toInt();

            emit pageInfo(data.at(1).toInt(),
                          data.at(2).toInt(),
                          info);
        }


        //***************************************
        else if (data.at(0) == "A")
        {
            mPageCount = data.at(1).toInt();
            emit allPagesCount(mPageCount);
        }


        //***************************************
        else if (data.at(0) == "X")
        {
            emit xRefInfo(data.at(1).toDouble(),
                          data.at(2).toDouble());
        }


        //***************************************
        else if (data.at(0) == "E")
        {
            emit error(data.at(1));
        }

        //***************************************
        else if (data.at(0) == "W")
        {
            qWarning() << data.at(1);
        }

        //***************************************
        else if (data.at(0) == "D")
        {
            qDebug() << data.at(1);
        }

        line = "";
    }

    mBuf.remove(0, i);
}


/************************************************

 ************************************************/
void PdfMergerIPCReader::mergerStdErrReady()
{
    QTextStream out(stderr);
    out << mProcess->readAllStandardError();
}


/************************************************
 *
 * ***********************************************/
PdfMergerIPCWriter::PdfMergerIPCWriter(QObject *parent):
    QObject(parent),
    mStdOut(stdout),
    mPageCount(0),
    mProgresPageNum(0)
{

}


/************************************************
 *
 * ***********************************************/
PdfMergerIPCWriter::~PdfMergerIPCWriter()
{
    mStdOut.flush();

}


/************************************************
 *
 * ***********************************************/
void PdfMergerIPCWriter::writeAllPagesCount(int pageCount)
{
    mPageCount = pageCount;
    mStdOut << "A:" << pageCount << '\n';
}


/************************************************
 *
 * ***********************************************/
void PdfMergerIPCWriter::writePageInfo(int fileNum, int pageNum, const PdfPageInfo &pageInfo)
{
    QString objNums;
    foreach(uint n, pageInfo.xObjNums)
    {
        if (objNums.isEmpty())
            objNums += QString("%1").arg(n);
        else
            objNums += QString("_%1").arg(n);
    }

    mStdOut << "P:"
            << fileNum << ':'
            << pageNum << ':'
            << objNums  << ':'
            << rectToString(pageInfo.cropBox)  << ':'
            << rectToString(pageInfo.mediaBox) << ':'
            << pageInfo.rotate << "\n";
}


/************************************************
 *
 * ***********************************************/
void PdfMergerIPCWriter::writeXRefInfo(qint64 xrefPos, qint32 freeNum)
{
    mStdOut << "X:"
            << xrefPos << ':'
            << freeNum << ':'
            << '\n';
}


/************************************************
 *
 * ***********************************************/
void PdfMergerIPCWriter::writeProgressStatus(int pageNum)
{
    mProgresPageNum = pageNum;
    mStdOut << "S:" << pageNum << '\n';
    mStdOut.flush();
}


/************************************************
 *
 ************************************************/
void PdfMergerIPCWriter::writeNextProgress()
{
    ++mProgresPageNum;
    mStdOut << "S:" << mProgresPageNum << '\n';
    mStdOut.flush();
}


/************************************************
 *
 * ***********************************************/
void PdfMergerIPCWriter::writeError(const QString &message)
{
    mStdOut << "E:" + message + '\n';
    mStdOut.flush();
}


/************************************************
 *
 * ***********************************************/
void PdfMergerIPCWriter::writeWarning(const QString &message)
{
    mStdOut << "W:" + message + '\n';
    mStdOut.flush();
}


/************************************************
 *
 * ***********************************************/
void PdfMergerIPCWriter::writeDebug(const QString &message)
{
    mStdOut << "D:" + message + '\n';
    mStdOut.flush();
}
