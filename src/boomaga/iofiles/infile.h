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


#ifndef INFILE_H
#define INFILE_H

#include <QObject>
#include "kernel/job.h"
#include "kernel/project.h"

class InFile : public QObject
{
    Q_OBJECT
public:
    enum class Type {
        Pdf,
        PostScript,
        Boo,
        CupsBoo
    };
    explicit InFile(QObject *parent = 0);
    virtual ~InFile() {}

    static Type getType(const QString &fileName);
    static InFile *fromFile(const QString &fileName, QObject *parent = 0);

    void load(const QString &fileName, qint64 startPos=0, qint64 endPos=0);
    virtual Type type() const = 0;

    QString fileName() const { return mFileName; }
    JobList jobs() const { return mJobs; }

    const MetaData metaData() const { return mMetaData; }
    void setMetadata(const MetaData &value) { mMetaData = value; }

signals:
    void startLongOperation(const QString &message);
    void endLongOperation();

protected:
    virtual void read() = 0;
    void addPages(const Job &src, QVector<int> pageNums, Job *dest);

    QString  mFileName;
    JobList  mJobs;
    MetaData mMetaData;
    qint64   mStartPos;
    qint64   mEndPos;
};

#endif // INFILE_H
