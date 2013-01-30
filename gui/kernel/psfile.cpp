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


#include "psfile.h"

#include <QTextStream>
#include <QDebug>
#include <QStringList>
#include "third-party/libspectre/ps.h"

#include <ghostscript/iapi.h>
#include <ghostscript/ierrors.h>


/************************************************

 ************************************************/
PsFilePos::PsFilePos()
{
    begin = 0;
    end = 0;
}


/************************************************

 ************************************************/
PsFilePage::PsFilePage(PsFile *file, const PsFilePos &pos, int pageNum):
    mFile(file),
    mPageNum(pageNum),
    mPos(pos)
{

}


/************************************************

 ************************************************/
void PsFilePage::setRect(const QRect &value)
{
    mRect = value;
}


/************************************************

 ************************************************/
PsFile::PsFile(const QString &fileName, QObject *parent):
    QObject(parent),
    mFile(fileName)
{
}


/************************************************

 ************************************************/
PsFile::~PsFile()
{

}


/************************************************

 ************************************************/
QString PsFile::fileName() const
{
    return mFile.fileName();
}


/************************************************

 ************************************************/
void PsFile::writeFilePart(const PsFilePos &pos, QTextStream *out)
{
    writeFilePart(pos.begin, pos.end, out);
}


/************************************************

 ************************************************/
void PsFile::writeFilePart(long begin, long end, QTextStream *out)
{
    if (!mFile.isOpen())
        mFile.open(QIODevice::ReadOnly | QIODevice::Text);

    if (mFile.isOpen())
    {
        mFile.seek(begin);
        QByteArray buf = mFile.read(end - begin);
        *out << buf;
    }
}


/************************************************

 ************************************************/
void PsFile::writePageBody(const PsFilePos &pos, QTextStream *out)
{
    writePageBody(pos.begin, pos.end, out);
}


/************************************************

 ************************************************/
void PsFile::writePageBody(long begin, long end, QTextStream *out)
{
    mFile.seek(begin);

    // Skeep %%Page: tag line
    QString s = mFile.readLine();
    qint64 len = end - begin - s.length();

    QByteArray buf = mFile.read(len);
    *out << buf;
}


/************************************************

 ************************************************/
bool PsFile::parse()
{
    document *doc = psscan(mFile.fileName().toLocal8Bit(), SCANSTYLE_NORMAL);

    if (!doc)
    {
        qWarning() << QString("Can't scan input file %1").arg(mFile.fileName());
        return false;
    }

    mTitle = QString::fromLocal8Bit(doc->title);
    mPrologPos.begin = doc->beginprolog;
    mPrologPos.end = doc->endprolog;

    mSetupPos.begin = doc->beginsetup;
    mSetupPos.end = doc->endsetup;

    mTrailerPos.begin = doc->begintrailer;
    mTrailerPos.end = doc->endtrailer;

    mPages.clear();
    for (int i = 0; i < doc->numpages; i++)
    {
        PsFilePos pos;
        pos.begin = doc->pages[i].begin;
        pos.end = doc->pages[i].end;
        PsFilePage psPage(this, pos, i);

        int rx=0;
        int ry=0;
        int lx=0;
        int ly=0;
        psgetpagebox(doc, i, &rx, &ry, &lx, &ly);
        psPage.setRect(QRect(lx, ly, rx - lx, ry - ly));
        mPages << psPage;
    }

    psdocdestroy(doc);

    return true;
}


/************************************************

 ************************************************/
GsMergeFile::GsMergeFile(const QString &fileName, QObject *parent):
    PsFile(fileName, parent)
{
}


/************************************************

 ************************************************/
GsMergeFile::~GsMergeFile()
{
    QFile::remove(fileName());
}


/************************************************

 ************************************************/
bool GsMergeFile::merge(const QStringList inputFiles)
{
    if (inputFiles.isEmpty())
    {
        mPages.clear();
        mFile.close();
        QFile::resize(mFile.fileName(), 0);
        return true;
    }

    mFile.close();
    void  *gsInstance;
    int gsRes;
    gsRes = gsapi_new_instance(&gsInstance, 0);
    if (gsRes < 0)
        return false;


    QList<QByteArray> args;
    args << "merge";
    args << "-q";
    args << "-dNOPAUSE";
    args << "-dBATCH";
    args << "-dHaveTrueTypes=false";
    args << "-sDEVICE=ps2write";
    args << QString("-sOutputFile=%1").arg(fileName()).toLocal8Bit();
    for (int i=0; i<inputFiles.count(); ++i)
        args << inputFiles.at(i).toLocal8Bit();

    int argc = args.count();
    char *argv[100];
    for (int i = 0; i < argc; ++i)
    {
        argv[i] = args[i].data();
    }

    // qDebug() << args;

    gsRes = gsapi_init_with_args(gsInstance, argc, argv);
    if (gsRes < -100)
    {
        gsapi_delete_instance(gsInstance);
        return false;
    }

    gsRes = gsapi_exit(gsInstance);
    gsapi_delete_instance(gsInstance);

    mPages.clear();
    if (gsRes != 0 && gsRes != e_Quit)
        return false;

    return parse();
}


/************************************************

 ************************************************/
bool GsMergeFile::merge(const QList<PsFile *> inputFiles)
{
    QStringList files;
    foreach (PsFile *f, inputFiles)
        files << f->fileName();

    return merge(files);
}
