/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2013 Boomaga team https://github.com/Boomaga
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


#include "printer.h"
#include "settings.h"

#include <QProcess>
#include <QSharedData>
#include <cups/cups.h>
#include <cups/ppd.h>
#include <QFileInfo>
#include <QDebug>
#include <QTemporaryFile>
#include <QDir>
#include <QCoreApplication>

#define CUPS_DEVICE_URI                  "device-uri"

#ifndef CUPS_SIDES
#  define CUPS_SIDES                     "sides"
#endif

#ifndef CUPS_SIDES_ONE_SIDED
#  define CUPS_SIDES_ONE_SIDED           "one-sided"
#endif

#ifndef CUPS_SIDES_TWO_SIDED_PORTRAIT
#  define CUPS_SIDES_TWO_SIDED_PORTRAIT  "two-sided-long-edge"
#endif

#ifndef CUPS_SIDES_TWO_SIDED_LANDSCAPE
#  define CUPS_SIDES_TWO_SIDED_LANDSCAPE "two-sided-short-edge"
#endif


#define A4_HEIGHT_MM    297
#define A4_HEIGHT_PT    842
#define A4_WIDTH_MM     210
#define A4_WIDTH_PT     595

#define MM_TO_PT    (A4_HEIGHT_PT * 1.0 / A4_HEIGHT_MM)
#define PT_TO_MM    (A4_HEIGHT_MM * 1.0 / A4_HEIGHT_PT)


/************************************************

 ************************************************/
qreal toUnit(qreal value, Printer::Unit unit)
{
    switch (unit)
    {
    case Printer::Point:
        return value;

    case Printer::Millimeter:
        return value * PT_TO_MM;
    }
    return 0;
}


/************************************************

 ************************************************/
qreal fromUnit(qreal value, Printer::Unit unit)
{
    switch (unit)
    {
    case Printer::Point:
        return value;

    case Printer::Millimeter:
        return value * MM_TO_PT;
    }
    return 0;
}


/************************************************

 ************************************************/
Printer::Printer()
{
    init();
}


/************************************************

 ************************************************/
Printer::Printer(QPrinterInfo printerInfo):
    mPrinterInfo(printerInfo)
{
    init();
    initFromCups();
    readSettings();
}


/************************************************

 ************************************************/
void Printer::init()
{
    mDeviceUri = "";

    mDuplexType = DuplexManualReverse;
    mReverseOrder = false;
    mPaperSize = QSizeF(A4_WIDTH_PT, A4_HEIGHT_PT);
    mLeftMargin = 0;
    mRightMargin = 0;
    mTopMargin = 0;
    mBottomMargin = 0;
    mInternalMargin = 14;
    mDrawBorder = false;
}


/************************************************

 ************************************************/
void Printer::initFromCups()
{
    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);
    cups_dest_t *dest = cupsGetDest(mPrinterInfo.printerName().toLocal8Bit().data(),
                                    0, num_dests, dests);

#if 0
    qDebug() << "**" << mPrinterInfo.printerName() << "*******************";
    for (int j=0; j<dest->num_options; ++j)
        qDebug() << "  *" << dest->options[j].name << dest->options[j].value;
