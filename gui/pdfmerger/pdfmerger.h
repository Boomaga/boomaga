#ifndef PDFMERGER_H
#define PDFMERGER_H

#include <QList>
#include <QVector>
#include <QRectF>
#include <poppler/PDFDoc.h>


/* Protocol ********************************
    Line format     Description
    F:n:cnt         File page counts:
                        n - file index.
                        cnt page count in this PDF file

    N:num           Next XRef free num
    X:pos           XRef position

    P:n:num:rect    PDF page info:
                        n - page number
                        num -  XForm object number for page
                        rect- page Rectangle format is
                            left,top,width,height
    E:msg           Error message

    A:cnt           All pages count

    S:page          Progress status:
                        page - page num
 ******************************************/

struct PdfPageInfo
{
    PdfPageInfo():
        doc(0),
        numOffset(-1),
        objNum(-1),
        pageNum(-1)
    {
    }

    ~PdfPageInfo()
    {
        page.free();
        stream.free();
    }

    PDFDoc *doc;
    Guint   numOffset;
    Object  page;
    Object  stream;
    Guint   objNum;
    int     pageNum;
    QRectF  mediaBox;
    QRectF  cropBox;
};


class PdfMerger
{
public:
    PdfMerger();
    ~PdfMerger();
    int majorVer() const { return mMajorVer; }
    int minorVer() const { return mMinorVer; }

    PDFDoc *addFile(const QString &fileName);
    bool run(const QString &outFileName);

    qint64 xrefPos() const { return mXrefPos; }

    //const QVector<PdfPageInfo*> pages() { return mOrigPages; }

private:
    XRef mXRef;
    QList<PDFDoc*> mDocs;
    QVector<PdfPageInfo*> mOrigPages;
    int mMajorVer;
    int mMinorVer;
    FileOutStream *mStream;

    Guint  mNextFreeNum;
    qint64 mXrefPos;
    bool writePageAsXObject(PdfPageInfo *pageInfo);
    bool writeDictValue(Dict *dict, const char *key, Guint numOffset);
};

#endif // PDFMERGER_H
