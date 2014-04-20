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


#include "projectfile.h"

#include <QFileInfo>
#include <QDebug>


#include <math.h>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0)) // for Qt::escape
    #include <QTextDocument>
#endif

/************************************************

 ************************************************/
ProjectFile::ProjectFile(QObject *parent) :
    QObject(parent)
{
}


/************************************************

 ************************************************/
ProjectFile::~ProjectFile()
{
    mJobs.clear();
}


/************************************************

 ************************************************/
void ProjectFile::load(const QString &fileName)
{
    mJobs.clear();
    QFileInfo fi(fileName);

    if (!fi.filePath().isEmpty())
    {
        if (!fi.exists())
            throw tr("I can't open file \"%1\" (No such file or directory)")
                              .arg(fi.filePath());

        if (!fi.isReadable())
            throw tr("I can't open file \"%1\" (Access denied)")
                              .arg(fi.filePath());
    }


    QFile file(fileName);
    if(!file.open(QFile::ReadOnly))
    {
        throw tr("I can't open file \"%1\"").arg(fileName) + "\n" + file.errorString();
    }

    file.seek(0);
    QByteArray mark = file.readLine();


    if (mark.startsWith("%PDF-"))
    {
        // Read PDF ..................................
        file.close();
        mJobs << Job(fileName);
        return;
        // Read PDF ..................................
    }
    else if (mark.startsWith("\x1B%-12345X@PJL BOOMAGA_PROGECT"))
    {
        // Read project file .........................
        QString metaAuthor;
        QString metaTitle;
        QString metaSubject;
        QString metaKeywords;

        QString title;
        QList<int> deletedPages;
        QList<int> insertedPages;

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

                    else if (subCommand == "JOB_HIDDEN_PAGES")
                        deletedPages = readPageList(value);

                    else if (subCommand == "JOB_INSERTED_PAGES")
                        insertedPages = readPageList(value);

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
                        qint64 endPos;
                        while (!file.atEnd())
                        {
                            QByteArray buf = file.readLine();
                            if (buf.startsWith("\x1B%-12345X@PJL"))
                                break;

                            endPos = file.pos() - 1;

                        }

                        Job job(fileName, startPos, endPos);
                        job.setTitle(title);

                        qSort(insertedPages);

                        foreach (int num, insertedPages)
                        {
                            if (num>-1)
                                job.insertBlankPage(num);
                        }


                        foreach (int num, deletedPages)
                        {
                            if (num > -1 && num < job.pageCount())
                                job.page(num)->setVisible(false);
                        }


                        mJobs << job;
                        title = "";
                        deletedPages.clear();
                        insertedPages.clear();
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

        // Read project file .........................
    }
    else
    {
        throw tr("I can't read file \"%1\" because is either not a supported file type or because the file has been damaged.").arg(file.fileName());
    }
}


/************************************************
 *
 * ***********************************************/
QList<int> ProjectFile::readPageList(const QString &str)
{
    QList<int> res;

    foreach(QString s, str.split(',', QString::SkipEmptyParts))
    {
        bool ok;
        // In the file pages are numbered starting with 1 not 0.
        int num = s.toInt(&ok) - 1;
        if (ok)
            res << num;
    }
    return res;
}



/************************************************

 ************************************************/
void ProjectFile::save(const QString &fileName)
{
    QString filePath = QFileInfo(fileName).absoluteFilePath();

    // If we write the same file, the program may rewrite data before
    // it will readed, so we store the data in the memory.
    QVector<QByteArray> documents(mJobs.count());
    for (int i=0; i<mJobs.count(); ++i)
    {
        const Job &job = mJobs.at(i);
        if (job.inputFile().fileName() == filePath)
        {
            documents[i] = readJobPDF(job);
        }
    }


    QFile file(filePath);
    if(!file.open(QFile::WriteOnly))
    {
        throw tr("I can't write to file '%1'").arg(fileName) + "\n" + file.errorString();
    }


    write(&file, "\x1B%-12345X@PJL BOOMAGA_PROGECT\n");


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

        QList<int> insertedPages;
        QList<int> deletedPages;
        for(int p=0; p<job.pageCount(); ++p)
        {
            if (job.page(p)->isBlankPage())
                insertedPages << p;

            if (!job.page(p)->visible())
                deletedPages << p;
        }

        if (insertedPages.count())
            writeCommand(&file, "JOB_INSERTED_PAGES", insertedPages);

        if (deletedPages.count())
            writeCommand(&file, "JOB_HIDDEN_PAGES", deletedPages);

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
QByteArray ProjectFile::readJobPDF(const Job &job)
{
    QFile file(job.inputFile().fileName());
    if(!file.open(QFile::ReadOnly))
    {
        throw QObject::tr("I can't read from file '%1'")
                .arg(job.inputFile().fileName()) +
                "\n" +
                file.errorString();
    }

    file.seek(job.inputFile().startPos());
    QByteArray res = file.read(job.inputFile().length());
    file.close();
    return res;
}


/************************************************

 ************************************************/
void ProjectFile::write(QFile *out, const QByteArray &data)
{
    if (out->write(data) < 0)
        throw QObject::tr("I can't write to file '%1'").arg(out->fileName()) + "\n" + out->errorString();
}



/************************************************

 ************************************************/
void ProjectFile::writeCommand(QFile *out, const QString &command, const QString &data)
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
void ProjectFile::writeCommand(QFile *out, const QString &command, const QList<int> &data)
{
    QStringList sl;
    for(int i=0; i<data.count(); ++i)
    {
        // In the file pages are numbered starting with 1 not 0.
        sl << QString("%1").arg(data.at(i) + 1);
    }

    writeCommand(out, command, sl.join(","));
}
