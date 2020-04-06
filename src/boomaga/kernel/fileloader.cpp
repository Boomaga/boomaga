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



#include "fileloader.h"
#include <QThread>
#include "../debug.h"
#include "pdfparser/pdfreader.h"
#include "pdfparser/pdfwriter.h"
#include "pdfparser/pdfobject.h"
#include "boomagatypes.h"
#include <QProcess>


/************************************************
 *
 ************************************************/
FileLoader::FileLoader(QObject *parent):
    QObject(parent)
{

}


/************************************************
 *
 ************************************************/
FileLoader::~FileLoader()
{

}


/************************************************
 *
 ************************************************/
void FileLoader::load(const QString &fileName)
{
    static quint64 id;
    id++;

    FileLoaderWorker *worker = new FileLoaderWorker(fileName, id);
    QThread *thread = new QThread();
    worker->moveToThread(thread);

    connect(thread, &QThread::started,
            worker, &FileLoaderWorker::run);

    connect(worker, &FileLoaderWorker::finished,
            this, [thread, worker, this]() {
        thread->quit();
        thread->wait();
        worker->deleteLater();
        thread->deleteLater();
        --mWorked;
        if (!mWorked) {
            sendResult();
        }
    });

    connect(worker, &FileLoaderWorker::errorOccurred,
            this, &FileLoader::readerError);

    connect(worker, &FileLoaderWorker::jobReady,
            this, &FileLoader::workerJobReady);

    ++mWorked;
    thread->start();
}


/************************************************
 *
 ************************************************/
void FileLoader::workerJobReady(const QString &jobFile, quint64 readerId, quint32 fileNum)
{
    DEBUG;
    mResults.insert(QString("%1:%2").arg(readerId).arg(fileNum), jobFile);
}


/************************************************
 *
 ************************************************/
void FileLoader::readerError(const QString &error)
{
    DEBUG << error;
    qWarning() << "ERROR" << error;
}


/************************************************
 *
 ************************************************/
void FileLoader::sendResult()
{
    DEBUG;
    emit jobsReady(mResults.values());
    mResults.clear();
}


/************************************************
 *
 ************************************************/
FileLoaderWorker::FileLoaderWorker(const QString &fileName, const quint64 id):
    QObject(),
    mFile(fileName),
    mId(id)
{

}


/************************************************
 *
 ************************************************/
void FileLoaderWorker::run()
{
    try {
        mustOpenFile(mFile.fileName(), &mFile);

        QByteArray mark = mFile.read(30);

        // Read CUPS_BOOMAGA .........................
        if (mark.startsWith("\033CUPS_BOOMAGA")) {
            readCupsBoo();
            emit finished();
            return;
        }

        // Read PDF ..................................
        if (mark.startsWith("%PDF-")) {
            mFile.close();
            readPdf(mFile.fileName());
            emit finished();
            return;
        }

        // Read BOO ..................................
        if (mark.startsWith("\x1B%-12345X@PJL BOOMAGA_PROJECT") ||
            mark.startsWith("\x1B%-12345X@PJL BOOMAGA_PROGECT")) {// <- Back compatibility
            readBoo();
            emit finished();
            return;
        }

        // Read PostScript ...........................
        if (mark.startsWith("%!PS-Adobe-")) {
            mFile.seek(0);
            readPostScript(mFile, mFile.size());
            emit finished();
            return;
        }

        // Unknown format ............................
        throw BoomagaError(
            tr("I can't read file \"%1\" either because "
               "it's not a supported file type, or because "
               "the file has been damaged.")
               .arg(mFile.fileName()).toStdString());

    }
    catch (BoomagaError &err) {
        emit errorOccurred(err.what());
        emit finished();
    }
}


/************************************************
 *
 ************************************************/
void FileLoaderWorker::readPdf(const QString &pdfFile)
{
    QFile jobFile(genJobFileName());
    if (!jobFile.open(QFile::WriteOnly)) {
        throw BoomagaError(tr("I can't write file \"%1\"")
                           .arg(jobFile.fileName())
                           + "\n" + jobFile.errorString());
    }

    PDF::Writer writer(&jobFile);
    PDF::Reader reader;

    reader.open(pdfFile);

    writer.writePDFHeader(1,7);
    for (auto x: reader.xRefTable()) {
        if (x.type() != PDF::XRefEntry::Free) {
            writer.writeObject(reader.getObject(x));
        }
    }

    writer.writeXrefTable();

    PDF::Dict trailer = reader.trailerDict();
    trailer.remove("Prev");
    writer.writeTrailer(trailer);

    jobFile.close();
    emit jobReady(jobFile.fileName(), mId, 0);
}


/************************************************
 *
 ************************************************/
void FileLoaderWorker::readCupsBoo()
{

}


/************************************************
 *
 ************************************************/
void FileLoaderWorker::readBoo()
{

}


/************************************************
 *
 ************************************************/
void FileLoaderWorker::readPostScript(QFile &psFile, qint64 endPos)
{
    QString pdfFile = genJobFileName();

    QStringList args;
    args << "-dNOPAUSE";
    args << "-dBATCH";
    args << "-dSAFER";
    args << "-sDEVICE=pdfwrite";
    args << "-sOutputFile=" + pdfFile;
    args << "-q";
    args << "-c" << ".setpdfwrite";
    args << "-f" << "-";


    QProcess process;
    process.start("gs", args, QProcess::ReadWrite);
    if (!process.waitForStarted()) {
        throw BoomagaError(tr("I can't start gs converter: \"%1\"",
                              "Error message. 'gs' is a command line tool from ghostscript")
                           .arg("timeout"));
    }

    const qint64 BUF_SIZE= 4096 * 1024;
    while (!psFile.atEnd() && psFile.pos() < endPos) {
        process.write(
            psFile.read(qMin(BUF_SIZE, endPos - psFile.pos())));
    }

    process.closeWriteChannel();

    while (!process.waitForFinished(5)) {
        qApp->processEvents();
    }

    if (process.exitCode() != 0) {
        throw BoomagaError(tr("I can't start gs converter: \"%1\"",
                              "Error message. 'gs' is a command line tool from ghostscript")
                           .arg(QString::fromLocal8Bit(process.readAllStandardError())));
    }

    readPdf(pdfFile);
}
