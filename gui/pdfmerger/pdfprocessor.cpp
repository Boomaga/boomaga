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
    QObject(),
    mFileName(fileName),
    mStartPos(startPos),
    mEndPos(endPos),
    mObjNumOffset(0),
    mWriter(nullptr)
{

}


/************************************************
 *
 ************************************************/
PdfProcessor::~PdfProcessor()
{
}


/************************************************
 *
 ************************************************/
void PdfProcessor::open()
{
    mReader.open(mFileName, mStartPos, mEndPos);
}


/************************************************
 *
 ************************************************/
quint32 PdfProcessor::pageCount()
{
    return mReader.pageCount();
}


/************************************************
 *
 ************************************************/
void PdfProcessor::run(PDF::Writer *writer, quint32 objNumOffset)
{
    mWriter = writer;
    mObjNumOffset = objNumOffset;

    PDF::Object catalog = mReader.getObject(mReader.trailerDict().value("Root").asLink());
    PDF::Object pages   = mReader.getObject(catalog.dict().value("Pages").asLink());

    mPageInfo.reserve(pages.dict().value("Count").asNumber().value());

    PDF::Dict dict;
    walkPageTree(0, pages, dict);

    writer = nullptr;
}


/************************************************
 *
 ************************************************/
void fillPageInfo(PdfPageInfo *pageInfo, const PDF::Dict &pageDict, const PDF::Dict &inherited)
{
    const PDF::Array &mediaBox = pageDict.value("MediaBox", inherited.value("MediaBox")).asArray();
    if (mediaBox.count() != 4)
        throw QString("Incorrect MediaBox rectangle");

    pageInfo->mediaBox = QRectF(mediaBox.at(0).asNumber().value(),
                                mediaBox.at(1).asNumber().value(),
                                mediaBox.at(2).asNumber().value() - mediaBox.at(0).asNumber().value(),
                                mediaBox.at(3).asNumber().value() - mediaBox.at(1).asNumber().value());

    const PDF::Array &cropBox  = pageDict.value("CropBox", inherited.value("CropBox")).asArray();
    if (cropBox.isValid())
    {
        if (cropBox.count() != 4)
            throw QString("Incorrect CropBox rectangle");

        pageInfo->cropBox = QRectF(cropBox.at(0).asNumber().value(),
                                   cropBox.at(1).asNumber().value(),
                                   cropBox.at(2).asNumber().value() - cropBox.at(0).asNumber().value(),
                                   cropBox.at(3).asNumber().value() - cropBox.at(1).asNumber().value());
    }
    else
    {
        pageInfo->cropBox = pageInfo->mediaBox;
    }
    pageInfo->rotate = pageDict.value("Rotate", inherited.value("Rotate")).asNumber().value();

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

        const PDF::Array &kids = pageDict.value("Kids").asArray();
        for (int i=0; i<kids.count(); ++i)
        {
            pageNum = walkPageTree(pageNum, mReader.getObject(kids.at(i).asLink()), dict);
        }
        return pageNum;
    }

    // Type = Page ........................................
    if (page.type() == "Page")
    {
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
            pageInfo.xObjNums << writeContentAsXObject(contents.asLink(), page.dict(), inherited);
            break;

        case PDF::Value::Type::Array:
            for (int i=0; i<contents.asArray().count(); ++i)
                pageInfo.xObjNums << writeContentAsXObject(contents.asArray().at(i).asLink(), page.dict(), inherited);
            break;

        default:
            throw QString("Page %1 %2 has incorrect content type.").arg(page.objNum()).arg(page.genNum());
        }

        mPageInfo << pageInfo;
        emit pageReady();

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

    const PDF::Object content = mReader.getObject(contentLink);
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
    addOffset(xObj);
    return xObj.objNum();
}


/************************************************
 *
 ************************************************/
PDF::Object &PdfProcessor::addOffset(PDF::Object &obj)
{
    obj.setObjNum(obj.objNum() + mObjNumOffset);
    offsetValue(obj.value());
    mWriter->writeObject(obj);
    return obj;
}


/************************************************
 *
 ************************************************/
void PdfProcessor::offsetValue(PDF::Value &value)
{
    if (value.isLink())
    {
        PDF::Link &link = value.asLink();
        if (!mProcessedObjects.contains(link.objNum()))
        {
            mProcessedObjects << link.objNum();
            PDF::Object obj = mReader.getObject(link);
            addOffset(obj);
        }
        link.setObjNum(link.objNum() + mObjNumOffset);
    }

    if (value.isArray())
    {
        PDF::Array &arr = value.asArray();

        for (int i=0; i<arr.count(); ++i)
        {
            offsetValue(arr[i]);
        }
    }

    if (value.isDict())
    {
        PDF::Dict &dict = value.asDict();

        foreach (auto &key, dict.keys())
        {
            offsetValue(dict[key]);
        }
    }
}
