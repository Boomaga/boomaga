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


#include <cstdlib>
#include <assert.h>

#include "pdfmerger.h"
#include "pdfprocessor.h"
#include "pdfparser/pdfobject.h"
#include "pdfparser/pdfwriter.h"
#include "pdfmergeripc.h"
#include <QFile>
#include <QDebug>

using namespace PDF;


/************************************************
 *
 ************************************************/
PdfMerger::PdfMerger(QObject *parent):
    QObject(parent)
{

}


/************************************************
 *
 ************************************************/
PdfMerger::~PdfMerger()
{
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
    PDF::Writer writer(outDevice);
    writer.writePDFHeader(1,7);

    PdfMergerIPCWriter ipc;

    QVector<PdfProcessor*> processors;
    quint32 pagesCnt = 0;
    foreach (const SourceFile &source, mSources)
    {
        PdfProcessor *proc = new PdfProcessor(source.fileName, source.startPos, source.endPos);
        proc->open();
        pagesCnt += proc->pageCount();
        processors << proc;
    }
    ipc.writeAllPagesCount(pagesCnt);

    QVector<PdfPageInfo> pages;
    int docNum = 0;
    foreach (PdfProcessor *proc, processors)
    {
        connect(proc, SIGNAL(pageReady()), &ipc, SLOT(writeNextProgress()));
        proc->run(&writer, writer.xRefTable().maxObjNum() + 3);

        pages << proc->pageInfo();
        for (int i=0; i< proc->pageInfo().count(); ++i)
            ipc.writePageInfo(docNum, i, proc->pageInfo().at(i));
        ++docNum;
    }
    qDeleteAll(processors);


    // Catalog object ...........................
    PDF::Object catalog;
    catalog.setObjNum(1);

    catalog.dict().insert("Type",  PDF::Name("Catalog"));
    catalog.dict().insert("Pages", PDF::Link(catalog.objNum() + 1));
    writer.writeObject(catalog);
    // ..........................................

    // Pages object .............................
    if (!std::getenv("BOOMAGAMERGER_DEBUGPAGES"))
    {
        PDF::Object pagesObj(2);
        pagesObj.dict().insert("Type",  Name("Pages"));
        pagesObj.dict().insert("Count", Number(0));
        pagesObj.dict().insert("Kids",  PDF::Array());
        writer.writeObject(pagesObj);
    }
    else
    {
        PDF::Object pagesObj(2);
        pagesObj.dict().insert("Type",  Name("Pages"));

        PDF::ObjNum pageNum = writer.xRefTable().maxObjNum() + 1;
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
                writer.writeObject(page);
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
                writer.writeObject(xobj);
            }

            {
                QString stream;
                for (int c=0; c<pi.xObjNums.count(); ++c)
                    stream += QString("/Im0_%1 Do ").arg(c);

                content.setStream(stream.toLatin1());
                content.dict().insert("Length", PDF::Number(content.stream().length()));
                writer.writeObject(content);
            }
        }

        pagesObj.dict().insert("Count", Number(kids.count()));
        pagesObj.dict().insert("Kids",  kids);
        writer.writeObject(pagesObj);
    }
    // ..........................................

    ipc.writeXRefInfo(outDevice->pos(), writer.xRefTable().maxObjNum() + 1);
    writer.writeXrefTable();
    writer.writeTrailer(Link(catalog.objNum()));
}
