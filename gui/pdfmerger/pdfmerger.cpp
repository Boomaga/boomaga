/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)GPL2+
 *
 *
 * Copyright: 2012-2013 Boomaga team https://github.com/Boomaga
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

 // This code based on poppler pdfunite.cc


#include "pdfmerger.h"
#include <QtAlgorithms>
#include <QDebug>
#include <QDir>

#include "../kernel/project.h"

#include <poppler/PDFDoc.h>
#include <poppler/GlobalParams.h>
#include <poppler/poppler-config.h>
#include <kernel/boomagapoppler.h>


#ifdef __GNUC__
#define GCC_VARIABLE_IS_USED __attribute__ ((unused))
#else
#define GCC_VARIABLE_IS_USED
#endif


static GBool GCC_VARIABLE_IS_USED printVersion = true;
static GBool GCC_VARIABLE_IS_USED printHelp = true;


void POPPLER_WriteObject(Object *obj, Ref *ref, OutStream* outStr, XRef *xref, Guint numOffset)
{
#if POPPLER_VERSION < 2100
    //static Guint writeObject (Object *obj, Ref *ref, OutStream* outStr, XRef *xref, Guint numOffset);
    PDFDoc::writeObject(obj, 0, outStr, xref, numOffset);
#else
    //static void writeObject (Object *obj, OutStream* outStr, XRef *xref, Guint numOffset, Guchar *fileKey,
    //                         CryptAlgorithm encAlgorithm, int keyLength, int objNum, int objGen);

    PDFDoc::writeObject(obj, outStr, xref, numOffset, NULL, cryptRC4, 0, 0, 0);
#endif
}


void POPPLER_WritePageObjects(PDFDoc *doc, OutStream *outStr, XRef *xRef, Guint numOffset)
{
#if POPPLER_VERSION < 2200
        doc->writePageObjects(outStr, xRef, numOffset);
#else
        doc->writePageObjects(outStr, xRef, numOffset, true);
#endif
}


void writeTrailer(XRef *xRef, int rootNum, OutStream* stream)
{

#if POPPLER_VERSION < 1904
    xRef->writeToFile(stream, false);

    Ref ref;
    ref.num = rootNum;
    ref.gen = 0;

    PDFDoc::writeTrailer(stream->getPos(), xRef->getNumObjects(), stream, false, 0,
                         &ref, xRef, "fileNamef", stream->getPos());
#else
    int uxrefOffset = stream->getPos();
    Ref ref;
    ref.num = rootNum;
    ref.gen = 0;

    Dict *trailerDict = PDFDoc::createTrailerDict(xRef->getNumObjects(), false, 0, &ref, xRef,
                                                  "fileName", stream->getPos());
    PDFDoc::writeXRefTableTrailer(trailerDict, xRef, false /* do not write unnecessary entries */,
                                  uxrefOffset, stream, xRef);
    delete trailerDict;
#endif
}

/************************************************

 ************************************************/
void error(const QString &message)
{
    PdfMergerIPCWriter().writeError(message);
    exit(3);
}


/************************************************

 ************************************************/
void warning(const QString &message)
{
    qWarning("%s", message.toLocal8Bit().data());
}


/************************************************

 ************************************************/
void warning(const char *message)
{
    warning(QString(message));
}


/************************************************

 ************************************************/
