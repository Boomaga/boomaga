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

#include "assert.h"
#include "pdfprocessor.h"
#include <QFile>
#include "pdfparser/pdfobject.h"
#include "pdfparser/pdfvalue.h"
#include <pdfparser/pdfreader.h>
#include <pdfparser/pdfwriter.h>


/************************************************
 *
 ************************************************/
PdfProcessor::PdfProcessor(const QString &fileName, qint64 startPos, qint64 endPos):
    mStartPos(startPos),
    mEndPos(endPos),
    mFile(fileName),
    mReader(nullptr),
    mBuf(nullptr),
    mObjNumOffset(0),
    mWriter(nullptr)
{

}


/************************************************
 *
 ************************************************/
PdfProcessor::~PdfProcessor()
{
    if (mBuf)
        mFile.unmap(mBuf);

    delete mReader;
}


/************************************************
 *
 ************************************************/
void PdfProcessor::open()
{
    if(!mFile.open(QFile::ReadOnly))
        throw QObject::tr("I can't open file \"%1\"").arg(mFileName) + "\n" + mFile.errorString();

    int start = mStartPos;
    int end   = mEndPos ? mEndPos : mFile.size();

    if (end < start)
        throw QString("Invalid request for %1, the start position (%2) is greater than the end (%3) one.")
            .arg(mFileName)
            .arg(mStartPos)
            .arg(mEndPos);

    mFile.seek(start);

    mBuf = mFile.map(start, end - start);
    mReader = new PDF::Reader(reinterpret_cast<const char*>(mBuf), end - start);
    mReader->open();
}


/************************************************
 *
 ************************************************/
quint32 PdfProcessor::pageCount()
{
    return mReader->find("/Root/Pages/Count").toNumber().value();
}


/************************************************
 *
 ************************************************/
void PdfProcessor::run(PDF::Writer *writer, quint32 objNumOffset)
{
    mWriter = writer;
    mObjNumOffset = objNumOffset;
    mSkippedObjects.clear();

    PDF::Object catalog = mReader->getObject(mReader->trailerDict().value("Root").toLink());
    mSkippedObjects << catalog.objNum();
    PDF::Object pages   = mReader->getObject(catalog.dict().value("Pages").toLink());

    mPageInfo.reserve(pages.dict().value("Count").toNumber().value());

    PDF::Dict dict;
    walkPageTree(0, pages, dict);

    foreach (const PDF::XRefEntry &xref, mReader->xRefTable())
    {
        if (xref.type == PDF::XRefEntry::Type::Free)
            continue;

        if (mSkippedObjects.contains(xref.objNum))
            continue;

        PDF::Object obj = mReader->getObject(xref.objNum, xref.genNum);
        mWriter->writeObject(addOffset(obj, mObjNumOffset));

    }

    writer = nullptr;
}


/************************************************
 *
 ************************************************/
void fillPageInfo(PdfPageInfo *pageInfo, const PDF::Dict &pageDict, const PDF::Dict &inherited)
{
    const PDF::Array &mediaBox = pageDict.value("MediaBox", inherited.value("MediaBox")).toArray();
    if (mediaBox.count() != 4)
        throw QString("Incorrect MediaBox rectangle");

    pageInfo->mediaBox = QRectF(mediaBox.at(0).toNumber().value(),
                                mediaBox.at(1).toNumber().value(),
                                mediaBox.at(2).toNumber().value() - mediaBox.at(0).toNumber().value(),
                                mediaBox.at(3).toNumber().value() - mediaBox.at(1).toNumber().value());

    const PDF::Array &cropBox  = pageDict.value("CropBox", inherited.value("CropBox")).toArray();
    if (cropBox.isValid())
    {
        if (cropBox.count() != 4)
            throw QString("Incorrect CropBox rectangle");

        pageInfo->cropBox = QRectF(cropBox.at(0).toNumber().value(),
                                   cropBox.at(1).toNumber().value(),
                                   cropBox.at(2).toNumber().value() - cropBox.at(0).toNumber().value(),
                                   cropBox.at(3).toNumber().value() - cropBox.at(1).toNumber().value());
    }
    else
    {
        pageInfo->cropBox = pageInfo->mediaBox;
    }
    pageInfo->rotate = pageDict.value("Rotate", inherited.value("Rotate")).toNumber().value();

}


/************************************************
 *
 ************************************************/
