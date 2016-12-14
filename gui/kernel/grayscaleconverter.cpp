/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2016 Boomaga team https://github.com/Boomaga
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


#include "grayscaleconverter.h"
#include "tmppdffile.h"
#include "project.h"
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QProcess>
#include <QDebug>

struct GrayScaleConverterRequest
{
    QString fromFileName;
    int     start;
    int     end;
};

GrayScaleConverter::GrayScaleConverter(QObject *parent) :
    QObject(parent),
    mRequests(new QList<GrayScaleConverterRequest>)
{
}

bool convert(const QString &fromFile, int startPos, int endPos, const QString &outFile)
{
    QStringList args;
    args << "-dQUIET";                          // Prevent Ghostscript from writing messages to standard output
    args << "-sDEVICE=pdfwrite";                // Write to PDF
    args << "-sProcessColorModel=DeviceGray";   // Write to PDF
    args << "-sColorConversionStrategy=Gray";   // Write to PDF
    args << "-o" << outFile;
    args << "-f" << fromFile;
    qDebug() << args;

    QProcess proc;
    proc.start("gs", args);
    proc.waitForStarted();
    proc.waitForFinished(-1);

    if (proc.exitStatus() == 0 && proc.exitCode() == 0)
        return true;

    QString msg = QString::fromLocal8Bit(proc.readAllStandardError());
    project->error(QString("Convert to grayscale: gs exit with code %1: %2")
          .arg(proc.exitCode())
          .arg(msg));

    return false;
}


GrayScaleConverter::~GrayScaleConverter()
{
    delete mRequests;
}

void GrayScaleConverter::addRequest(const QString &fromFileName, int startPos, int endPos)
{
    GrayScaleConverterRequest req;
    req.fromFileName = fromFileName;
    req.start = startPos;
    req.end = endPos;
    *mRequests << req;
}

QStringList GrayScaleConverter::run()
{
    QStringList res;

    QFutureWatcher<bool> watcher;


    foreach (const GrayScaleConverterRequest &req, *mRequests)
    {
        QString outFile = genTmpFileName();
        res << outFile;
        qDebug() << "**********************START" << req.fromFileName << outFile;
        QFuture<bool> future = QtConcurrent::run(convert, req.fromFileName, req.start, req.end, outFile);
        watcher.setFuture(future);
    }

    watcher.waitForFinished();

    qDebug() << "**********************DONE" << watcher.result();

    return res;
}
