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


#ifndef TMPPDFFILE_H
#define TMPPDFFILE_H

#include <QList>
#include <QStringList>
#include <QHash>
#include "project.h"
#include "../pdfmerger/pdfmergeripc.h"

class QProcess;
class Sheet;
class Job;
class JobList;
class Render;

#include "job.h"

class TmpPdfFile: public QObject
{
    Q_OBJECT
    friend class PdfMerger;
public:
    explicit TmpPdfFile(const JobList &jobs, QObject *parent = 0);
    virtual ~TmpPdfFile();

    void merge();
    void updateSheets(QList<Sheet *> &sheets);
    void stop();

    QList<InputFile> inputFiles() const  { return  mInputFiles; }

    QString fileName() const { return mFileName; }

    bool writeDocument(const QList<Sheet*> &sheets, QIODevice *out);

    bool isValid() const { return mValid; }

    PdfPageInfo pageInfo(InputFile file, int pageNum);

signals:
    void merged();
    void progress(int progress, int all) const;

private slots:
    void mergerFinished(int exitCode);

    void ipcPageInfo(int fileNum, int pageNum, const PdfPageInfo &info);
    void ipcXRefInfo(qint64 xrefPos, qint32 freeNum);

private:
    void getPageStream(QString *out, const Sheet *sheet) const;
    void writeSheets(QIODevice *out, const QList<Sheet *> &sheets) const;

    static QString genFileName();

    QList<InputFile> mInputFiles;
    QHash<QString, PdfPageInfo> mPagesInfo;

    QString mFileName;
    qint32 mFirstFreeNum;
    qint64 mOrigFileSize;
    qint64 mOrigXrefPos;
    QProcess *mMerger;
    bool mValid;
};


#endif // TMPPDFFILE_H
