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


class PsFile : public QObject
{
    Q_OBJECT
public:
    explicit PsFile(const QString &fileName, QObject *parent = 0);
    virtual ~PsFile();

    QString fileName() const;

    QString title() const { return mTitle; }

    PsFilePage page(int index) { return mPages[index]; }
    int pageCount() const { return mPages.count(); }

    PsFilePos prologPos() const { return mPrologPos; }
    PsFilePos setupPos() const { return mSetupPos; }
    PsFilePos trailerPos() const { return mTrailerPos; }

    void writeFilePart(const PsFilePos &pos, QTextStream *out);
    void writeFilePart(long begin, long end, QTextStream *out);

    void writePageBody(const PsFilePos &pos, QTextStream *out);
    void writePageBody(long begin, long end, QTextStream *out);

    bool parse();

protected:
    QFile mFile;
    QList<PsFilePage> mPages;

private:
    QString mTitle;
    PsFilePos mPrologPos;
    PsFilePos mSetupPos;
    PsFilePos mTrailerPos;
};


class GsMergeFile: public PsFile
{
    Q_OBJECT
public:
    GsMergeFile(const QString &fileName, QObject *parent = 0);
    virtual ~GsMergeFile();

    bool merge(const QStringList inputFiles);
    bool merge(const QList<PsFile*> inputFiles);

private:

};

#endif // PSFILE_H
