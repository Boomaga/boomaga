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


class FileLoader : public QObject
{
    Q_OBJECT
public:
    explicit FileLoader(QObject *parent = nullptr);
    ~FileLoader();

public slots:
    void load(const QString &fileName);

signals:
    void jobsReady(const QStringList &jobs);

private slots:
    void workerJobReady(const QString &jobFile, quint64 readerId, quint32 fileNum);
    void readerError(const QString &error);

private:
    QMap<QString, QString> mResults;
    quint32 mWorked;

    void sendResult();
};


class FileLoaderWorker : public QObject
{
    Q_OBJECT
public:
    explicit FileLoaderWorker(const QString &fileName, const quint64 id);

    struct Result
    {
        quint64 id = 0;
        quint64 subId = 0;
        QString jobFileName;
        quint32 pageCount = 0;
    };
public slots:
    void run();

signals:
    void jobReady(const QString &jobFile, quint64 readerId, quint32 fileNum);
    void finished();
    void errorOccurred(const QString &error);

private:
    QFile mFile;
    quint64 mId;

    void readPdf(const QString &pdfFile);
    void readCupsBoo();
    void readBoo();
    void readPostScript(QFile &psFile, qint64 endPos);
};

#endif // FILELOADER_H
