/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2013 Boomaga team https://github.com/Boomaga
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

#include "tmppdffile.h"
#include <QCryptographicHash>
#include <QDir>
#include <cmath>
#include <QDateTime>

#include "sheet.h"
#include "layout.h"
#include "pdfprocessor.h"
#include "pdfparser/pdfwriter.h"
#include "pdfparser/pdfobject.h"
#include "pdfparser/pdfxref.h"
#include "projectpage.h"
#include "printer.h"
#include "project.h"
#include "../debug.h"


/************************************************

 ************************************************/
QIODevice &operator<<(QIODevice &out, const QString &str)
{
    out.write(str.toLocal8Bit());
    return out;
}


/************************************************

 ************************************************/
QIODevice &operator<<(QIODevice &out, const int value)
{
    out.write(QString("%1").arg(value).toLatin1());
    return out;
}

struct JobInfo
{
    quint64 xrefPos = 0;
    quint64 endPos  = 0;

    PDF::XRefTable xref;
};

class TmpPdfFile::Data
{
public:
    Data()
    {
    }

    quint64 startPos = 0;
    QVector<JobInfo> mJobInfoList;
};


/************************************************

 ************************************************/
TmpPdfFile::TmpPdfFile(QObject *parent):
    QObject(parent),
    mData(new Data),
    mFileName(genTmpFileName()),
    mFile(mFileName)
{
    mValid = mFile.open(QFile::WriteOnly | QFile::Truncate);

    if (!mValid){
        mErrorString = tr("I can't write file \"%1\"")
                .arg(mFile.fileName()) + ":" +
                mFile.errorString();

        return;
    }


    PDF::Writer writer(&mFile);
    writer.writePDFHeader();
    mData->startPos = mFile.pos();
}


/************************************************

 ************************************************/
TmpPdfFile::~TmpPdfFile()
{
}


/************************************************
 *
 ************************************************/
JobList TmpPdfFile::add(const JobList &jobs)
{
    try {
        JobList res;
        QVector<PdfPageInfo> pages;

        PDF::ObjNum offset = 0;
        quint64 startPos = mData->startPos;
        if (!mData->mJobInfoList.isEmpty()) {
            const JobInfo &ji = mData->mJobInfoList.last();

            startPos = ji.endPos;
            offset += ji.xref.maxObjNum();
        }

        mFile.seek(startPos);
        for (const Job &job: jobs) {

            PDF::Writer writer(&mFile);

            PdfProcessor proc(job.fileName());
            proc.open();
            proc.run(&writer, offset);

            for (quint32 i=0; i<job.pageCount(); ++i) {
                int p = job.page(i)->jobPageNum();
                if (p>=0 && p<proc.pageCount()) {
                    job.page(i)->setPdfInfo(proc.pageInfo().at(p));
                }
            }
            res << job;

            quint64 xrefPos = writeCatalog(&writer, proc.pageInfo());
            JobInfo info;
            info.xref    = writer.xRefTable();
            info.xrefPos = xrefPos;
            info.endPos  = mFile.pos();
            mData->mJobInfoList.append(info);

            pages << proc.pageInfo();
            offset = writer.xRefTable().maxObjNum();
            mFirstFreeNum = writer.xRefTable().maxObjNum() + 1;
            mOrigXrefPos  = xrefPos;
        }

        mOrigFileSize = mFile.pos();
        mFile.resize(mFile.pos());
        mFile.flush();
        return res;
    }
    catch (PDF::Error &err) {
        mValid = false;
        mErrorString = err.what();
        return JobList();
    }
}


/************************************************
 *
 ************************************************/
