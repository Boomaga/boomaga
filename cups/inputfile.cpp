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


#include "inputfile.h"
#include "common.h"
#include <QFile>
#include <QProcess>
#include <QDebug>

#define BUF_SIZE  10485760

/************************************************
 *
 ************************************************/
QString getOutFileName(const QString &dir, const QString &jobId, int fileNum,  const QString &ext)
{
    return QString("%1/boomaga_in_file_%2[%4].%3").arg(dir, jobId, ext).arg(fileNum, 2, 10, QChar('0'));
}

/************************************************
 *
 ************************************************/
void openOutFile(const QString &dir, const QString &jobId, int fileNum,  const QString &ext, QFile *out)
{
    QString name = getOutFileName(dir, jobId, fileNum, ext);
    out->setFileName(name);
    if(!out->open(QFile::WriteOnly))
        error(QString("Can't write to %1").arg(out->fileName()));
}


/************************************************
 * PDF
 * Issue #29
 * It seems like the CUPS actually does get all of the files,
 * but as multiple PDF files simply concatenated each after the other
 * (not as a real PDF file with two pages).
 ************************************************/
QStringList processPDF(QFile &in, QByteArray &buf, const QString &jobId, const QString &outDir)
{
    QStringList res;
    int fileNum = 1;
    QFile out;
    openOutFile(outDir, jobId, fileNum, "pdf", &out);
    res << out.fileName();

    QByteArray prep;
    out.write(buf);

    while (!in.atEnd())
    {

        buf = in.read(BUF_SIZE);
        if (prep.length())
            buf.prepend(prep);

        int start = 0;
        int pos = buf.indexOf("\n%PDF-", start);
        while (pos > -1)
        {
            out.write(buf.data() + start, pos - start);
            start = pos;

            out.close();
            fileNum++;
            openOutFile(outDir, jobId, fileNum, "pdf", &out);
            res << out.fileName();

            pos = buf.indexOf("\n%PDF-", start+1);
        }

        int end = buf.length();
        if (buf.endsWith("\n%PDF"))
        {
            prep = "\n%PDF";
            end -= 5;
        }
        else if (buf.endsWith("\n%PD"))
        {
            prep = "\n%PD";
            end -= 4;
        }
        else if (buf.endsWith("\n%P"))
        {
            prep = "\n%P";
            end -= 3;
        }
        else if (buf.endsWith("\n%"))
        {
            prep = "\n%";
            end -= 2;
        }
        else if (buf.endsWith("\n"))
        {
            prep = "\n";
            end -= 1;
        }
        else
        {
            prep = "";
        }

        out.write(buf.data() + start, end - start);
    }

    if (prep.length())
        out.write(prep);

    return res;

}



/************************************************
 * Convert .ps file to PDF
 * gs work faster with real file then STDIN, so we
 * save ps file before start gs.
 ************************************************/
QStringList processPostScript(QFile &in, QByteArray &buf, const QString &jobId, const QString &outDir)
{
    QString outFileName = getOutFileName(outDir, jobId, 1, "pdf");

    QFile out;
    openOutFile(outDir, jobId, 1, "ps", &out);
    out.write(buf);
    while (!in.atEnd())
        out.write(in.read(BUF_SIZE));
    out.close();


    QList<QString> args;
    args << "-dSAFER";              // Restricts file operations the job can perform. Strongly recommended
    args << "-dNOPAUSE";            // Disables the prompt and pause at the end of each page.
    args << "-dBATCH";              // Causes Ghostscript to exit after processing all files
    args << "-sDEVICE=pdfwrite";    // Write to PDF
    //args << "-sstdout=%stderr";   // Redirect stdout to stderr
    args << QString("-sOutputFile=%1").arg(outFileName).toLocal8Bit();
    args << "-c" << ".setpdfwrite";
    args << "-f" << out.fileName().toLocal8Bit();
    //qDebug() << args;

    QProcess proc;
    proc.start("gs", args);
    proc.waitForStarted();
    proc.waitForFinished(-1);

    if (proc.exitStatus() == 0 && proc.exitCode() == 0)
    {
        debug("gs was successfully finished");
    }
    else
    {
        QString msg = QString::fromLocal8Bit(proc.readAllStandardError());
        error(QString("gs exit with code %1: %2")
              .arg(proc.exitCode())
              .arg(msg));
    }

    return QStringList() << outFileName;
}


/************************************************
 *
 ************************************************/
QStringList createJobFiles(const QString &jobId, const QString &outDir)
{
    QFile in;
    if(!in.open(stdin, QFile::ReadOnly))
        error("Can't read from stdin");

    QByteArray buf = in.read(10);

    // PDF ......................................
    if (buf.startsWith("%PDF-"))
    {
        QStringList res = processPDF(in, buf, jobId, outDir);
        in.close();
        return res;
    }


    // PostScript ...............................
    if (buf.startsWith("%!PS-Adobe"))
    {
        return processPostScript(in, buf, jobId, outDir);
    }


    error("Unknown format of input file");
    return QStringList();
}
