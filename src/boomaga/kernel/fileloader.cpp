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
#include "boofile.h"
#include <QProcess>
#include "kernel/project.h"
#include <QUrl>


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
            this, &FileLoader::errorOccurred);

    connect(worker, &FileLoaderWorker::jobReady,
            this, &FileLoader::workerJobReady);

    connect(worker, &FileLoaderWorker::metaDataReady,
            this, &FileLoader::metaDataReady);

    ++mWorked;
    thread->start();
}


/************************************************
 *
 ************************************************/
void FileLoader::workerJobReady(const Job &job, quint64 readerId, quint32 fileNum)
{
    mResults.insert(QString("%1:%2").arg(readerId).arg(fileNum), job);
}


/************************************************
 *
 ************************************************/
void FileLoader::sendResult()
{
    emit jobsReady(mResults.values());
    mResults.clear();
}


/************************************************
 *
 ************************************************/
FileLoaderWorker::FileLoaderWorker(const QString &fileName, const quint64 id):
    QObject(),
    mFileName(fileName),
    mId(id)
{

}


/************************************************
 *
 ************************************************/
void FileLoaderWorker::run()
{
    try {
        QFile file;
        mustOpenFile(mFileName, &file);

        QByteArray mark = file.read(30);

        // Load CUPS_BOOMAGA .........................
        if (mark.startsWith("\033CUPS_BOOMAGA")) {
            loadCupsBoo(file);
            emit finished();
            return;
        }

        // Load PDF ..................................
        if (mark.startsWith("%PDF-")) {
            file.close();
            Job job = readPdf(file.fileName(), 0, 0);
            emit jobReady(job, mId, 0);
            emit finished();
            return;
        }

        // Load BOO ..................................
        if (mark.startsWith("\x1B%-12345X@PJL BOOMAGA_PROJECT") ||
            mark.startsWith("\x1B%-12345X@PJL BOOMAGA_PROGECT")) {// <- Back compatibility
            loadBoo(file);
            emit finished();
            return;
        }

        // Load PostScript ...........................
        if (mark.startsWith("%!PS-Adobe-")) {
            file.seek(0);
            Job job = readPostScript(file, file.size());
            emit jobReady(job, mId, 0);
            emit finished();
            file.close();
            return;
        }

        // Unknown format ............................
        throw BoomagaError(
            tr("I can't read file \"%1\" either because "
               "it's not a supported file type, or because "
               "the file has been damaged.")
               .arg(mFileName).toStdString());

    }
    catch (PDF::Error &err) {
        QString msg = tr("I can't read file \"%1\" either because "
                         "it's not a supported file type, or because "
                         "the file has been damaged.")
                         .arg(mFileName);

        qWarning() << err.what();
        emit errorOccurred(msg);
        emit finished();

    }
    catch (BoomagaError &err) {
        emit errorOccurred(err.what());
        emit finished();
    }
}


/************************************************
 *
 ************************************************/
Job FileLoaderWorker::readPdf(const QString &pdfFile, qint64 startPos, qint64 endPos)
{
    QFile outFile(genJobFileName());
    if (!outFile.open(QFile::WriteOnly)) {
        throw BoomagaError(tr("I can't write file \"%1\"")
                           .arg(outFile.fileName())
                           + "\n" + outFile.errorString());
    }

    PDF::Writer writer(&outFile);
    PDF::Reader reader;

    reader.open(pdfFile, startPos, endPos);

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

    outFile.close();

    Job job;
    job.setFileName(outFile.fileName());
    job.setTitle(reader.find("/Trailer/Info/Title").asString().value());
    for (quint32 i=0; i<reader.pageCount(); ++i) {
        job.addPage(new ProjectPage(i));
    }

    return job;
}


/************************************************
 *
 ************************************************/
