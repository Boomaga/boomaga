/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2019 Boomaga team https://github.com/Boomaga
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


#include "cupsboofile.h"
#include "pdffile.h"
#include "postscriptfile.h"
#include <QFile>
#include <QUrl>


struct CupsOptions {
    CupsOptions(const QString &str);
    QVector<int> pages;

private:
    void parsePages(const QString &value);
};


/************************************************
 *
 ************************************************/
CupsOptions::CupsOptions(const QString &str)
{
    foreach (const QString s, str.split(' ', QString::SkipEmptyParts))
    {
        QString key = s.section("=", 0, 0);
        QString val = s.section("=", 1);
        if (key == "page-ranges")
        {
            parsePages(val);
            continue;
        }
    }
}


/************************************************
 *
 ************************************************/
void CupsOptions::parsePages(const QString &value)
{
    QStringList items = value.split(',');
    foreach (QString s, items)
    {
        bool ok;

        int fromPage = s.section("-", 0, 0).toInt(&ok)-1;
        if (!ok)
        {
            qWarning() << QString("Wrong page number '%1' in page spec '%2'")
                          .arg(s.section("-", 0, 0))
                          .arg(value);
            continue;
        }

        int toPage = s.section("-", 1).toInt(&ok)-1;
        if (!ok)
            toPage = fromPage;

        for (int p = fromPage; p<=toPage; ++p)
            pages << p;
    }
}


/************************************************
 *
 ************************************************/
CupsBooFile::CupsBooFile(QObject *parent):
    InFile(parent)
{
}


/************************************************
 *
 ************************************************/
void CupsBooFile::read()
{
    QFile file;
    mustOpenFile(mFileName, &file);
    file.seek(mStartPos);

    QString title;
    QString options;
    int count =1;

    while (!file.atEnd())
    {
         QByteArray line = file.readLine().trimmed();

         if (line.startsWith("TITLE="))
         {
             title = QUrl::fromPercentEncoding(line).mid(6);
             continue;
         }

         if (line.startsWith("OPTIONS="))
         {
             options = QUrl::fromPercentEncoding(line).mid(8);
             continue;
         }

         if (line.startsWith("COUNT="))
         {
             int n =  QString(line).mid(6).toInt();
             if (n) count = n;
             continue;
         }

         if (line.startsWith("CUPS_BOOMAGA_DATA"))
             break;
    }

    CupsOptions opts(options);
    auto startPos = file.pos();
    QByteArray line = file.readLine().trimmed();

    QObject keeper;
    InFile *inFile;

    // Read PDF ..................................
    if (line.startsWith("%PDF-"))
    {
        inFile = new PdfFile(&keeper);
    }

    // Read PostScript ...........................
    else if (line.startsWith("%!PS-Adobe-"))
    {
        inFile = new PostScriptFile(&keeper);
    }

    // Unknown format ............................
    else
    {
        throw BoomagaError(tr("I can't read file \"%1\" either because it's not a supported file type, "
                              "or because the file has been damaged.").arg(mFileName));
    }

    inFile->load(mFileName, startPos, mEndPos);
    Job inJob = inFile->jobs().first();
    int cnt = inJob.pageCount();

    Job job;
    job.setFileName(inJob.fileName());
    job.setFilePos(inJob.fileStartPos(), inJob.fileEndPos());
    job.setTitle(title);

    if (opts.pages.isEmpty())
    {
        // All pages
        for (int p=0; p<cnt; ++p)
        {
            job.addPage(inJob.page(p)->clone());
        }
    }
    else
    {
        // Specified pages
        foreach (int p, opts.pages)
        {
            if (p>-1 && p < cnt)
                job.addPage(inJob.page(p)->clone());
        }
    }

    for (int c=0; c<count; ++c)
    {
        mJobs << job.clone();
    }
}




