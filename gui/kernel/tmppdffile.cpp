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

#include "project.h"
#include "job.h"
#include "inputfile.h"
#include "sheet.h"
#include "layout.h"
#include "math.h"
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
TmpPdfFile::TmpPdfFile(const QList<InputFile> files, QObject *parent):
    QObject(parent),
    mJobs(new JobList()),
    mMerger(0),
    mPageCount(0),
    mValid(false),
    mRender(0)
{
    mOrigFileSize = 0;
    mOrigXrefPos = 0;
    mFirstFreeNum = 0;

    mFileName = genFileName();

    mInputFiles << files;
}


/************************************************

 ************************************************/
TmpPdfFile::~TmpPdfFile()
{
    delete mJobs;
    delete mRender;
    delete mMerger;
    QFile::remove(mFileName);
}


/************************************************

 ************************************************/
void TmpPdfFile::merge()
{
    mMerger = new QProcess(this);

    connect(mMerger, SIGNAL(finished(int)),
            this, SLOT(mergerFinished(int)));

    connect(mMerger, SIGNAL(readyReadStandardOutput()),
            this, SLOT(mergerOutputReady()));

    mJobs->reserve(mInputFiles.count());

    QStringList args;
    foreach (const InputFile file, mInputFiles)
    {
        args << file.fileName();

        Job *job = new Job(file, this);
        job->setTitle(file.title());
        *mJobs << job;
    }

    args << mFileName;

    static QString boomagamerger;

    if (boomagamerger.isNull())
    {
        QStringList dirs;
        dirs << QApplication::applicationDirPath() + "/";
        dirs << QApplication::applicationDirPath() + "/../lib/boomaga/";

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
        project->error(tr("Something wrong. I can't find boomagamerger program.\nPlease reinstall me."));
        return;
    }

    mMerger->start(boomagamerger, args);

}


/************************************************

 ************************************************/
