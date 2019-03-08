/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2014 Boomaga team https://github.com/Boomaga
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


#include "boofile.h"
#include "pdffile.h"
#include <QFile>
#include <QFileInfo>


/************************************************
 *
 ************************************************/
BooFile::PageSpec::PageSpec(int pageNum, bool hidden, Rotation rotation, bool startBooklet):
    pageNum(pageNum),
    rotation(rotation),
    hidden(hidden),
    startBooklet(startBooklet)
{
}


/************************************************
 *
 ************************************************/
BooFile::PageSpec::PageSpec(const QString &str):
    pageNum(0),
    rotation(NoRotate),
    hidden(false),
    startBooklet(0)
{
    bool ok;
    // In the file pages are numbered starting with 1 not 0.
    pageNum = str.section(":", 0, 0).toInt(&ok) - 1;
    if (!ok)
        pageNum = -1;

    hidden = str.section(":", 1, 1) == "H";

    QString s = str.section(":", 2, 2);
    if      (s == "90" ) rotation = Rotate90;
    else if (s == "180") rotation = Rotate180;
    else if (s == "270") rotation = Rotate270;
    else                 rotation = NoRotate;

    startBooklet = str.section(":", 3, 3) == "S";
}


/************************************************
 *
 ************************************************/
QList<BooFile::PageSpec> BooFile::PageSpec::readPagesSpec(const QString &str)
{
    QList<PageSpec> res;

    foreach(QString s, str.split(',', QString::SkipEmptyParts))
        res << PageSpec(s);

    return res;
}


/************************************************
 *
 ************************************************/
QString BooFile::PageSpec::toString() const
{
    QString res;

    if (startBooklet)
        res = ":S" + res;
    else if (!res.isEmpty())
        res = ":" + res;


    switch (rotation)
    {
    case Rotate90:  res = ":90"  + res; break;
    case Rotate180: res = ":180" + res; break;
    case Rotate270: res = ":270" + res; break;
    default:
        if (!res.isEmpty())
            res = ":" + res;
    }

    if (hidden)
        res = ":H" + res;
    else if (!res.isEmpty())
        res = ":" + res;


    if (pageNum < 0)
        res = "B" + res;
    else
        res = QString("%1").arg(pageNum + 1) + res;

    return res;
}



/************************************************
 *
 ************************************************/
BooFile::BooFile(QObject *parent):
    InFile(parent)
{
}


/************************************************
 *
 ************************************************/
void BooFile::read()
{
    QFile file;
    mustOpenFile(mFileName, &file);

    file.seek(mStartPos);

    QString metaAuthor;
    QString metaTitle;
    QString metaSubject;
    QString metaKeywords;

    QString title;
    QList<PageSpec> pagesSpec;

    while (!file.atEnd())
    {
        QString line = QString::fromUtf8(file.readLine()).trimmed();

        if (line.startsWith("@PJL"))
        {
            QString command = line.section(' ', 1, 1, QString::SectionSkipEmpty).toUpper();


            // Boomaga commands ................................
            if (command == "BOOMAGA")
            {
                QString subCommand = line.section(' ', 2, -1, QString::SectionSkipEmpty)
                        .section('=', 0, 0, QString::SectionSkipEmpty)
                        .trimmed()
                        .toUpper();

                QString value = line.section('=', 1,-1, QString::SectionSkipEmpty).trimmed();
                if (value.startsWith('"') || value.startsWith('\''))
                    value = value.mid(1, value.length()-2);


                if (subCommand == "META_AUTHOR")
                    metaAuthor = value;

                else if (subCommand == "META_TITLE")
                    metaTitle = value;

                else if (subCommand == "META_SUBJECT")
                    metaSubject = value;

                else if (subCommand == "META_KEYWORDS")
                    metaKeywords = value;



                else if (subCommand == "JOB_TITLE")
                    title = value;

                else if (subCommand == "JOB_PAGES")
                    pagesSpec = PageSpec::readPagesSpec(value);

                else
                    qWarning() << QString("Unknown command '%1' in the line '%2'").arg(subCommand).arg(line);

            }
            // Boomaga commands ................................


            // PDF stream ......................................
            else if (command == "ENTER")
            {
                QString subCommand = line.section(' ', 2, -1, QString::SectionSkipEmpty).remove(' ').toUpper();

                if (subCommand == "LANGUAGE=PDF")
                {
                    qint64 startPos = file.pos();
                    qint64 endPos = 0;
                    while (!file.atEnd())
                    {
                        QByteArray buf = file.readLine();
                        if (buf.startsWith("\x1B%-12345X@PJL"))
                            break;

                        endPos = file.pos() - 1;
                    }

                    PdfFile pdf;
                    pdf.load(mFileName, startPos, endPos);

                    Job job;
                    job.setFileName(mFileName);
                    job.setFilePos(startPos, endPos);
                    job.setTitle(title);

                    Job pdfJob = pdf.jobs().first();

                    // remove wrong pages .................
                    {
                        int cnt = pdfJob.pageCount();
                        for (int i=pagesSpec.count()-1; i>=0; --i)
                        {
                            if (pagesSpec.at(i).pageNum >= cnt)
                                pagesSpec.removeAt(i);
                        }
                    }

                    QVector<int> pageNums;
                    pageNums.reserve(pagesSpec.count());
                    foreach(const PageSpec &spec, pagesSpec)
                        pageNums << spec.pageNum;

                    addPages(pdfJob, pageNums, &job);

                    for(int i=0; i<pagesSpec.count(); ++i)
                    {
                        const PageSpec &spec = pagesSpec.at(i);
                        ProjectPage *page = job.page(i);

                        if (spec.hidden)
                            page->setVisible(false);

                        if (spec.startBooklet)
                            page->setManualStartSubBooklet(true);

                        page->setManualRotation(spec.rotation);
                    }

                    mJobs << job;
                    title.clear();
                    pagesSpec.clear();
                }
            }
            // PDF stream ......................................
        }
    }
    file.close();

    mMetaData.setAuthor(metaAuthor);
    mMetaData.setTitle(metaTitle);
    mMetaData.setSubject(metaSubject);
    mMetaData.setKeywords(metaKeywords);
}


