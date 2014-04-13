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


#ifndef PROJECTFILE_H
#define PROJECTFILE_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include "job.h"
#include "project.h"

class QFile;

class ProjectFile : public QObject
{
    Q_OBJECT
public:
    explicit ProjectFile(QObject *parent = 0);
    virtual ~ProjectFile();

    void load(const QString &fileName);
    void save(const QString &fileName);

    JobList jobs() const { return mJobs; }
    void setJobs(const JobList value) { mJobs = value; }

    const MetaData metaData() const { return mMetaData; }
    void setMetadata(const MetaData &value) { mMetaData = value; }

signals:
    
public slots:
    
private:
    QString mFileName;
    JobList mJobs;
    MetaData mMetaData;

    QList<int> readPageList(const QString &str);
    QByteArray readJobPDF(const Job &job);
    void write(QFile *out, const QByteArray &data);
    void writeCommand(QFile *out, const QString &command, const QString &data);
    void writeCommand(QFile *out, const QString &command, const QList<int> &data);
};

#endif // PROJECTFILE_H
