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

#include <QFileInfo>
#include <QDebug>


#include <math.h>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0)) // for Qt::escape
    #include <QTextDocument>
#endif


/************************************************
 *
 ************************************************/
BooFile::PageSpec::PageSpec(int pageNum, bool hidden, Rotation rotation, bool startBooklet):
    mPageNum(pageNum),
    mHidden(hidden),
    mRotation(rotation),
    mStartBooklet(startBooklet)
{

}

/************************************************
 *
 ************************************************/
BooFile::PageSpec::PageSpec(const QString &spec)
{
    bool ok;
    // In the file pages are numbered starting with 1 not 0.
    mPageNum = spec.section(":", 0, 0).toInt(&ok) - 1;
    if (!ok)
        mPageNum = -1;

    mHidden = spec.section(":", 1, 1) == "H";

    QString s = spec.section(":", 2, 2);
    if      (s == "90" ) mRotation = Rotate90;
    else if (s == "180") mRotation = Rotate180;
    else if (s == "270") mRotation = Rotate270;
    else                 mRotation = NoRotate;

    mStartBooklet = spec.section(":", 3, 3) == "S";
}

/************************************************
 *
 ************************************************/
QString BooFile::PageSpec::asString()
{
    QString res;

    if (mStartBooklet)
        res = ":S" + res;
    else if (!res.isEmpty())
        res = ":" + res;


    switch (mRotation)
    {
    case Rotate90:  res = ":90"  + res; break;
    case Rotate180: res = ":180" + res; break;
    case Rotate270: res = ":270" + res; break;
    default:
        if (!res.isEmpty())
            res = ":" + res;
    }

    if (mHidden)
        res = ":H" + res;
    else if (!res.isEmpty())
        res = ":" + res;


    if (mPageNum < 0)
        res = "B" + res;
    else
        res = QString("%1").arg(mPageNum + 1) + res;

    return res;
}


/************************************************
 *
 * ***********************************************/
QList<BooFile::PageSpec> BooFile::PageSpec::readPagesSpec(const QString &str)
{
    QList<PageSpec> res;

    foreach(QString s, str.split(',', QString::SkipEmptyParts))
    {
        res << PageSpec(s);

    }
    return res;
}


/************************************************

 ************************************************/
BooFile::BooFile(QObject *parent) :
    QObject(parent)
{
}


/************************************************

 ************************************************/
BooFile::~BooFile()
{
    mJobs.clear();
}


/************************************************

 ************************************************/
void BooFile::load(const QString &fileName)
{
    mJobs.clear();

    QFile file(fileName);
    if(!file.open(QFile::ReadOnly))
    {
        throw tr("I can't open file \"%1\"").arg(fileName) + "\n" + file.errorString();
    }

    file.seek(0);

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

                    QList<int> pages;
                    foreach(PageSpec spec, pagesSpec)
                    {
                        if (!spec.isblank())
                            pages << spec.pageNum();
                    }

                    Job job(fileName, pages, startPos, endPos);
                    job.setTitle(title);

                    for (int i=0; i<qMin(pagesSpec.count(), job.pageCount()); ++i)
                    {
                        PageSpec spec = pagesSpec.at(i);
                        if (spec.isblank())
                            job.insertBlankPage(i);

                        ProjectPage *page = job.page(i);

                        if (spec.isHidden())
                            page->setVisible(false);

                        if (spec.isStartBooklet())
                            page->setManualStartSubBooklet(true);

                        page->setManualRotation(spec.rotation());
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
                             ).asString();
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


/************************************************

 ************************************************/
QByteArray BooFile::readJobPDF(const Job &job)
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

 ************************************************/
void BooFile::write(QFile *out, const QByteArray &data)
{
    if (out->write(data) < 0)
        throw QObject::tr("I can't write to file '%1'").arg(out->fileName()) + "\n" + out->errorString();
}



/************************************************

 ************************************************/
void BooFile::writeCommand(QFile *out, const QString &command, const QString &data)
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    QString v = Qt::escape(data);
#else
    QString v = data.toHtmlEscaped();
#endif
    write(out, QString("@PJL BOOMAGA %1=\"%2\"\n").arg(command).arg(v).toLocal8Bit());
}


/************************************************

 ************************************************/
void BooFile::writeCommand(QFile *out, const QString &command, const QList<int> &data)
{
    QStringList sl;
    for(int i=0; i<data.count(); ++i)
    {
        // In the file pages are numbered starting with 1 not 0.
        sl << QString("%1").arg(data.at(i) + 1);
    }

    writeCommand(out, command, sl.join(","));
}
