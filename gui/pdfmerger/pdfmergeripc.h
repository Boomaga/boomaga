#ifndef PDFMERGERIPC_H
#define PDFMERGERIPC_H

#include <QObject>
#include <QString>
#include <QTextStream>

class QProcess;
class QRectF;


class PdfMergerIPCReader: public QObject
{
    Q_OBJECT
public:
    PdfMergerIPCReader(QProcess *process, QObject *parent = 0);

signals:
    void allPagesCount(int pageCount);
    void pageInfo(int fileNum, int pageNum, uint objNum, const QRectF &cropBox);
    void xRefInfo(qint64 xrefPos, qint32 freeNum);
    void progress(int pageNum, int total);
    void error(const QString &message);


private slots:
    void mergerOutputReady();

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
    void writePageInfo(int fileNum, int pageNum, uint objNum, const QRectF &cropBox);
    void writeXRefInfo(qint64 xrefPos, qint32 freeNum);
    void writeProgressStatus(int pageNum);
    void writeError(const QString &message);
    void writeDebug(const QString &message);

private:
    QTextStream mStdOut;
};

#endif // PDFMERGERIPC_H
