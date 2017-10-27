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
#include <QtAlgorithms>
#include <QDebug>
#include <QFile>
#include <QCryptographicHash>
#include <QVector>
#include <QApplication>
#include <QDir>
#include <QProcess>
#include <math.h>

#include "project.h"
#include "job.h"
#include "inputfile.h"
#include "sheet.h"
#include "layout.h"
#include "render.h"


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


/************************************************

 ************************************************/
QString TmpPdfFile::genFileName()
{
    static int num = 0;
    num++;

    return QString("%1/.cache/boomaga_tmp_%2-%3.pdf")
            .arg(QDir::homePath())
            .arg(QCoreApplication::applicationPid())
            .arg(num);
}


/************************************************

 ************************************************/
TmpPdfFile::TmpPdfFile(const JobList &jobs, QObject *parent):
    QObject(parent),
    mMerger(0),
    mValid(false)
{
    mOrigFileSize = 0;
    mOrigXrefPos = 0;
    mFirstFreeNum = 0;

    mFileName = genFileName();

    foreach (Job job, jobs)
        mInputFiles << job.inputFile();
}


/************************************************

 ************************************************/
TmpPdfFile::~TmpPdfFile()
{
    delete mMerger;
    QFile::remove(mFileName);
}


/************************************************

 ************************************************/
void TmpPdfFile::merge()
{
    delete mMerger;
    mMerger = new QProcess(this);

    connect(mMerger, SIGNAL(finished(int)),
            this, SLOT(mergerFinished(int)));


    PdfMergerIPCReader *ipc = new PdfMergerIPCReader(mMerger, mMerger);

    connect(ipc, SIGNAL(pageInfo(int,int,PdfPageInfo)),
            this, SLOT(ipcPageInfo(int,int,PdfPageInfo)));

    connect(ipc, SIGNAL(xRefInfo(qint64,qint32)),
            this, SLOT(ipcXRefInfo(qint64,qint32)));

    connect(ipc, SIGNAL(progress(int,int)),
            this, SIGNAL(progress(int,int)), Qt::QueuedConnection);

    connect(ipc, SIGNAL(error(QString)),
            project, SLOT(error(QString)));

    QStringList args;
    foreach (const InputFile file, mInputFiles)
    {
        args << file.fileName();
        args << QString("%1").arg(file.startPos());
        args << QString("%1").arg(file.endPos());
    }

    args << mFileName;

    static QString boomagamerger;

    if (boomagamerger.isNull())
    {
        QStringList dirs;
        dirs << QApplication::applicationDirPath() + "/";
        dirs << NONGUI_DIR  "/";

        foreach (QString dir, dirs)
        {
            if (QFileInfo(dir + "boomagamerger").exists())
            {
                boomagamerger = dir + "boomagamerger";
                break;
            }
        }
    }

    if (boomagamerger.isEmpty())
    {
        project->error(tr("Something is wrong. I can't find boomagamerger program.\nPlease reinstall me."));
        return;
    }

    mMerger->start(boomagamerger, args);

}


/************************************************

 ************************************************/
void TmpPdfFile::updateSheets(QList<Sheet *> &sheets)
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
void TmpPdfFile::stop()
{
    mMerger->kill();
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
    *out << "0 3\n";
    *out << "0000000001 65535 f \n";
    *out << "0000000002 00000 f \n";
    *out << "0000000000 00000 f \n";
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

            double dx = 0;
            double dy = 0;

            switch (spec.rotation)
            {
            case NoRotate:
                dx = spec.rect.left();
                dy = printer->paperRect().height() - spec.rect.bottom();
                break;

            case Rotate90:
                dx = spec.rect.left();
                dy = printer->paperRect().height() - spec.rect.top();
                break;

            case Rotate180:
                dx = spec.rect.right();
                dy = printer->paperRect().height() - spec.rect.top();
                break;

            case Rotate270:
                dx = spec.rect.right();
                dy = printer->paperRect().height() - spec.rect.bottom();
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

            // Translate for page rect(x1,y1) ..
            *out += QString("q\n1 0 0 1 %1 %2 cm\n")
                    .arg(-page->rect().left(), 0, 'f', 3)
                    .arg(-page->rect().top(), 0, 'f', 3);


            for (int j=0; j<page->pdfInfo().xObjNums.size(); ++j)
                *out += QString("/Im%1_%2 Do\n").arg(i).arg(j);


            if (printer->drawBorder())
            {
                *out += QString("%1 %2 %3 %4 re\nS\n")
                        .arg(page->rect().left(),   0, 'f', 3)
                        .arg(page->rect().top(),    0, 'f', 3)
                        .arg(page->rect().width(),  0, 'f', 3)
                        .arg(page->rect().height(), 0, 'f', 3);
            }


            *out += "Q\n";
            *out += "Q\n";
            *out += "Q\n";
            *out += "Q\n";
        }
    }
}


/************************************************

 ************************************************/
void TmpPdfFile::mergerFinished(int)
{
    mValid = (mMerger->exitCode() == 0 ) &&
             (mMerger->exitStatus() == QProcess::NormalExit);

    mMerger->close();
    mMerger->deleteLater();
    mMerger = 0;

    QFile f(mFileName);
    f.open(QFile::ReadOnly);
    mOrigFileSize = f.size();
    f.close();

    emit progress(0,0);
    emit merged();
}


/************************************************
 *
 * ***********************************************/
void TmpPdfFile::ipcPageInfo(int fileNum, int pageNum, const PdfPageInfo &info)
{
    mPagesInfo.insert(QString("%1:%2").arg(fileNum).arg(pageNum), info);
}


/************************************************
 *
 * ***********************************************/
void TmpPdfFile::ipcXRefInfo(qint64 xrefPos, qint32 freeNum)
{
    mOrigXrefPos = xrefPos;
    mFirstFreeNum = freeNum;
}


/************************************************
 *
 * ***********************************************/
PdfPageInfo TmpPdfFile::pageInfo(InputFile file, int pageNum)
{
    return mPagesInfo.value(QString("%1:%2").arg(mInputFiles.indexOf(file)).arg(pageNum));
}
