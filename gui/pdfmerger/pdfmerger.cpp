#include "pdfmerger.h"
#include <QtAlgorithms>
#include <QDebug>
//#include <QFile>
//#include <QCryptographicHash>
//#include <QVector>
//#include <QApplication>

#include "../kernel/project.h"
//#include ".h"
//#include "math.h"

#include <poppler/PDFDoc.h>
#include <poppler/GlobalParams.h>
#include <poppler/poppler-config.h>

static GBool printVersion = true;
static GBool printHelp = true;



void POPPLER_WriteObject(Object *obj, Ref *ref, OutStream* outStr, XRef *xref, Guint numOffset)
{
// 0.18.14
//static Guint writeObject (Object *obj, Ref *ref, OutStream* outStr, XRef *xref, Guint numOffset);
    PDFDoc::writeObject(obj, 0, outStr, xref, numOffset);

// 0.24.1
//static void writeObject (Object *obj, OutStream* outStr, XRef *xref, Guint numOffset, Guchar *fileKey,
//                         CryptAlgorithm encAlgorithm, int keyLength, int objNum, int objGen);
// PDFDoc::writeObject(&value, outStr, yRef, offsets[i], NULL, cryptRC4, 0, 0, 0);

//    PDFDoc::writeObject(obj, outStr, xref, numOffset, NULL, cryptRC4, 0, 0, 0);
}

void POPPLER_WritePageObjects(PDFDoc *doc, OutStream *outStr, XRef *xRef, Guint numOffset)
{
#if POPPLER_VERSION < 2200
        doc->writePageObjects(outStr, xRef, numOffset);
#else
        doc->writePageObjects(outStr, xRef, numOffset, true);
#endif
}


/************************************************

 ************************************************/
void error(const QString &message)
{
    QTextStream out(stdout);
    out << "E:" << message << endl;
    exit(3);
}


/************************************************

 ************************************************/
void print(const QString &message)
{
    QTextStream out(stdout);
    out << message << endl;
}


/************************************************

 ************************************************/
OutStream &operator<<(OutStream &stream, const QString &str)
{
    stream.printf("%s", str.toLocal8Bit().constData());
    return stream;
}


/************************************************

 ************************************************/
OutStream &operator<<(OutStream &stream, const char *str)
{
    stream.printf("%s", str);
    return stream;
}


/************************************************

 ************************************************/
OutStream &operator<<(OutStream &stream, const PDFRectangle &rect)
{
    stream << QString("[ %1 %2 %3 %4 ]")
              .arg(rect.x1)
              .arg(rect.y1)
              .arg(rect.x2)
              .arg(rect.y2);
    return stream;
}


/************************************************

 ************************************************/
OutStream &operator<<(OutStream &stream, const QRectF &rect)
{
    stream << QString("[ %1 %2 %3 %4 ]")
              .arg(rect.left())
              .arg(rect.top())
              .arg(rect.right())
              .arg(rect.bottom());
    return stream;
}


/************************************************

 ************************************************/
OutStream &operator<<(OutStream &stream, const PDFRectangle *rect)
{
    return operator<<(stream, *rect);
}


/************************************************

 ************************************************/
OutStream &operator<<(OutStream &stream, const unsigned int value)
{
    stream << QString("%1").arg(value);
    return stream;
}


/************************************************

 ************************************************/
OutStream &operator<<(OutStream &stream, const int value)
{
    stream << QString("%1").arg(value);
    return stream;
}


/************************************************

 ************************************************/
OutStream &operator<<(OutStream &stream, Stream &value)
{
    Object obj1;
    value.getDict()->lookup((char*)"Length", &obj1);
    const int length = obj1.getInt();
    obj1.free();

    value.unfilteredReset();

    for (int i=0; i<length; i++)
    {
        int c = value.getUnfilteredChar();
        stream.printf("%c", c);
    }

    value.reset();
    return stream;
}


/************************************************

 ************************************************/
OutStream &operator<<(OutStream &stream, Stream *value)
{
    return operator<<(stream, *value);
}


/************************************************

 ************************************************/
PdfMerger::PdfMerger():
    mMajorVer(1),
    mMinorVer(4),
    mStream(0),
    mNextFreeNum(0),
    mXrefPos(0)
{
    static bool popplerGlobalParamsInited(false);
     if (!popplerGlobalParamsInited)
     {
         globalParams = new GlobalParams();
        popplerGlobalParamsInited = true;
     }
}


/************************************************

 ************************************************/
PdfMerger::~PdfMerger()
{
    qDeleteAll(mOrigPages);
    qDeleteAll(mDocs);
    delete mStream;
}


/************************************************

 ************************************************/
