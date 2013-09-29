#ifndef TMPPDFFILE_H
#define TMPPDFFILE_H

#include <QList>
#include <QStringList>
#include <QHash>
#include "project.h"

class QProcess;
class Sheet;
class Job;
class Render;

struct TmpPdfFilePage
{
    int     index;
    int     pdfObjNum;
    QRectF  rect;
};


class TmpPdfFile: public QObject
{
    Q_OBJECT
    friend class PdfMerger;
public:
    explicit TmpPdfFile(const QList<Job> jobs, QObject *parent = 0);
    virtual ~TmpPdfFile();

    void merge();
    void updateSheets(QList<Sheet *> &sheets);
    void stop();

    QList<Job> jobs() const  { return  mJobs; }

    QString fileName() const { return mFileName; }

    int inputFilePageCount(const QString &fileName) const { return mJobsPageCounts.value(fileName); }
    const TmpPdfFilePage page(int i) const { return  mPages.at(i); }

    void writeDocument(const QList<Sheet *> &sheets, QIODevice *out);

    bool isValid() const { return mValid; }

    QImage image(int sheetNum) const;

signals:
    void merged();
    void progress(int progress, int all) const;
    void imageChanged(int sheetNum);

private slots:
    void mergerOutputReady();
    void mergerFinished(int exitCode);

private:
    void getPageStream(QString *out, const Sheet *sheet) const;
    void writeSheets(QIODevice *out, const QList<Sheet *> &sheets) const;

    static QString genFileName();

    QList<Job> mJobs;

    QHash<QString,int> mJobsPageCounts;
    QVector<TmpPdfFilePage> mPages;
    QString mFileName;
    qint32 mFirstFreeNum;
    qint64 mOrigFileSize;
    qint64 mOrigXrefPos;
    QProcess *mMerger;
    QByteArray mBuf;
    int mPageCount;
    bool mValid;
    Render *mRender;
};

#endif // TMPPDFFILE_H