#endif


    mDeviceUri = cupsGetOption(CUPS_DEVICE_URI, dest->num_options, dest->options);
    QString duplexStr = cupsGetOption(CUPS_SIDES, dest->num_options, dest->options);

    bool duplex = false;
    duplex = duplex || QString(cupsGetOption(CUPS_SIDES, dest->num_options, dest->options)).toUpper().startsWith("TWO-");
    duplex = duplex || QString(cupsGetOption("Duplex", dest->num_options, dest->options)).toUpper().startsWith("DUPLEX-");
    duplex = duplex || QString(cupsGetOption("JCLDuplex", dest->num_options, dest->options)).toUpper().startsWith("DUPLEX-");
    duplex = duplex || QString(cupsGetOption("EFDuplex", dest->num_options, dest->options)).toUpper().startsWith("DUPLEX-");
    duplex = duplex || QString(cupsGetOption("KD03Duplex", dest->num_options, dest->options)).toUpper().startsWith("DUPLEX-");

    // Read values from PPD
    // The returned filename is stored in a static buffer
    const char * ppdFile = cupsGetPPD(mPrinterInfo.printerName().toLocal8Bit().data());
    if (ppdFile != 0)
    {
        ppd_file_t *ppd = ppdOpenFile(ppdFile);
        if (ppd)
        {
            ppdMarkDefaults(ppd);

            ppd_size_t *size = ppdPageSize(ppd, 0);
            if (size)
            {
                mPaperSize = QSizeF(size->width, size->length);
                mLeftMargin   = size->left;
                mRightMargin  = size->width - size->right;
                mTopMargin    = size->length - size->top;
                mBottomMargin = size->bottom;
            }

            duplex = duplex || ppdIsMarked(ppd, "Duplex",     "DuplexNoTumble");
            duplex = duplex || ppdIsMarked(ppd, "Duplex",     "DuplexTumble");
            duplex = duplex || ppdIsMarked(ppd, "JCLDuplex",  "DuplexNoTumble");
            duplex = duplex || ppdIsMarked(ppd, "JCLDuplex",  "DuplexTumble");
            duplex = duplex || ppdIsMarked(ppd, "EFDuplex",   "DuplexNoTumble");
            duplex = duplex || ppdIsMarked(ppd, "EFDuplex",   "DuplexTumble");
            duplex = duplex || ppdIsMarked(ppd, "KD03Duplex", "DuplexNoTumble");
            duplex = duplex || ppdIsMarked(ppd, "KD03Duplex", "DuplexTumble");

            ppdClose(ppd);
        }
        QFile::remove(ppdFile);
    }

    if (duplex)
        mDuplexType = DuplexAuto;


    bool ok;
    int n;

    n = QString(cupsGetOption("page-left", dest->num_options, dest->options)).toInt(&ok);
    if (ok)
        mLeftMargin = n;

    n = QString(cupsGetOption("page-right", dest->num_options, dest->options)).toInt(&ok);
    if (ok)
        mRightMargin = n;

    n = QString(cupsGetOption("page-top", dest->num_options, dest->options)).toInt(&ok);
    if (ok)
        mTopMargin = n;

    n = QString(cupsGetOption("page-bottom", dest->num_options, dest->options)).toInt(&ok);
    if (ok)
        mBottomMargin = n;

    cupsFreeDests(num_dests, dests);
}


/************************************************

 ************************************************/
void Printer::readSettings()
{
    QString id = mPrinterInfo.printerName();
    mDuplexType     = static_cast<DuplexType>(settings->printerValue(id, Settings::Printer_DuplexType, mDuplexType).toInt());
    mDrawBorder     = settings->printerValue(id, Settings::Printer_DrawBorder,      mDrawBorder).toBool();
    mReverseOrder   = settings->printerValue(id, Settings::Printer_ReverseOrder,    mReverseOrder).toBool();

    mLeftMargin     = settings->printerValue(id, Settings::Printer_LeftMargin,      mLeftMargin).toDouble();
    mRightMargin    = settings->printerValue(id, Settings::Printer_RightMargin,     mRightMargin).toDouble();
    mTopMargin      = settings->printerValue(id, Settings::Printer_TopMargin,       mTopMargin).toDouble();
    mBottomMargin   = settings->printerValue(id, Settings::Printer_BottomMargin,    mBottomMargin).toDouble();
    mInternalMargin = settings->printerValue(id, Settings::Printer_InternalMargin,  mInternalMargin).toDouble();

}


/************************************************

 ************************************************/
void Printer::saveSettings()
{
    QString id = mPrinterInfo.printerName();
    settings->setPrinterValue(id, Settings::Printer_DuplexType,     mDuplexType);
    settings->setPrinterValue(id, Settings::Printer_DrawBorder,     mDrawBorder);
    settings->setPrinterValue(id, Settings::Printer_ReverseOrder,   mReverseOrder);

    settings->setPrinterValue(id, Settings::Printer_LeftMargin,     mLeftMargin);
    settings->setPrinterValue(id, Settings::Printer_RightMargin,    mRightMargin);
    settings->setPrinterValue(id, Settings::Printer_TopMargin,      mTopMargin);
    settings->setPrinterValue(id, Settings::Printer_BottomMargin,   mBottomMargin);
    settings->setPrinterValue(id, Settings::Printer_InternalMargin, mInternalMargin);
}


