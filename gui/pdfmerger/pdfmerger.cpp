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

#include "assert.h"
#include "pdfmerger.h"
#include "pdfparser/pdfobject.h"
#include "pdfparser/pdfwriter.h"
#include <QFile>
#include <QDebug>

using namespace PDF;


/************************************************
 *
 ************************************************/
PdfMerger::PdfMerger(QObject *parent):
    QObject(parent),
    mReader(nullptr),
    mWriter(new Writer()),
    mObjNumOffset(0)
{

}


/************************************************
 *
 ************************************************/
PdfMerger::~PdfMerger()
{
    delete mWriter;
}


/************************************************
 *
 ************************************************/
void PdfMerger::addSourceFile(const QString &fileName, qint64 startPos, qint64 endPos)
{
    mSources << PdfMerger::SourceFile{fileName, startPos, endPos};
}


/************************************************
 *
 ************************************************/
void PdfMerger::run(const QString &outFileName)
{
    QFile file(outFileName);
    if (! file.open(QFile::WriteOnly | QFile::Truncate))
    {
        throw tr("I can't write file \"%1\"").arg(file.fileName()) + "\n" + file.errorString();
    }

    // QFile destructor close the file.
    run(&file);
}


/************************************************
 *
 ************************************************/
void PdfMerger::run(QIODevice *outDevice)
{
    mWriter->setDevice(outDevice);
    mWriter->writePDFHeader(1,7);

    int rootNum = 1;
    int pagesNum = rootNum +1;

    // Catalog object ...........................
    {
        PDF::Object obj;
        obj.setObjNum(rootNum);
        obj.setGenNum(0);

        obj.setValue(Dict());
        obj.dict().insert("Type",  Name("Catalog"));
        obj.dict().insert("Pages", Number(pagesNum));
        mWriter->writeObject(obj);
    }
    // ..........................................


    // Pages object .............................
    {
        PDF::Object obj;
        obj.setObjNum(pagesNum);
        obj.setGenNum(0);
        obj.setValue(Dict());
        obj.dict().insert("Type",  Name("Pages"));
        obj.dict().insert("Count", Number(0));
        obj.dict().insert("Kids",  Array());
        mWriter->writeObject(obj);
    }
    // ..........................................

    mObjNumOffset = mWriter->xRefTable().count();

    foreach (const SourceFile &source, mSources)
    {
        mSkippedObjects.clear();
        mWriter->writeComment(" Document **************************************");

        QFile file(source.fileName);
        if(!file.open(QFile::ReadOnly))
            throw tr("I can't open file \"%1\"").arg(source.fileName) + "\n" + file.errorString();

        int start = source.startPos;
        int end   = source.endPos ? source.endPos : file.size();

        if (end < start)
            throw QString("Invalid request for %1, the start position (%2) is greater than the end (%3) one.")
                .arg(source.fileName)
                .arg(source.startPos)
                .arg(source.endPos);

        file.seek(start);

        uchar *buf = file.map(start, end - start);
        Reader reader(reinterpret_cast<const char*>(buf), end - start);
        reader.open();
        mReader = &reader;

        PDF::Object catalog = reader.getObject(reader.trailerDict().value("Root").toLink());
        mSkippedObjects << catalog.objNum();
        PDF::Object pages   = reader.getObject(catalog.dict().value("Pages").toLink());

        mWriter->writeComment("-- Start XObjects ------------------------------------");
        PDF::Dict dict;
        walkPageTree(0, pages, dict);
        mWriter->writeComment("-- End XObjects --------------------------------------");

        mWriter->writeComment("-- Start copied objects ------------------------------");
        foreach (const PDF::XRefEntry &xref, mReader->xRefTable())
        {
            if (xref.type == PDF::XRefEntry::Type::Free)
                continue;

            if (mSkippedObjects.contains(xref.objNum))
                continue;
            PDF::Object obj = mReader->getObject(xref.objNum, xref.genNum);
            mWriter->writeComment(QString("%%%%%% copied from [%1 %2]").arg(xref.objNum).arg(xref.genNum));
            mWriter->writeObject(addOffset(obj, mObjNumOffset));

        }
        mWriter->writeComment("-- End copied objects --------------------------------");

        mObjNumOffset = mWriter->xRefTable().maxObjNum();
        file.unmap(buf);
    }

    mWriter->writeXrefTable();
    mWriter->writeTrailer(Link(1,0));
}


/************************************************
 *
 ************************************************/
void PdfMerger::emitError(const QString &message)
{
    qWarning() << message;
    emit error(message);
}


/************************************************
 *
 ************************************************/
int PdfMerger::walkPageTree(int pageNum, const PDF::Object &page, const PDF::Dict &inherited)
{
    assert(page.type() == "Pages" || page.type() == "Page");

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

        const Array &kids = pageDict.value("Kids").toArray();
        for (int i=0; i<kids.count(); ++i)
        {
            pageNum = walkPageTree(pageNum, mReader->getObject(kids.at(i).toLink()), dict);
        }
        return pageNum;
    }

    if (page.type() == "Page")
    {
        mSkippedObjects << page.objNum();
        this->writePageAsXObject(page, inherited);
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
void PdfMerger::writePageAsXObject(const PDF::Object &page, const Dict &inherited)
{
    const PDF::Object &contents = mReader->getObject(page.dict().value("Contents").toLink());
    mSkippedObjects << contents.objNum();

    PDF::Object xObj;
    xObj.setObjNum(page.objNum());
    xObj.setGenNum(page.genNum());

    xObj.setValue(contents.dict());
    PDF::Dict &dict = xObj.dict();

    dict.insert("Type",     Name("XObject"));
    dict.insert("Subtype",  Name("Form"));
    dict.insert("FormType", Number(1));

    const PDF::Dict &pageDict = page.dict();
    dict.insert("Resources", pageDict.value("Resources", inherited.value("Resources")));

    foreach (auto &key, QStringList() << "CropBox" << "MediaBox")
    {
        if (pageDict.contains(key))
        {
            dict.insert("BBox", pageDict.value(key));
            break;
        }

        if (inherited.contains(key))
        {
            dict.insert("BBox", inherited.value(key));
            break;
        }
    }

    foreach (auto &key, QStringList() << "Metadata" << "PieceInfo" << "LastModified" << "StructParents")
    {
        if (pageDict.contains(key))
            dict.insert(key, pageDict.value(key));
    }

    xObj.setStream(contents.stream());

    mWriter->writeObject(addOffset(xObj, mObjNumOffset));
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
Object &PdfMerger::addOffset(PDF::Object &obj, quint32 offset)
{
    if (offset == 0)
        return obj;

    obj.setObjNum(obj.objNum() + offset);
    offsetValue(obj.value(), offset);

    return obj;
}
