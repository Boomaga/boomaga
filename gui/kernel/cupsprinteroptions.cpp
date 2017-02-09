/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2017 Boomaga team https://github.com/Boomaga
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


#include "cupsprinteroptions.h"
#include <cups/cups.h>
#include <cups/ppd.h>
#include <QFile>
#include <QStringList>

#define CUPS_DEVICE_URI                  "device-uri"

#ifndef CUPS_SIDES
#  define CUPS_SIDES                     "sides"
#endif


void findGrayScaleOption(ppd_file_t *ppd, QString *grayScaleOption, QString *colorOption)
{
    QStringList cases;
    cases << "ColorModel"       << "Gray"       << "CMYK";
    cases << "HPColorMode"      << "grayscale"  << "colorsmart";
    cases << "BRMonoColor"      << "Mono"       << "FullColor";     // Brother
    cases << "CNIJSGrayScale"   << "1"          << "0";             //
    cases << "HPColorAsGray"    << "True"       << "False";         // HP
    cases << "XRColorMode"      << "Black"      << "Color";         // Xerox


    for (int i=0; i<cases.count(); i+=3)
    {
        ppd_option_t *option  = ppdFindOption(ppd, cases[i].toLatin1().data());
        if (option && ppdFindChoice(option, cases[i+1].toLatin1().data()))
        {
            *grayScaleOption = QString("%1=%2").arg(cases[i], cases[i+1]);
            *colorOption     = QString("%1=%2").arg(cases[i], cases[i+2]);
            return;
        }
    }
}


CupsPrinterOptions::CupsPrinterOptions(const QString &printerName):
    mDuplex(false),
    mPaperSize(QSizeF(0,0)),
    mLeftMargin(0),
    mRightMargin(0),
    mTopMargin(0),
    mBottomMargin(0)
{
    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);
    cups_dest_t *dest = cupsGetDest(printerName.toLocal8Bit().data(),
                                    0, num_dests, dests);

    if (!dest)
        return;

#if 0
    qDebug() << "**" << mPrinterInfo.printerName() << "*******************";
    for (int j=0; j<dest->num_options; ++j)
        qDebug() << "  *" << dest->options[j].name << dest->options[j].value;
#endif



    mDeviceURI = QString(cupsGetOption(CUPS_DEVICE_URI, dest->num_options, dest->options));
    //QString duplexStr = cupsGetOption(CUPS_SIDES, dest->num_options, dest->options);

    mDuplex = false;
    mDuplex = mDuplex || QString(cupsGetOption(CUPS_SIDES, dest->num_options, dest->options)).toUpper().startsWith("TWO-");
    mDuplex = mDuplex || QString(cupsGetOption("Duplex", dest->num_options, dest->options)).toUpper().startsWith("DUPLEX-");
    mDuplex = mDuplex || QString(cupsGetOption("JCLDuplex", dest->num_options, dest->options)).toUpper().startsWith("DUPLEX-");
    mDuplex = mDuplex || QString(cupsGetOption("EFDuplex", dest->num_options, dest->options)).toUpper().startsWith("DUPLEX-");
    mDuplex = mDuplex || QString(cupsGetOption("KD03Duplex", dest->num_options, dest->options)).toUpper().startsWith("DUPLEX-");

    // Read values from PPD
    // The returned filename is stored in a static buffer
    const char * ppdFile = cupsGetPPD(printerName.toLocal8Bit().data());
    if (ppdFile != 0)
    {
        ppd_file_t *ppd = ppdOpenFile(ppdFile);
        if (ppd)
        {
            ppdMarkDefaults(ppd);

            ppd_size_t *size = ppdPageSize(ppd, 0);
            if (size)
            {
                mPaperSize    = QSizeF(size->width, size->length);
                mLeftMargin   = size->left;
                mRightMargin  = size->width;
                mTopMargin    = size->length - size->top;
                mBottomMargin = size->bottom;
            }

            mDuplex = mDuplex || ppdIsMarked(ppd, "Duplex",     "DuplexNoTumble");
            mDuplex = mDuplex || ppdIsMarked(ppd, "Duplex",     "DuplexTumble");
            mDuplex = mDuplex || ppdIsMarked(ppd, "JCLDuplex",  "DuplexNoTumble");
            mDuplex = mDuplex || ppdIsMarked(ppd, "JCLDuplex",  "DuplexTumble");
            mDuplex = mDuplex || ppdIsMarked(ppd, "EFDuplex",   "DuplexNoTumble");
            mDuplex = mDuplex || ppdIsMarked(ppd, "EFDuplex",   "DuplexTumble");
            mDuplex = mDuplex || ppdIsMarked(ppd, "KD03Duplex", "DuplexNoTumble");
            mDuplex = mDuplex || ppdIsMarked(ppd, "KD03Duplex", "DuplexTumble");


            // Grayscale options ..........................
            findGrayScaleOption(ppd, &mGrayScaleOption, &mColorOption);

            ppdClose(ppd);
        }
        QFile::remove(ppdFile);
    }


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
        mBottomMargin =n;

    cupsFreeDests(num_dests, dests);
}