quint64 TmpPdfFile::writeCatalog(PDF::Writer *writer, const QVector<PdfPageInfo> &pages)
{
    PDF::ObjNum catalogObjNum = writer->xRefTable().maxObjNum() + 1;
    PDF::ObjNum pagesObjNum   = catalogObjNum + 1;


    // Catalog object ...........................
    PDF::Object catalog;
    catalog.setObjNum(catalogObjNum);

    catalog.dict().insert("Type",  PDF::Name("Catalog"));
    catalog.dict().insert("Pages", PDF::Link(pagesObjNum));
    writer->writeObject(catalog);
    // ..........................................

    // Pages object .............................
    if (!std::getenv("BOOMAGAMERGER_DEBUGPAGES"))
    {
        PDF::Object pagesObj(pagesObjNum);
        pagesObj.dict().insert("Type",  PDF::Name("Pages"));
        pagesObj.dict().insert("Count", PDF::Number(0));
        pagesObj.dict().insert("Kids",  PDF::Array());
        writer->writeObject(pagesObj);
    }
    else
    {
        PDF::Object pagesObj(pagesObjNum);
        pagesObj.dict().insert("Type",  PDF::Name("Pages"));

        PDF::ObjNum pageNum = writer->xRefTable().maxObjNum() + 1;
        PDF::Array kids;
        for (int i=0; i< pages.count(); ++i)
        {
            PdfPageInfo pi = pages.at(i);

            PDF::Object page(   pageNum + i * 3 + 1);
            PDF::Object xobj(   pageNum + i * 3 + 2);
            PDF::Object content(pageNum + i * 3 + 3);
            kids.append(PDF::Link(page.objNum()));

            {
                PDF::Dict dict;
                dict.insert("Type",      PDF::Name("Page"));
                dict.insert("Parent",    PDF::Link(pagesObj.objNum(), 0));
                dict.insert("Resources", PDF::Link(xobj.objNum()));

                PDF::Array mediaBox;
                mediaBox.append(PDF::Number(pi.mediaBox.left()));
                mediaBox.append(PDF::Number(pi.mediaBox.top()));
                mediaBox.append(PDF::Number(pi.mediaBox.width()));
                mediaBox.append(PDF::Number(pi.mediaBox.height()));
                dict.insert("MediaBox",  mediaBox);

                PDF::Array cropBox;
                cropBox.append(PDF::Number(pi.mediaBox.left()));
                cropBox.append(PDF::Number(pi.mediaBox.top()));
                cropBox.append(PDF::Number(pi.mediaBox.width()));
                cropBox.append(PDF::Number(pi.mediaBox.height()));
                dict.insert("CropBox",   cropBox);

                dict.insert("Rotate",    PDF::Number(pi.rotate));
                dict.insert("Contents",  PDF::Link(content.objNum()));
                page.setValue(dict);
                writer->writeObject(page);
            }

            {
                xobj.dict().insert("ProcSet", PDF::Array() << PDF::Name("PDF"));
                xobj.dict().insert("XObject", PDF::Dict());
                PDF::Dict dict;
                for (int c=0; c<pi.xObjNums.count(); ++c)
                {
                    dict.insert(QString("Im0_%1").arg(c),
                                PDF::Link(pi.xObjNums.at(c)));
                }

                xobj.dict().insert("XObject", dict);
                writer->writeObject(xobj);
            }

            {
                QString stream;
                for (int c=0; c<pi.xObjNums.count(); ++c)
                    stream += QString("/Im0_%1 Do ").arg(c);

                content.setStream(stream.toLatin1());
                content.dict().insert("Length", PDF::Number(content.stream().length()));
                writer->writeObject(content);
            }
        }

        pagesObj.dict().insert("Count", PDF::Number(kids.count()));
        pagesObj.dict().insert("Kids",  kids);
        writer->writeObject(pagesObj);
    }
    // ..........................................

    quint64 xrefPos = writer->device()->pos();
    writer->writeXrefTable();
    quint64 prev = mData->mJobInfoList.empty() ? 0 : mData->mJobInfoList.last().xrefPos;
    writer->writeTrailer(PDF::Link(catalogObjNum), PDF::Link(), prev);
    return xrefPos;
}



/************************************************

 ************************************************/
void TmpPdfFile::updateSheets(const QList<Sheet *> &sheets)
{
    if (mValid)
    {
        QFile file(mFileName);
        if (!file.open(QFile::ReadWrite))
        {
            project->error(tr("I can't create temporary file \"%1\"")
                           .arg(mFileName));
            return;
        }
        file.seek(mOrigFileSize);

        writeSheets(&file, sheets);

        file.resize(file.pos());
        file.close();
   }
}


/************************************************

 ************************************************/