PDFDoc *PdfMerger::addFile(const QString &fileName)
{
    // PDFDoc take ownership of the GooString object.
    PDFDoc *doc = new PDFDoc(new GooString(fileName.toLocal8Bit()), 0, 0, 0);


    if (!doc->isOk())
    {
        error(QObject::tr("PDF file \"%1\" is damaged.").arg(fileName));
        return 0;
    }

    if (doc->isEncrypted())
    {
        error(QObject::tr("PDF file \"%1\" is encripted.").arg(fileName));
        return 0;
    }

    if (doc->getPDFMajorVersion() > mMajorVer)
    {
        mMajorVer = doc->getPDFMajorVersion();
        mMinorVer = doc->getPDFMinorVersion();
    }
    else if (doc->getPDFMajorVersion() == mMinorVer)
    {
        if (doc->getPDFMinorVersion() > mMinorVer)
        {
            mMinorVer = doc->getPDFMinorVersion();
        }
    }

    print(QString("F:%1:%2").arg(mDocs.count()).arg(doc->getNumPages()));

    mDocs << doc;
    return doc;
}


/************************************************

 ************************************************/
bool PdfMerger::run(const QString &outFileName)
{
    FILE *f = fopen(outFileName.toLocal8Bit(), "wb");
    if (!f)
        error(QObject::tr("I can't open file \"%1\"").arg(outFileName));

    mStream = new FileOutStream(f, 0);
    PDFDoc::writeHeader(mStream, mMajorVer, mMinorVer);

    mXRef.add(0, 65535, 0, false);

    int rootNum = mXRef.getNumObjects();
    int pagesNum = rootNum +1;


    // Catalog object ...........................
    mXRef.add(rootNum, 0, mStream->getPos(), true);
    *mStream << rootNum << " 0 obj\n";
    *mStream << "<<\n";
    *mStream << "/Type /Catalog\n";
    *mStream << "/Pages " << pagesNum << " 0 R\n";
    *mStream << ">>\n";
    *mStream << "endobj\n";
    // ..........................................


    // Pages object .............................
    mXRef.add(pagesNum, 0, mStream->getPos(), true);
    *mStream << pagesNum << " 0 obj\n";
    *mStream << "<<\n";
    *mStream << "/Type /Pages\n";
    *mStream << "/Kids [ ] /Count 0\n";
    *mStream << ">>\n";
    *mStream << "endobj\n";
    // ..........................................

    int pagesCnt = 0;
    foreach (PDFDoc *doc, mDocs)
        pagesCnt += doc->getNumPages();

    mOrigPages.resize(pagesCnt);

    print(QString("A:%1").arg(pagesCnt));
    Guint numOffset = mXRef.getNumObjects();
    XRef countXref;

    int n=0;
    foreach (PDFDoc *doc, mDocs)
    {
        //*mStream << "\n% Document **************************************\n\n";
        for (int i = 1; i <= doc->getNumPages(); i++)
        {
            Page *page = doc->getCatalog()->getPage(i);
            {
                PDFRectangle *cropBox = 0;
                if (page->isCropped())
                    cropBox = page->getCropBox();

                doc->replacePageDict(i,
                                 doc->getCatalog()->getPage(i)->getRotate(),
                                 doc->getCatalog()->getPage(i)->getMediaBox(), cropBox, NULL);
            }

            Ref *refPage = doc->getCatalog()->getPageRef(i);
            PdfPageInfo *pageInfo = new PdfPageInfo();

            pageInfo->doc = doc;
            pageInfo->numOffset = numOffset;
            doc->getXRef()->fetch(refPage->num, refPage->gen, &(pageInfo->page));
            Dict *pageDict = pageInfo->page.getDict();

            PDFRectangle *mediaBox = page->getMediaBox();

            pageInfo->mediaBox.setLeft(mediaBox->x1);
            pageInfo->mediaBox.setTop(mediaBox->y1);
            pageInfo->mediaBox.setRight(mediaBox->x2);
            pageInfo->mediaBox.setBottom(mediaBox->y2);

            PDFRectangle *cropBox = page->getCropBox();
            pageInfo->cropBox.setLeft(cropBox->x1);
            pageInfo->cropBox.setTop(cropBox->y1);
            pageInfo->cropBox.setRight(cropBox->x2);
            pageInfo->cropBox.setBottom(cropBox->y2);

            pageInfo->pageNum = i;
            if (pageDict->hasKey((char *)"Contents"))
            {
                pageDict->lookup((char *)"Contents", &(pageInfo->stream));
                pageDict->remove((char *)"Contents");
            }

            mOrigPages[n] = pageInfo;

            doc->markPageObjects(pageDict, &mXRef, &countXref, numOffset);

            if (n % 2)
                print(QString("S:%1").arg((n +1) / 2));

            n++;
        }

        POPPLER_WritePageObjects(doc, mStream, &mXRef, numOffset);
        numOffset = mXRef.getNumObjects() + 1;
    }

    int xformStartNum = mXRef.getNumObjects();
    //*mStream  << "\n% XObjects **************************************\n\n";
    for (int i=0; i<mOrigPages.count(); ++i)
    {
        PdfPageInfo *pageInfo = mOrigPages[i];
        pageInfo->objNum = xformStartNum + i;
        writePageAsXObject(pageInfo);
        if (i % 2)
            print(QString("S:%1").arg(pagesCnt / 2 + i));

    }

    mXrefPos = mStream->getPos();
    mXRef.writeToFile(mStream, false);

    Ref ref;
    ref.num = rootNum;
    ref.gen = 0;

    PDFDoc::writeTrailer(mXrefPos, mXRef.getNumObjects(), mStream, false, 0,
      &ref, &mXRef, outFileName.toLocal8Bit(), mStream->getPos());

    //*mStream  << "\n% End update **************************************\n\n";

    mStream->close();
    delete mStream;
    mStream = 0;
    fclose(f);


    for (int i=0; i<mOrigPages.count(); ++i)
    {
        PdfPageInfo *pageInfo = mOrigPages.at(i);
        print(QString("P:%1:%2:%3,%4,%5,%6")
              .arg(i)
              .arg(pageInfo->objNum)
              .arg(pageInfo->cropBox.left())
              .arg(pageInfo->cropBox.top())
              .arg(pageInfo->cropBox.width())
              .arg(pageInfo->cropBox.height()));
    }

    print(QString("N:%1").arg(mXRef.getNumObjects()));
    print(QString("X:%1").arg(mXrefPos));

    return true;
}


