/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2020 Boomaga team https://github.com/Boomaga
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




#ifndef FILELOADER_H
#define FILELOADER_H

#include <QObject>
#include <QMap>
#include "job.h"
#include "boomagatypes.h"

class MetaData;


class FileLoader : public QObject
{
    Q_OBJECT
public:
    explicit FileLoader(QObject *parent = nullptr);
    ~FileLoader();

public slots:
    void load(const QString &fileName);

signals:
    void jobsReady(const JobList &jobs);
    void metaDataReady(const MetaData &metaData);

private slots:
    void workerJobReady(const Job &job, quint64 readerId, quint32 fileNum);
    void readerError(const QString &error);

private:
    QMap<QString, Job> mResults;
    quint32 mWorked = 0;

    void sendResult();
};


class FileLoaderWorker : public QObject
{
    Q_OBJECT
public:
    explicit FileLoaderWorker(const QString &fileName, const quint64 id);

public slots:
    void run();

signals:
    void jobReady(const Job &job, quint64 readerId, quint32 fileNum);
    void metaDataReady(const MetaData &metaData);
    void finished();
    void errorOccurred(const QString &error);

private:
    QString mFileName;
    quint64 mId;

    Job readPdf(const QString &pdfFile, qint64 startPos = 0, qint64 endPos = 0);
    Job readPostScript(QFile &psFile, qint64 endPos);
    void loadBoo(QFile &booFile);
    void loadCupsBoo(QFile &inFile);
};

#endif // FILELOADER_H
