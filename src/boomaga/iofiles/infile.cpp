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


#include "infile.h"
#include <QFile>
#include <QFileInfo>
#include "boomagatypes.h"
#include "pdffile.h"
#include "boofile.h"
#include "cupsboofile.h"
#include "postscriptfile.h"


/************************************************
 *
 ************************************************/
InFile::Type InFile::getType(const QString &fileName)
{
    QFile file;
    mustOpenFile(fileName, &file);

    QByteArray mark = file.read(30);
    file.close();

    // Read CUPS_BOOMAGA .........................
    if (mark.startsWith("\033CUPS_BOOMAGA"))
        return Type::CupsBoo;

    // Read PDF ..................................
    if (mark.startsWith("%PDF-"))
        return Type::Pdf;


    // Read BOO ..................................
    if (mark.startsWith("\x1B%-12345X@PJL BOOMAGA_PROJECT") ||
        mark.startsWith("\x1B%-12345X@PJL BOOMAGA_PROGECT")) // <- Back compatibility
        return Type::Boo;


    // Read PostScript ...........................
    if (mark.startsWith("%!PS-Adobe-"))
        return Type::PostScript;

    // Unknown format ............................
    throw BoomagaError(
                tr("I can't read file \"%1\" either because "
                   "it's not a supported file type, or because "
                   "the file has been damaged.")
                .arg(file.fileName()).toStdString());
}


/************************************************
 *
 ************************************************/
InFile *InFile::fromFile(const QString &fileName, QObject *parent)
{
    switch (getType(fileName))
    {
    case Type::Pdf:
        return new PdfFile(parent);

    case Type::PostScript:
        return new PostScriptFile(parent);

    case Type::Boo:
        return new BooFile(parent);

    case Type::CupsBoo:
        return new CupsBooFile(parent);

    default:
        return nullptr;
    }
}


/************************************************
 *
 ************************************************/
InFile::InFile(QObject *parent) :
    QObject(parent),
    mStartPos(0),
    mEndPos(0)
{

}


/************************************************
 *
 ************************************************/
void InFile::load(const QString &fileName, qint64 startPos, qint64 endPos)
{
    mFileName = fileName;
    mStartPos = startPos;
    mEndPos   = endPos;

    if (!mEndPos)
    {
        QFileInfo fi(fileName);
        mEndPos = fi.size();
    }

    read();
}


/************************************************
 *
 ************************************************/
void InFile::addPages(const Job &src, QVector<int> pageNums, Job *dest)
{
    if (pageNums.isEmpty())
    {
        for (int p=0; p<src.pageCount(); ++p)
            dest->addPage(src.page(p)->clone());
    }
    else
    {
        int cnt = src.pageCount();
        foreach(const int &pageNum, pageNums)
        {
            // Some wrong, skip
            if (pageNum >= cnt)
            {
                qWarning() << QString("Page %1 out of range 1..%3")
                              .arg(pageNum+1).arg(cnt);
                continue;
            }

            if (pageNum < 0)
                dest->addPage(new ProjectPage());    // Blank page
            else
                dest->addPage(src.page(pageNum)->clone()); // Normal page
        }
    }
}