bool TmpPdfFile::writeDocument(const QList<Sheet*> &sheets, QIODevice *out)
{
    QFile f(mFileName);
    if (!f.open(QFile::ReadOnly))
        return project->error(tr("I can't read file '%1'").arg(mFileName) + "\n" + out->errorString());


    qint64 bufLen = qMin(mOrigFileSize - f.pos(), (qint64)(1024 * 1024));
    while (bufLen > 0)
    {
        int wrote = out->write(f.read(bufLen));
        if (wrote<0)
            return project->error(tr("I can't write to file '%1'").arg(mFileName) + "\n" + out->errorString());

        bufLen = qMin(mOrigFileSize - f.pos(), (qint64)(1024 * 1024));
    }

    writeSheets(out, sheets);
    return true;
}


/************************************************

 ************************************************/
void TmpPdfFile::writeSheets(QIODevice *out, const QList<Sheet *> &sheets) const
{
    assert(!mData->mJobInfoList.isEmpty());

    qint32 rootNum = mFirstFreeNum;
    qint32 metaDataNum = rootNum + 1;
    qint32 pagesNum = metaDataNum + 1;

    QMap<int, qint64> xref;
    QStringList pagesKids;

    // Catalog object ...........................
    xref.insert(rootNum, out->pos());
    *out << rootNum << " 0 obj\n";
    *out << "<<\n";
    *out << "/Type /Catalog\n";
    //*out << "/Metadata " << metaDataNum << " 0 R\n";
    *out << "/Pages " << pagesNum << " 0 R\n";
    *out << ">>\n";
    *out << "endobj\n";
    // ..........................................

    // Page objects .............................
    qint32 num = pagesNum + 1;
    foreach(const Sheet *sheet, sheets)
    {
        int pageNum      = num;
        int resourcesNum = num + 1;
        int contentsNum  = num + 2;
        num += 3;


        // Page ............................
        xref.insert(pageNum, out->pos());
        *out << pageNum << " 0 obj\n";
        *out << "<<\n";
        *out << "/Type /Page\n";
        *out << "/Contents "  << contentsNum  << " 0 R\n";
        *out << "/Resources " << resourcesNum << " 0 R\n";
        *out << "/Parent " << pagesNum << " 0 R\n";

        *out << "/Rotate " << (int)sheet->rotation() << "\n";
        *out << ">>\n";
        *out << "endobj\n";

        pagesKids << QString("%1 0 R").arg(pageNum);
        //..................................


        // Resources .......................
        xref.insert(resourcesNum, out->pos());
        *out << resourcesNum << " 0 obj\n";
        *out << "<<\n";
        *out << "/XObject << ";
        for (int i=0; i< sheet->count(); ++i)
        {
            const ProjectPage *page = sheet->page(i);
            if (!page)
                continue;

            for (int j=0; j<page->pdfInfo().xObjNums.count(); ++j)
            {
                *out << "/Im" << i << "_" << j << " " << page->pdfInfo().xObjNums.at(j) <<  " 0 R ";
            }
        }
        *out << ">>\n";

        *out << "/ProcSet [ /PDF ]\n";
        *out << ">>\n";
        *out << "endobj\n";
        //..................................


        // Contents ........................
        QString buf;
        getPageStream(&buf, sheet);

        xref.insert(contentsNum, out->pos());
        *out << contentsNum << " 0 obj\n";
        *out << "<<\n";
        *out << "/Length " << buf.size() << "\n";
        *out << ">>\n";
        *out << "stream\n";
        *out << buf;
        *out << "endstream\n";
        *out << "endobj\n";
        //..................................
    }
    // ..........................................


    // Pages object .............................
    QRectF mediaBox = project->printer()->paperRect();

    xref.insert(pagesNum, out->pos());
    *out << pagesNum << " 0 obj\n";
    *out << "<<\n";
    *out << "/Type /Pages\n";
    *out << QString("/MediaBox [%1 %2 %3 %4]\n")
            .arg(mediaBox.left())
            .arg(mediaBox.top())
            .arg(mediaBox.width())
            .arg(mediaBox.height());

    *out << "/Count " << sheets.count() << "\n";
    *out << "/Kids [ " << pagesKids.join("\n") << " ]\n";
    *out << ">>\n";
    *out << "endobj\n";
    // ..........................................

    // MetaData dictionary ......................
    xref.insert(metaDataNum, out->pos());
    *out << metaDataNum << " 0 obj\n";
    *out << "<<\n";
    out->write(project->metaData().asPDFDict());
    *out << ">>\n";
    *out << "endobj\n";

    /*
    QByteArray metaData = project->metaData().asXMP();
    xref.insert(metaDataNum, out->pos());
    *out << metaDataNum << " 0 obj\n";
    *out << "<<\n";
    *out << "/Length " << metaData.length() << "\n";
    *out << "/Subtype /XML\n";
    *out << "/Type /Metadata\n";
    *out << ">>\n";
    *out << "stream\n";
    out->write(metaData);
    *out << "endstream\n";
    *out << "endobj\n";
    */
    // ..........................................

    // XRef for old objects .....................
    qint64 xrefPos = out->pos();
    *out << "xref\n";
   // ..........................................

    // XRef for new objects .....................
    QMap<int, qint64>::const_iterator i = xref.constBegin();
    *out << rootNum << " " << xref.count() << "\n";
    while (i != xref.constEnd())
    {
        *out << QString("%1").arg(i.value(), 10, 10, QChar('0')) << " 00000 n \n";
        ++i;
    }
    // ..........................................


    // Trailer ..................................
    QString hash = QCryptographicHash::hash(mFileName.toLocal8Bit(), QCryptographicHash::Md5).toHex();
    *out << "trailer\n";
    *out << "<<\n";
    *out << "/Size " << (rootNum + xref.count()) << "\n";
    *out << "/Prev " << mOrigXrefPos << "\n";
    *out << "/Root " << rootNum << " 0 R\n";
    *out << "/Info " << metaDataNum << " 0 R\n";
    *out << QString("/ID [<%1> <%1>]\n").arg(hash);
    *out << ">>\n";

    *out << "startxref\n";
    *out << xrefPos << "\n";
    *out << "%%EOF\n";
    // ..........................................
}