Job FileLoaderWorker::readPostScript(QFile &psFile, qint64 endPos)
{
    QString tmpFile = genJobFileName();

    QStringList args;
    args << "-dNOPAUSE";
    args << "-dBATCH";
    args << "-dSAFER";
    args << "-sDEVICE=pdfwrite";
    args << "-sOutputFile=" + tmpFile;
    args << "-q";
    args << "-c" << ".setpdfwrite";
    args << "-f" << "-";


    QProcess process;
    process.start("gs", args, QProcess::ReadWrite);
    if (!process.waitForStarted()) {
        throw BoomagaError(tr("I can't start gs converter: \"%1\"",
                              "Error message. 'gs' is a command line tool from ghostscript")
                           .arg(process.errorString()));
    }

    const qint64 BUF_SIZE= 4096 * 1024;
    while (!psFile.atEnd() && psFile.pos() < endPos) {
        process.write(
            psFile.read(qMin(BUF_SIZE, endPos - psFile.pos())));
    }

    process.closeWriteChannel();

    if (!process.waitForFinished()) {
        throw BoomagaError(tr("I can't start gs converter: \"%1\"",
                              "Error message. 'gs' is a command line tool from ghostscript")
                           .arg(process.errorString()));
    }

    if (process.exitCode() != 0) {
        throw BoomagaError(tr("I can't start gs converter: \"%1\"",
                              "Error message. 'gs' is a command line tool from ghostscript")
                           .arg(QString::fromLocal8Bit(process.readAllStandardError())));
    }

    Job job = readPdf(tmpFile);
    QFile::remove(tmpFile);
    return job;
}


/************************************************
 *
 ************************************************/
void FileLoaderWorker::loadBoo(QFile &booFile)
{
    MetaData metaData;
    QString title;
    QList<BooFile::PageSpec> pagesSpec;

    int num = 0;
    while (!booFile.atEnd())
    {
        QString line = QString::fromUtf8(booFile.readLine()).trimmed();

        if (line.startsWith("@PJL"))
        {
            QString command = line.section(' ', 1, 1, QString::SectionSkipEmpty).toUpper();


            // Boomaga commands ................................
            if (command == "BOOMAGA")
            {
                QString subCommand = line.section(' ', 2, -1, QString::SectionSkipEmpty)
                        .section('=', 0, 0, QString::SectionSkipEmpty)
                        .trimmed()
                        .toUpper();

                QString value = line.section('=', 1,-1, QString::SectionSkipEmpty).trimmed();
                if (value.startsWith('"') || value.startsWith('\''))
                    value = value.mid(1, value.length()-2);


                if (subCommand == "META_AUTHOR")
                    metaData.setAuthor(value);

                else if (subCommand == "META_TITLE")
                    metaData.setTitle(value);

                else if (subCommand == "META_SUBJECT")
                    metaData.setSubject(value);

                else if (subCommand == "META_KEYWORDS")
                    metaData.setKeywords(value);



                else if (subCommand == "JOB_TITLE")
                    title = value;

                else if (subCommand == "JOB_PAGES")
                    pagesSpec = BooFile::PageSpec::readPagesSpec(value);

                else
                    qWarning() << QString("Unknown command '%1' in the line '%2'").arg(subCommand).arg(line);

            }
            // Boomaga commands ................................


            // PDF stream ......................................
            else if (command == "ENTER")
            {
                QString subCommand = line.section(' ', 2, -1, QString::SectionSkipEmpty).remove(' ').toUpper();

                if (subCommand == "LANGUAGE=PDF")
                {
                    qint64 startPos = booFile.pos();
                    qint64 endPos = 0;
                    while (!booFile.atEnd())
                    {
                        QByteArray buf = booFile.readLine();
                        if (buf.startsWith("\x1B%-12345X@PJL"))
                            break;

                        endPos = booFile.pos() - 1;
                    }

                    Job job = readPdf(mFileName, startPos, endPos);
                    job.setTitle(title);

                    // Copy pages from spec
                    if (!pagesSpec.isEmpty()) {
                        QList<ProjectPage*> pages = job.takeAllPages();
                        int cnt = pages.count();
                        for(const BooFile::PageSpec &spec: pagesSpec) {

                            // Some is wrong, skip pageSpec
                            if (spec.pageNum >= cnt)
                            {
                                qWarning() << QString("Page %1 out of range 1..%3")
                                              .arg(spec.pageNum+1).arg(cnt);
                                continue;
                            }

                            ProjectPage *page;
                            if (spec.pageNum < 0)
                                page = new ProjectPage();               // Blank page
                            else
                                page = pages.at(spec.pageNum)->clone(); // Normal page

                            if (spec.hidden)
                                page->setVisible(false);

                            if (spec.startBooklet)
                                page->setManualStartSubBooklet(true);

                            page->setManualRotation(spec.rotation);
                            job.addPage(page);
                        }
                    }


                    emit jobReady(job, mId, num);
                    num++;
                    title.clear();
                    pagesSpec.clear();
                }
            }
            // PDF stream ......................................
        }
    }
    booFile.close();

    if (!metaData.isEmpty())
        emit metaDataReady(metaData);
}


