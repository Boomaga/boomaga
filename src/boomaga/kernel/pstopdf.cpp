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