/************************************************

 ************************************************/
static void write(QFile *out, const QByteArray &data)
{
    if (out->write(data) < 0)
        throw QObject::tr("I can't write to file '%1'").arg(out->fileName()) + "\n" + out->errorString();
}


/************************************************

 ************************************************/
static void writeCommand(QFile *out, const QString &command, const QString &data)
{
    QString v = data.toHtmlEscaped();
    write(out, QString("@PJL BOOMAGA %1=\"%2\"\n").arg(command).arg(v).toLocal8Bit());
}


/************************************************

 ************************************************/
static QByteArray readJobPDF(const Job &job)
{
    QFile file(job.fileName());
    if(!file.open(QFile::ReadOnly))
    {
        throw QObject::tr("I can't read from file '%1'")
                .arg(job.fileName()) +
                "\n" +
                file.errorString();
    }

    file.seek(job.fileStartPos());
    QByteArray res = file.read(job.fileEndPos() - job.fileStartPos());
    file.close();
    return res;
}


/************************************************
 *
 ************************************************/
void BooFile::save(const QString &fileName)
{
    QString filePath = QFileInfo(fileName).absoluteFilePath();

    // If we write the same file, the program may rewrite data before
    // it will readed, so we store the data in the memory.
    QVector<QByteArray> documents(mJobs.count());
    for (int i=0; i<mJobs.count(); ++i)
    {
        const Job &job = mJobs.at(i);
        if (job.fileName() == filePath)
        {
            documents[i] = readJobPDF(job);
        }
    }


    QFile file(filePath);
    if(!file.open(QFile::WriteOnly))
    {
        throw tr("I can't write to file '%1'").arg(fileName) + "\n" + file.errorString();
    }


    write(&file, "\x1B%-12345X@PJL BOOMAGA_PROJECT\n");


    if (!mMetaData.author().isEmpty())
        writeCommand(&file, "META_AUTHOR", mMetaData.author());

    if (!mMetaData.title().isEmpty())
        writeCommand(&file, "META_TITLE", mMetaData.title());

    if (!mMetaData.subject().isEmpty())
        writeCommand(&file, "META_SUBJECT", mMetaData.subject());

    if (!mMetaData.keywords().isEmpty())
        writeCommand(&file, "META_KEYWORDS", mMetaData.keywords());


    for (int i=0; i<mJobs.count(); ++i)
    {
        const Job &job = mJobs.at(i);

        QStringList pages;
        for (int p=0; p<job.pageCount(); ++p)
        {
            const ProjectPage *page = job.page(p);
            pages << PageSpec(page->jobPageNum(),
                              page->visible() == false,
                              page->manualRotation(),
                              page->isManualStartSubBooklet()
                             ).toString();
        }

        writeCommand(&file, "JOB_PAGES", pages.join(","));


        if (!job.title(false).isEmpty())
            writeCommand(&file, "JOB_TITLE", job.title(false));

        write(&file, "@PJL ENTER LANGUAGE=PDF\n");

        if (documents.at(i).count())
        {
            const QByteArray &data = documents.at(i);
            write(&file, data);
            if (!data.endsWith('\n'))
                write(&file, "\n");
        }
        else
        {
            const QByteArray data = readJobPDF(job);
            write(&file, data);
            if (!data.endsWith('\n'))
                write(&file, "\n");
        }

        write(&file, "\x1B%-12345X@PJL\n");
    }

    write(&file, "@PJL EOJ\n");
    write(&file, "\x1B%-12345X");

    file.close();
}