void TmpPdfFile::updateSheets(QList<Sheet *> &sheets)
{
    if (mRender)
    {
        delete mRender;
        mRender = 0;
    }

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

        mRender = new Render(mFileName);
        connect(mRender, SIGNAL(imageChanged(int)),
                this, SIGNAL(imageChanged(int)));
        mRender->start();
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
void TmpPdfFile::writeDocument(const QList<Sheet *> &sheets, QIODevice *out)
{
    QFile f(mFileName);
    if (!f.open(QFile::ReadOnly))
    {
        project->error(tr("I can read file '%1'").arg(mFileName));
        return;
    }

    qint64 bufLen = qMin(mOrigFileSize - f.pos(), (qint64)(1024 * 1024));
    while (bufLen > 0)
    {
        out->write(f.read(bufLen));
        bufLen = qMin(mOrigFileSize - f.pos(), (qint64)(1024 * 1024));
    }

    writeSheets(out, sheets);
}


/************************************************

 ************************************************/
void TmpPdfFile::writeSheets(QIODevice *out, const QList<Sheet *> &sheets) const
{
    qint32 rootNum = mFirstFreeNum;
    qint32 pagesNum = rootNum + 1;

    QMap<int, qint64> xref;
    QStringList pagesKids;

    // Catalog object ...........................
    xref.insert(rootNum, out->pos());
    *out << rootNum << " 0 obj\n";
    *out << "<<\n";
    *out << "/Type /Catalog\n";
    *out << "/Pages " << pagesNum << " 0 R\n";
    *out << ">>\n";
    *out << "endobj\n";
    // ..........................................

    // Page objects .............................
    qint32 num = pagesNum + 1;
    foreach(const Sheet *sheet, sheets)
    {
        int pegeNum      = num;
        int resourcesNum = num + 1;
        int contentsNum  = num + 2;
        num += 3;


        // Page ............................
        xref.insert(pegeNum, out->pos());
        *out << pegeNum << " 0 obj\n";
        *out << "<<\n";
        *out << "/Type /Page\n";
        *out << "/Contents "  << contentsNum  << " 0 R\n";
        *out << "/Resources " << resourcesNum << " 0 R\n";
        *out << "/Parent " << pagesNum << " 0 R\n";
        *out << ">>\n";
        *out << "endobj\n";

        pagesKids << QString("%1 0 R").arg(pegeNum);
        //..................................


        // Resources .......................
        xref.insert(resourcesNum, out->pos());
        *out << resourcesNum << " 0 obj\n";
        *out << "<<\n";
        *out << "/XObject << ";
        for (int i=0; i< sheet->count(); ++i)
        {
            const ProjectPage *page = sheet->page(i);
            if (page)
            {
                *out << "/Im" << i << " " << sheet->page(i)->pdfObjectNum() <<  " 0 R ";
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

            TransformSpec spec = project->layout()->transformSpec(sheet, i);

            QPointF basePoint;
            switch (spec.rotation)
            {
            case TransformSpec::NoRotate:
                basePoint = spec.rect.topLeft();
                break;

            case TransformSpec::Rotate90:
                basePoint = spec.rect.topRight();
                break;

            case TransformSpec::Rotate180:
                basePoint = spec.rect.bottomRight();
                break;

            case TransformSpec::Rotate270:
                basePoint = spec.rect.bottomLeft();
                break;
            }


            // Translate ........................
            *out += QString("q\n1 0 0 1 %1 %2 cm\n")
                    .arg(basePoint.x(), 0, 'f', 3)
                    .arg(basePoint.y(), 0, 'f', 3);

            // Rotate ...........................
            *out += QString("q\n%1 %2 -%2 %1 0 0 cm\n")
                    .arg(cos(spec.rotation * M_PI / 180), 0, 'f', 3)
                    .arg(sin(spec.rotation * M_PI / 180), 0, 'f', 3);

            // Scale ...........................
            *out += QString("q\n%1 0 0 %1 0 0 cm\n")
                    .arg(spec.scale);

            // Translate for page rect(x1,y1) ..
            *out += QString("q\n1 0 0 1 %1 %2 cm\n")
                    .arg(-page->rect().left(), 0, 'f', 3)
                    .arg(-page->rect().top(), 0, 'f', 3);


            if (printer->drawBorder())
            {
                *out += QString("%1 %2 %3 %4 re\nS\n")
                        .arg(page->rect().left(), 0, 'f', 3)
                        .arg(page->rect().top(), 0, 'f', 3)
                        .arg(page->rect().width(), 0, 'f', 3)
                        .arg(page->rect().height(), 0, 'f', 3);
            }


            *out += QString("/Im%1 Do\n").arg(i);

            *out += "Q\n";
            *out += "Q\n";
            *out += "Q\n";
            *out += "Q\n";
        }
    }
}


/************************************************

 ************************************************/
void TmpPdfFile::mergerOutputReady()
{
    mBuf += QString::fromLocal8Bit(mMerger->readAllStandardOutput());
    QString line;
    int i;
    for (i=0; i<mBuf.length(); ++i)
    {
        QChar c = mBuf.at(i);
        if (c != '\n')
        {
            line += c;
            continue;
        }


        /* Protocol ********************************
            Line format     Description
            F:n:cnt:title   File page counts:
                                n - file index.
                                cnt - page count in this PDF file
                                title - document title

            N:num           Next XRef free num
            X:pos           XRef position

            P:f:n:num:rect  PDF page info:
                            f - file number
                            n - page number
                            num -  XForm object number for page
                            rect- page Rectangle format is
                                left,top,width,height

            E:msg           Error message

            D:msg           Debug message

            A:cnt           All pages count

            S:page          Progress status:
                                page - page num
         ******************************************/

        QStringList data = line.split(':', QString::KeepEmptyParts);

        //***************************************
        if (data.at(0) == "S")
        {
            emit progress(data.at(1).toInt(), mPageCount);
        }


        //***************************************
        else if (data.at(0) == "P")
        {
            int jobNum = data.at(1).toInt();
            int pageNum = data.at(2).toInt();
            int pdfObj  = data.at(3).toInt();
            QStringList r = data.at(4).split(",");

            ProjectPage *page = mJobs->at(jobNum)->page(pageNum);

            page->setPdfObjectNum(pdfObj);
            page->setRect(QRectF(r.at(0).toDouble(),
                               r.at(1).toDouble(),
                               r.at(2).toDouble(),
                               r.at(3).toDouble()));
        }


        //***************************************
        else if (data.at(0) == "A")
        {
            mPageCount = data.at(1).toInt();
        }


        //***************************************
        else if (data.at(0) == "X")
        {
            mOrigXrefPos = data.at(1).toDouble();
        }


        //***************************************
        else if (data.at(0) == "N")
        {
            mFirstFreeNum = data.at(1).toDouble();
        }


        //***************************************
        else if (data.at(0) == "F")
        {
            int fileNum = data.at(1).toInt();
            int pageCnt = data.at(2).toInt();
            QString title = data.at(3);

            Job *job = mJobs->at(fileNum);
            job->setTitle(title);

            for (int j=0; j<pageCnt; ++j)
            {
                ProjectPage *page = new ProjectPage(mInputFiles.at(fileNum), j);
                job->addPage(page);
            }
        }


        //***************************************
        else if (data.at(0) == "E")
        {
            project->error(data.at(1));
            return;
        }


        //***************************************
        else if (data.at(0) == "D")
        {
            qDebug() << data.at(1);
        }

        line = "";
    }

    mBuf.remove(0, i);
}


/************************************************

 ************************************************/
void TmpPdfFile::mergerFinished(int exitCode)
{
    mValid = (mMerger->exitCode() == 0 ) &&
             (mMerger->exitStatus() == QProcess::NormalExit);

    mMerger->close();
    mMerger->deleteLater();
    mMerger = 0;
    mBuf.clear();

    QFile f(mFileName);
    f.open(QFile::ReadOnly);
    mOrigFileSize = f.size();
    f.close();

    emit progress(0,0);
    emit merged();
}


/************************************************

 ************************************************/
QImage TmpPdfFile::image(int sheetNum) const
{
    if (mRender)
        return mRender->image(sheetNum);
    else
        return QImage();
}



