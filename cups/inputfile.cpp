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

#define BUF_SIZE 1024 * 1024 * 10


class InputFile
{
public:
    explicit InputFile(QFile &file, const QByteArray &prev);

    bool atEnd() { return mAtEnd; }
    QByteArray readLine();
private:
    QFile &mFile;
    QByteArray mBuffer;
    bool mNeedRead;
    int mStart;
    bool mAtEnd;
};



/************************************************
 *
 ************************************************/
InputFile::InputFile(QFile &file, const QByteArray &prev):
    mFile(file),
    mBuffer(prev),
    mNeedRead(true),
    mStart(0),
    mAtEnd(false)
{
}


/************************************************
 *
 ************************************************/
QByteArray InputFile::readLine()
{
    if (mNeedRead)
    {
        mBuffer.append(mFile.read(BUF_SIZE));
        mNeedRead = false;
    }

    int end = mBuffer.indexOf('\n', mStart) + 1;
    if (end)
    {
        int n = mStart;
        mStart = end;
        if (mFile.atEnd())
            mAtEnd = (end == mBuffer.length());

        return  mBuffer.mid(n, (end - n));

    }
    else
    {
        if (mFile.atEnd())
        {
            mAtEnd = true;
            return mBuffer.right(mBuffer.length() - mStart);
        }

        mBuffer = mBuffer.right(mBuffer.length() - mStart);
        mStart = 0;
        mNeedRead = true;
        return readLine();
    }
}


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
    QFile out;
    int fileNum = 0;

    bool prevIsEOF = true;

    InputFile file(in, buf);
    while (!file.atEnd())
    {
        QByteArray line = file.readLine();
        if (prevIsEOF && line.startsWith("%PDF-"))
        {
            if (out.isOpen())
                out.close();

            fileNum++;
            openOutFile(outDir, jobId, fileNum, "pdf", &out);
            res << out.fileName();
        }

        prevIsEOF = line.startsWith("%%EOF");
        out.write(line);
    }
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

    QFile ps;
    openOutFile(outDir, jobId, 1, "ps", &ps);
    ps.write(buf);
    while (!in.atEnd())
        ps.write(in.read(BUF_SIZE));
    ps.close();


    QStringList args;
    args << "-dSAFER";              // Restricts file operations the job can perform. Strongly recommended
    args << "-dNOPAUSE";            // Disables the prompt and pause at the end of each page.
    args << "-dBATCH";              // Causes Ghostscript to exit after processing all files
    args << "-sDEVICE=pdfwrite";    // Write to PDF
    //args << "-sstdout=%stderr";   // Redirect stdout to stderr
    args << QString("-sOutputFile=%1").arg(outFileName).toLocal8Bit();
    args << "-c" << ".setpdfwrite";
    args << "-f" << ps.fileName().toLocal8Bit();
    debug(QString("Start gs: '%1'").arg(args.join("', '")));

    QProcess proc;
    proc.start("gs", args);
    proc.waitForStarted();
    proc.waitForFinished(-1);
    ps.remove();

    if (proc.exitStatus() == 0 && proc.exitCode() == 0)
    {
        debug("Convert PostScript to PDF: gs was successfully finished");
    }
    else
    {
        QString msg = QString::fromLocal8Bit(proc.readAllStandardError());
        error(QString("Convert PostScript to PDF: gs exit with code %1: %2")
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