void debug(const QString &message)
{
    QTextStream out(stdout);
    PdfMergerIPCWriter().writeDebug(message);
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


class PdfMergerPageInfo: public PdfPageInfo
{
public:

    PdfMergerPageInfo():
        PdfPageInfo(),
        doc(0),
        numOffset(-1),
        pageNum(-1)
    {
    }

    ~PdfMergerPageInfo()
    {
        page.free();
        stream.free();
    }

    PDFDoc *doc;
    Guint   numOffset;
    Object  page;
    Object  stream;
    int     pageNum;

    QString dump();
};


/************************************************

 ************************************************/
QString PdfMergerPageInfo::dump()
{
    QString res;
    res += QString("  Page: %1\n").arg(pageNum);
    res += QString("  Content type: %1\n").arg(stream.getTypeName());
    if (stream.isArray())
    {
        Array *array = stream.getArray();
        res += QString("  Array length: %1\n").arg(array->getLength());

        for (int i=0; i < array->getLength(); ++i)
        {
            Object o;
            array->get(i, &o);
            res += QString("    * %1 - %2\n").arg(i).arg(o.getTypeName());
        }
    }

    return res;
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
    if (!globalParams)
        globalParams = new GlobalParams();
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
PDFDoc *PdfMerger::addFile(const QString &fileName, qint64 startPos, qint64 endPos)
{
    BoomagaPDFDoc *doc = new BoomagaPDFDoc(fileName, startPos, endPos);

    if (!doc->isValid())
    {
        error(doc->errorString());
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

    mDocs << doc;
    return doc;
}


/************************************************

 ************************************************/
bool PdfMerger::run(const QString &outFileName)
{
    QDir().mkpath(outFileName + "/..");

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

    PdfMergerIPCWriter ipc;
    ipc.writeAllPagesCount(pagesCnt);

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
            PdfMergerPageInfo *pageInfo = new PdfMergerPageInfo();

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

            pageInfo->rotate = page->getRotate();

            pageInfo->pageNum = i;
            if (pageDict->hasKey((char *)"Contents"))
            {
                pageDict->lookup((char *)"Contents", &(pageInfo->stream));
                pageDict->remove((char *)"Contents");
            }

            mOrigPages[n] = pageInfo;

            doc->markPageObjects(pageDict, &mXRef, &countXref, numOffset);

            if (n % 2)
                ipc.writeProgressStatus((n + 1) / 2);

            n++;
        }

        POPPLER_WritePageObjects(doc, mStream, &mXRef, numOffset);
        numOffset = mXRef.getNumObjects() + 1;
    }

    int xformStartNum = mXRef.getNumObjects();
    //*mStream  << "\n% XObjects **************************************\n\n";
    for (int i=0; i<mOrigPages.count(); ++i)
    {
        PdfMergerPageInfo *pageInfo = mOrigPages[i];
        pageInfo->objNum = xformStartNum + i;
        writePageAsXObject(pageInfo);
        if (i % 2)
            ipc.writeProgressStatus((pagesCnt + i ) / 2);
    }

    mXrefPos = mStream->getPos();

    writeTrailer(&mXRef, rootNum, mStream);

    //*mStream  << "\n% End update **************************************\n\n";

    mStream->close();
    delete mStream;
    mStream = 0;
    fclose(f);

    int i = 0;
    for (int docNum=0; docNum<mDocs.count(); ++docNum)
    {
        PDFDoc *doc = mDocs.at(docNum);

        int pc = doc->getNumPages();
        for (int pageNum=0; pageNum<pc; ++pageNum)
        {
            PdfMergerPageInfo *pageInfo = mOrigPages.at(i);
            ipc.writePageInfo(docNum, pageNum, *pageInfo);
            i++;
        }
    }

    ipc.writeXRefInfo(mXrefPos, mXRef.getNumObjects());
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
bool PdfMerger::writePageAsXObject(PdfMergerPageInfo *pageInfo)
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
    writeDictValue(pageDict, "Resources",     pageInfo->numOffset);
    writeDictValue(pageDict, "Metadata",      pageInfo->numOffset);
    writeDictValue(pageDict, "PieceInfo",     pageInfo->numOffset);
    writeDictValue(pageDict, "LastModified",  pageInfo->numOffset);
    writeDictValue(pageDict, "StructParents", pageInfo->numOffset);
    // ..........................................


    Stream *stream = 0;

    if (pageInfo->stream.isStream())
    {
        stream = pageInfo->stream.getStream();
    }
    else if (pageInfo->stream.isArray())
    {
        Array *array = pageInfo->stream.getArray();
        if (array->getLength() == 1)
        {
            Object o;
            array->get(0, &o);
            if (o.isStream())
                stream = o.getStream();
            else
                warning("Page content is array with incorrect item type:\n" + pageInfo->dump() + "\n");
        }
        else
        {
            warning("Page content is array with incorrect length:\n" +pageInfo->dump() + "\n");
        }
    }
    else
    {
        warning("Page has incorrect content type:\n" + pageInfo->dump() + "\n");
    }


    if (stream)
    {
        // Copy dict from the stream object .........
        Dict *dict = stream->getDict();
        for (int i=0; i<dict->getLength(); ++i)
        {
            Object value;
            dict->getVal(i, &value);
            *mStream << "/" << dict->getKey(i) << " ";
            POPPLER_WriteObject(&value, 0, mStream, &mXRef, pageInfo->numOffset);
            *mStream << "\n";
            value.free();
        }
        *mStream << " >>\n";

        // Write stream .............................
        *mStream << "stream\n";
        *mStream << *stream;
        *mStream << "endstream\n";
    }
    else
    {
        *mStream << "/Length 0\n";
        *mStream << " >>\n";

        *mStream << "stream\n";
        *mStream << "endstream\n";
    }

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
