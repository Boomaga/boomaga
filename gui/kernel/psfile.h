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


#ifndef PSFILE_H
#define PSFILE_H


#include <QObject>
#include <QList>
#include <QFile>
#include <QRect>
#include <QHash>

class PsFile;
class QTextStream;

class PsFilePos
{
public:
    PsFilePos();
    long begin;
    long end;
};


class PsFilePage
{
    friend class PsFile;
public:
    PsFilePage(PsFile *file, const PsFilePos &pos, int pageNum);

    PsFile *file() const { return mFile; }
    int pageNum() const { return mPageNum; }
    PsFilePos filePos() const { return mPos; }
    QRect rect() const { return mRect; }

protected:
    void setRect(const QRect &value);

private:
    PsFile *mFile;
    PsFilePos mPos;
    int mPageNum;
    QRect mRect;
};


class PsFile
{
public:
    explicit PsFile(const QString &fileName);
    virtual ~PsFile();

    QString fileName() const;

    QString title() const { return mTitle; }

    PsFilePage page(int index) { return mPages[index]; }
    int pageCount() const { return mPages.count(); }

    void writeProlog(QTextStream *out) const;
    void writeSetup(QTextStream *out) const;
    void writeTrailer(QTextStream *out) const;

protected:
    PsFile();

    QFile mFile;
    QList<PsFilePage> mPages;

    bool parse();
    void writeFilePart(long begin, long end, QTextStream *out) const;

private:
    QString mTitle;
    PsFilePos mPrologPos;
    PsFilePos mSetupPos;
    PsFilePos mTrailerPos;
};


class GsMergeFile: public PsFile
{
public:
    GsMergeFile(const QString &fileName);
    virtual ~GsMergeFile();

    bool merge(const QList<PsFile*> inputFiles);

    void writePage(const PsFile* mergedFile, int pageNum, QTextStream *out) const;
    QRect pageRect(const PsFile* mergedFile, int pageNum) const;

private:
    QHash<const PsFile*, int> mMergedFiles;

    int pageIndex(const PsFile* mergedFile, int pageNum) const;
};

#endif // PSFILE_H
