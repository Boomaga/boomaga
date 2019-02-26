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


#include "postscriptfile.h"
#include "pdffile.h"
#include "boomagatypes.h"

#include <QProcess>
#include <QFile>

/************************************************
 *
 ************************************************/
PostScriptFile::PostScriptFile(QObject *parent) :
    InFile(parent)
{
}


/************************************************
 *
 ************************************************/
void PostScriptFile::read()
{
    QFile psFile;
    mustOpenFile(mFileName, &psFile);
    psFile.seek(mStartPos);


    QString pdfFileName = genTmpFileName("in.pdf");
    emit startLongOperation(tr("Converting PostScript to PDF", "Progressbar text"));
    convertToPdf(psFile, pdfFileName);
    emit endLongOperation();

    PdfFile pdf;
    pdf.load(pdfFileName);
    mJobs << pdf.jobs();
}


/************************************************
 *
 ************************************************/
void PostScriptFile::convertToPdf(QFile &psFile, const QString &pdfFile)
{
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
    if (!process.waitForStarted())
        throw BoomagaError(tr("I can't start gs converter: \"%1\"")
                           .arg("timeout"));


    const qint64 BUF_SIZE= 4096 * 1024;
    while (!psFile.atEnd() && psFile.pos() < mEndPos)
    {
        process.write(
            psFile.read(qMin(BUF_SIZE, mEndPos - psFile.pos())));
    }

    process.closeWriteChannel();

    while (!process.waitForFinished(5))
    {
        qApp->processEvents();
    }

    if (process.exitCode() != 0)
        throw BoomagaError(tr("I can't start gs converter: \"%1\"")
                           .arg(QString::fromLocal8Bit(process.readAllStandardError())));
}
