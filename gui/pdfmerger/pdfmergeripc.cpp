#include "pdfmergeripc.h"

#include <QProcess>
#include <QRectF>
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
            emit pageInfo(data.at(1).toInt(),
                          data.at(2).toInt(),
                          data.at(3).toInt(),
                          strToRect(data.at(4)));
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
        else if (data.at(0) == "D")
        {
            qDebug() << data.at(1);
        }

        line = "";
    }

    mBuf.remove(0, i);
}


/************************************************
 *
 * ***********************************************/
PdfMergerIPCWriter::PdfMergerIPCWriter(QObject *parent):
    QObject(parent),
    mStdOut(stdout)
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
    mStdOut << "A:" << pageCount << '\n';
}


/************************************************
 *
 * ***********************************************/
void PdfMergerIPCWriter::writePageInfo(int fileNum, int pageNum, uint objNum, const QRectF &cropBox)
{
    mStdOut << "P:"
            << fileNum << ':'
            << pageNum << ':'
            << objNum  << ':'
            << rectToString(cropBox)
            << "\n";
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
    mStdOut << "S:" << pageNum << '\n';
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
void PdfMergerIPCWriter::writeDebug(const QString &message)
{
    mStdOut << "D:" + message + '\n';
    mStdOut.flush();
}
