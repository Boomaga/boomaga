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


#include "pstopdf.h"
#include "boomagatypes.h"
#include <QString>
#include <QDebug>

#include <QProcess>
#include <fstream>


using namespace std;

/************************************************
 *
 ************************************************/
PsToPdf::PsToPdf(QObject *parent):
    QObject(parent)
{

}


/************************************************
 *
 ************************************************/
PsToPdf::~PsToPdf()
{

}


/************************************************
 *
 ************************************************/
void PsToPdf::execute(const string &psFile, const string &pdfFile)
{
    ifstream in(psFile);
    if (!in.is_open())
        throw BoomagaError(QObject::tr("I can't open file \"%1\"").arg(psFile.c_str())
                           + "\n" + strerror(errno));

    PsToPdf converter;
    converter.execute(in, pdfFile);
}


/************************************************
 *
 ************************************************/
void PsToPdf::execute(ifstream &psStream, const string &pdfFile)
{
    QStringList args;
    args << "-dNOPAUSE";
    args << "-dBATCH";
    args << "-dSAFER";
    args << "-sDEVICE=pdfwrite";
    args << "-sOutputFile=" + QString::fromStdString(pdfFile);
    args << "-q";
    args << "-c" << ".setpdfwrite";
    args << "-f" << "-";


    mProcess.start("gs", args, QProcess::ReadWrite);
    if (!mProcess.waitForStarted())
        throw BoomagaError(tr("I can't start gs converter: \"%1\"")
                           .arg("timeout"));


    const size_t BUF_SIZE= 4096 * 1024;
    char buf[BUF_SIZE];
    while (psStream)
    {
        psStream.read(buf, BUF_SIZE);
        mProcess.write(buf, psStream.gcount());
    }

    mProcess.closeWriteChannel();

    while (!mProcess.waitForFinished(5))
    {
        qApp->processEvents();
    }

    if (mProcess.exitCode() != 0)
        throw BoomagaError(tr("I can't start gs converter: \"%1\"")
                           .arg(QString::fromLocal8Bit(mProcess.readAllStandardError())));
}


/************************************************
 *
 ************************************************/
void PsToPdf::terminate()
{
     mProcess.terminate();
    if (mProcess.waitForFinished(1000))
        return;

    mProcess.kill();
    mProcess.waitForFinished(-1);
}