/************************************************

 ************************************************/
void TmpPdfFile::getPageStream(QString *out, const Sheet *sheet) const
{
    Printer * printer = project->printer();

    for(int i=0; i<sheet->count(); ++i)
    {
        const ProjectPage *page = sheet->page(i);

        if (page)
        {

            TransformSpec spec = project->layout()->transformSpec(sheet, i, project->rotation());
            QRectF paperRect = printer->paperRect();

            double dx = 0;
            double dy = 0;

            switch (spec.rotation)
            {
            case NoRotate:
                dx = spec.rect.left();
                dy = paperRect.height() - spec.rect.bottom();
                break;

            case Rotate90:
                dx = spec.rect.left();
                dy = paperRect.height() - spec.rect.top();
                break;

            case Rotate180:
                dx = spec.rect.right();
                dy = paperRect.height() - spec.rect.top();
                break;

            case Rotate270:
                dx = spec.rect.right();
                dy = paperRect.height() - spec.rect.bottom();
                break;
            }


            // Translate ........................
            *out += QString("q\n1 0 0 1 %1 %2 cm\n")
                    .arg(dx, 0, 'f', 3)
                    .arg(dy, 0, 'f', 3);

            // Rotate ...........................
            *out += QString("q\n%1 %2 %3 %4 0 0 cm\n")
                    .arg( cos(- spec.rotation * M_PI / 180), 0, 'f', 3)
                    .arg( sin(- spec.rotation * M_PI / 180), 0, 'f', 3)
                    .arg(-sin(- spec.rotation * M_PI / 180), 0, 'f', 3)
                    .arg( cos(- spec.rotation * M_PI / 180), 0, 'f', 3);

            // Scale ...........................
            *out += QString("q\n%1 0 0 %1 0 0 cm\n")
                    .arg(spec.scale, 0, 'f', 3);

            QRectF rect = page->rect();

            // Translate for page rect(x1,y1) ..
            *out += QString("q\n1 0 0 1 %1 %2 cm\n")
                    .arg(-rect.left(), 0, 'f', 3)
                    .arg(-rect.top(), 0, 'f', 3);


            for (int j=0; j<page->pdfInfo().xObjNums.size(); ++j)
                *out += QString("/Im%1_%2 Do\n").arg(i).arg(j);


            if (printer->drawBorder())
            {
                *out += QString("%1 %2 %3 %4 re\nS\n")
                        .arg(rect.left(),   0, 'f', 3)
                        .arg(rect.top(),    0, 'f', 3)
                        .arg(rect.width(),  0, 'f', 3)
                        .arg(rect.height(), 0, 'f', 3);
            }


            *out += "Q\n";
            *out += "Q\n";
            *out += "Q\n";
            *out += "Q\n";
        }
    }
}