int PdfProcessor::walkPageTree(int pageNum, const PDF::Object &page, const PDF::Dict &inherited)
{
    assert(page.type() == "Pages" || page.type() == "Page");

    // Type = Pages .......................................
    if (page.type() == "Pages")
    {
        mSkippedObjects << page.objNum();
        const PDF::Dict &pageDict = page.dict();
        PDF::Dict dict = inherited;
        if (pageDict.contains("Resources"))
            dict.insert("Resources", pageDict.value("Resources"));

        if (pageDict.contains("MediaBox"))
            dict.insert("MediaBox",  pageDict.value("MediaBox"));

        if (pageDict.contains("CropBox"))
            dict.insert("CropBox",   pageDict.value("CropBox"));

        if (pageDict.contains("Rotate"))
            dict.insert("Rotate",    pageDict.value("Rotate"));

        const PDF::Array &kids = pageDict.value("Kids").toArray();
        for (int i=0; i<kids.count(); ++i)
        {
            pageNum = walkPageTree(pageNum, mReader->getObject(kids.at(i).toLink()), dict);
        }
        return pageNum;
    }

    // Type = Page ........................................
    if (page.type() == "Page")
    {
        mSkippedObjects << page.objNum();

        PdfPageInfo pageInfo;
        try
        {
            fillPageInfo(&pageInfo, page.dict(), inherited);
        }
        catch (const QString &err)
        {
            throw QString("Error on page %1 %2: %3").arg(page.objNum()).arg(page.genNum()).arg(err);
        }

        const PDF::Value &contents = page.dict().value("Contents");
        switch (contents.type())
        {
        case PDF::Value::Type::Link:
            pageInfo.xObjNums << writeContentAsXObject(contents.toLink(), page.dict(), inherited);
            break;

        case PDF::Value::Type::Array:
            for (int i=0; i<contents.toArray().count(); ++i)
                pageInfo.xObjNums << writeContentAsXObject(contents.toArray().at(i).toLink(), page.dict(), inherited);
            break;

        default:
            throw QString("Page %1 %2 has incorrect content type.").arg(page.objNum()).arg(page.genNum());
        }

        mPageInfo << pageInfo;

        return pageNum + 1;
    }

    return pageNum;
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


  Page Boundaries
  ================

  MediaBox (required, inheritable)
    The media box defines the boundaries of the physical medium on which the
    page is to be printed. It may include any extended area surrounding the
    finished page for bleed, printing marks, or other such purposes. It may also
    include areas close to the edges of the medium that cannot be marked because
    of physical limitations of the output device. Content falling outside this
    boundary can safely be discarded without affecting the meaning of the PDF file.

  CropBox (optional, inheritable, default=MediaBox)
    The crop box defines the region to which the contents of the page are to be
    clipped (cropped) when displayed or printed. Unlike the other boxes, the crop
    box has no defined meaning in terms of physical page geometry or intended
    use; it merely imposes clipping on the page contents. However, in the absence
    of additional information (such as imposition instructions specified in a JDF or
    PJTF job ticket), the crop box determines how the page’s contents are to be po-
    sitioned on the output medium. The default value is the page’s media box.

  BleedBox (optional, default=CropBox)
    The bleed box defines the region to which the contents of the page
    should be clipped when output in a production environment. This may include
    any extra bleed area needed to accommodate the physical limitations of cut-
    ting, folding, and trimming equipment. The actual printed page may include
    printing marks that fall outside the bleed box. The default value is the page’s
    crop box.

  TrimBox (optional, defaul=CropBox)
    The trim box (PDF 1.3) defines the intended dimensions of the finished page
    after trimming. It may be smaller than the media box to allow for production-
    related content, such as printing instructions, cut marks, or color bars.
    The default value is the page’s crop box.

  ArtBox (optional, default=CropBox)
    The art box defines the extent of the page’s meaningful content
    (including potential white space) as intended by the page’s creator. The default
    value is the page’s crop box.
 ************************************************/
PDF::ObjNum PdfProcessor::writeContentAsXObject(const PDF::Link &contentLink, const PDF::Dict &pageDict, const PDF::Dict &inherited)
{
    if (!contentLink.isValid())
        throw "Page has incorrect content type";

    mSkippedObjects << contentLink.objNum();
    const PDF::Object content = mReader->getObject(contentLink);
    PDF::Object xObj;
    xObj.setObjNum(content.objNum());
    xObj.setGenNum(content.genNum());
    xObj.setValue(content.dict());

    xObj.dict().insert("Type",     PDF::Name("XObject"));
    xObj.dict().insert("Subtype",  PDF::Name("Form"));
    xObj.dict().insert("FormType", PDF::Number(1));

    xObj.dict().insert("Resources", pageDict.value("Resources", inherited.value("Resources")));
    xObj.dict().insert("BBox", pageDict.value("CropBox",  inherited.value("CropBox",
                               pageDict.value("MediaBox", inherited.value("MediaBox")))));

    if (pageDict.contains("Metadata"))      xObj.dict().insert("Metadata",      pageDict.value("Metadata"));
    if (pageDict.contains("PieceInfo"))     xObj.dict().insert("PieceInfo",     pageDict.value("PieceInfo"));
    if (pageDict.contains("LastModified"))  xObj.dict().insert("LastModified",  pageDict.value("LastModified"));
    if (pageDict.contains("StructParents")) xObj.dict().insert("StructParents", pageDict.value("StructParents"));

    xObj.setStream(content.stream());
    mWriter->writeObject(addOffset(xObj, mObjNumOffset));
    return xObj.objNum();
}


/************************************************
 *
 ************************************************/
void offsetValue(PDF::Value &value, const quint32 offset)
{
    if (value.isLink())
    {
        PDF::Link &link = value.toLink();
        link.setObjNum(link.objNum() + offset);
    }

    if (value.isArray())
    {
        PDF::Array &arr = value.toArray();

        for (int i=0; i<arr.count(); ++i)
        {
            offsetValue(arr[i], offset);
        }
    }

    if (value.isDict())
    {
        PDF::Dict &dict = value.toDict();

        foreach (auto &key, dict.keys())
        {
            offsetValue(dict[key], offset);
        }
    }
}


/************************************************
 *
 ************************************************/
PDF::Object &PdfProcessor::addOffset(PDF::Object &obj, quint32 offset)
{
    if (offset == 0)
        return obj;

    obj.setObjNum(obj.objNum() + offset);
    offsetValue(obj.value(), offset);

    return obj;
}