/************************************************
    XObject            Page         Const
    ----------------------------------------
    Type                -           XObject
    Subtype             -           Form
    FormType            -           1
    BBox             CropBox
    Matrix              -
    Resources        Resources
    Group               -
    Ref                 -
    Metadata         Metadata
    PieceInfo        PieceInfo
    LastModified    LastModified
    StructParent        -
    StructParents   StructParents
    OPI                 -
    OC                  -
    Name                -
 ************************************************/
bool PdfMerger::writePageAsXObject(PdfPageInfo *pageInfo)
{
    mXRef.add(pageInfo->objNum, 0, mStream->getPos(), true);
    *mStream << pageInfo->objNum << " 0 obj\n";
    *mStream << "<<\n";
    Dict *pageDict = pageInfo->page.getDict();

    // Copy dict from the page object ...........
    *mStream << "/Type /XObject\n";
    *mStream << "/Subtype /Form\n";
    *mStream << "/FormType 1\n";
    *mStream << "/Boomaga  1\n";
    *mStream << "/BBox " << pageInfo->cropBox <<"\n";
    writeDictValue(pageDict, "Resources", pageInfo->numOffset);
    writeDictValue(pageDict, "Metadata", pageInfo->numOffset);
    writeDictValue(pageDict, "PieceInfo", pageInfo->numOffset);
    writeDictValue(pageDict, "LastModified", pageInfo->numOffset);
    writeDictValue(pageDict, "StructParents", pageInfo->numOffset);
    // ..........................................

    // Copy dict from the stream object .........
    Stream *stream = 0;
    if (pageInfo->stream.isStream())
    {
        stream = pageInfo->stream.getStream();

        Dict *dict = stream->getDict();
        for (int i=0; i<dict->getLength(); ++i)
        {
            Object value;
            dict->getValNF(i, &value);
            *mStream << "/" << dict->getKey(i) << " ";
            POPPLER_WriteObject(&value, 0, mStream, &mXRef, pageInfo->numOffset);
            *mStream << "\n";
            value.free();
        }
    }
    else
    {
        *mStream << "/Length 0\n";
    }
    *mStream << " >>\n";
    // ..........................................

    // Write stream .............................
    *mStream << "stream\n";
    if (stream)
        *mStream << *stream;
    *mStream << "endstream\n";
    // ..........................................

    *mStream << "endobj\n";
    return true;
}


/************************************************

 ************************************************/
bool PdfMerger::writeDictValue(Dict *dict, const char *key, Guint numOffset)
{
    if (!dict->hasKey((char*)key))
        return false;

    Object value;
    dict->lookupNF((char*)key, &value);
    *mStream << "/" << key << " ";
    POPPLER_WriteObject(&value, 0, mStream, &mXRef, numOffset);
    *mStream << "\n";
    value.free();
    return true;
}