/************************************************

 ************************************************/
QString Printer::printerName() const
{
    return mPrinterInfo.printerName();
}


/************************************************

 ************************************************/
QSizeF Printer::paperSize(Printer::Unit unit) const
{
    return QSizeF(toUnit(mPaperSize.width(),  unit),
                  toUnit(mPaperSize.height(), unit));
}


/************************************************

 ************************************************/
void Printer::setPaperSize(const QSizeF &paperSize, Printer::Unit unit)
{
    mPaperSize.setWidth( fromUnit(paperSize.width(),  unit));
    mPaperSize.setHeight(fromUnit(paperSize.height(), unit));
}


/************************************************

 ************************************************/
QRectF Printer::paperRect(Printer::Unit unit) const
{
    return QRectF(QPointF(0, 0), paperSize(unit));
}


/************************************************

 ************************************************/
QRectF Printer::pageRect(Printer::Unit unit) const
{
    return paperRect(unit).adjusted(
                mLeftMargin,
                mTopMargin,
                -mRightMargin,
                -mBottomMargin);
}


/************************************************

 ************************************************/
qreal Printer::leftMargin(Printer::Unit unit)
{
    return toUnit(mLeftMargin, unit);
}


/************************************************

 ************************************************/
void Printer::setLeftMargin(qreal value, Printer::Unit unit)
{
    mLeftMargin = fromUnit(value, unit);
}


/************************************************

 ************************************************/
qreal Printer::rightMargin(Printer::Unit unit)
{
    return toUnit(mRightMargin, unit);
}


/************************************************

 ************************************************/
void Printer::setRightMargin(qreal value, Printer::Unit unit)
{
    mRightMargin = fromUnit(value, unit);
}


/************************************************

 ************************************************/
qreal Printer::topMargin(Printer::Unit unit)
{
    return toUnit(mTopMargin, unit);
}


/************************************************

 ************************************************/
void Printer::setTopMargin(qreal value, Printer::Unit unit)
{
    mTopMargin = fromUnit(value, unit);
}


/************************************************

 ************************************************/
qreal Printer::bottomMargin(Printer::Unit unit)
{
    return toUnit(mBottomMargin, unit);
}


/************************************************

 ************************************************/
void Printer::setBottomMargin(qreal value, Printer::Unit unit)
{
    mBottomMargin = fromUnit(value, unit);
}


/************************************************

 ************************************************/
qreal Printer::internalMarhin(Printer::Unit unit)
{
    return toUnit(mInternalMargin, unit);
}


/************************************************

 ************************************************/
void Printer::setInternalMargin(qreal value, Printer::Unit unit)
{
    mInternalMargin = fromUnit(value, unit);
}


/************************************************

 ************************************************/
void Printer::setDrawBorder(bool value)
{
    mDrawBorder = value;
}


/************************************************

 ************************************************/
void Printer::setDuplexType(DuplexType duplexType)
{
    mDuplexType = duplexType;
}


/************************************************

 ************************************************/
void Printer::setReverseOrder(bool value)
{
    mReverseOrder = value;
}


/************************************************

 ************************************************/
void Printer::print(const QList<Sheet *> &sheets, const QString &jobName, bool duplex, int numCopies) const
{
//#define DEBUG_PRINT
#ifdef DEBUG_PRINT

    QString fileName;
    {
        QTemporaryFile f;
        f.open();
        fileName = f.fileName();
        f.close();
    }

    project->writeDocument(sheets, fileName);

    QProcess::startDetached("okular", QStringList() << fileName);
#else
    QString file = QString("%1/.cache/boomaga_tmp_%2-print.pdf")
                          .arg(QDir::homePath())
                          .arg(QCoreApplication::applicationPid());

    project->writeDocument(sheets, file);

    QStringList args;
    args << "-P" << printerName();               // Prints files to the named printer.
    args << "-#" << QString("%1").arg(numCopies);// Sets the number of copies to print
    args << "-T" << jobName;                     // Sets the job name.
    args << "-r";                                // The print files should be deleted after printing them
    if (mDuplexType == DuplexAuto && !duplex)
        args << "-o sides=one-sided";            // Turn off duplex printing

    args << file.toLocal8Bit();

    QProcess proc;
    proc.startDetached("lpr", args);
#endif
}