struct CupsOptions {
    CupsOptions(const QString &str);
    QVector<int> pages;

private:
    void parsePages(const QString &value);
};


/************************************************
 *
 ************************************************/
CupsOptions::CupsOptions(const QString &str)
{
    foreach (const QString s, str.split(' ', QString::SkipEmptyParts))
    {
        QString key = s.section("=", 0, 0);
        QString val = s.section("=", 1);
        if (key == "page-ranges")
        {
            parsePages(val);
            continue;
        }
    }
}


/************************************************
 *
 ************************************************/
void CupsOptions::parsePages(const QString &value)
{
    QStringList items = value.split(',');
    foreach (QString s, items)
    {
        bool ok;

        int fromPage = s.section("-", 0, 0).toInt(&ok)-1;
        if (!ok)
        {
            qWarning() << QString("Wrong page number '%1' in page spec '%2'")
                          .arg(s.section("-", 0, 0))
                          .arg(value);
            continue;
        }

        int toPage = s.section("-", 1).toInt(&ok)-1;
        if (!ok)
            toPage = fromPage;

        for (int p = fromPage; p<=toPage; ++p)
            pages << p;
    }
}



/************************************************
 *
 ************************************************/
void FileLoaderWorker::loadCupsBoo(QFile &inFile)
{

    QString title;
    QString options;
    int count =1;

    while (!inFile.atEnd())
    {
         QByteArray line = inFile.readLine().trimmed();

         if (line.startsWith("TITLE=")) {
             title = QUrl::fromPercentEncoding(line).mid(6);
             continue;
         }

         if (line.startsWith("OPTIONS=")) {
             options = QUrl::fromPercentEncoding(line).mid(8);
             continue;
         }

         if (line.startsWith("COUNT=")) {
             int n =  QString(line).mid(6).toInt();
             if (n) count = n;
             continue;
         }

         if (line.startsWith("CUPS_BOOMAGA_DATA")) {
             break;
         }
    }

    CupsOptions opts(options);
    auto startPos = inFile.pos();
    QByteArray line = inFile.readLine().trimmed();

    Job job;

    // Read PDF ..................................
    if (line.startsWith("%PDF-")) {
        job = readPdf(inFile.fileName(), startPos);
    }

    // Read PostScript ...........................
    else if (line.startsWith("%!PS-Adobe-")) {
        job = readPostScript(inFile, inFile.size());
    }

    // Unknown format ............................
    else {
        throw BoomagaError(tr("I can't read file \"%1\" either because it's not a supported file type, "
                              "or because the file has been damaged.").arg(mFileName));
    }

    job.setTitle(title);

    if (!opts.pages.isEmpty()) {
        // Specified pages
        const QList<ProjectPage*> pages = job.takeAllPages();
        for (int i: opts.pages) {
            if (i > -1 && i < pages.count()) {
                job.addPage(pages.at(i)->clone());
            }
        }

        qDeleteAll(pages);

    }

    emit jobReady(job, mId, 0);
    for (int i=1; i<count; ++i) {
        emit jobReady(job.clone(), mId, i);
    }
}
