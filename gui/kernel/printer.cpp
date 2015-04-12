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
#include <QFileDialog>

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
qreal toUnit(qreal value, Unit unit)
{
    switch (unit)
    {
    case UnitPoint:
        return value;

    case UnitMillimeter:
        return value * PT_TO_MM;
    }
    return 0;
}


/************************************************

 ************************************************/
qreal fromUnit(qreal value, Unit unit)
{
    switch (unit)
    {
    case UnitPoint:
        return value;

    case UnitMillimeter:
        return value * MM_TO_PT;
    }
    return 0;
}



/************************************************

 ************************************************/
void initFromCups(const QString &printerName, QString *deviceUri, PrinterProfile *profile)
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



    *deviceUri = QString(cupsGetOption(CUPS_DEVICE_URI, dest->num_options, dest->options));
    QString duplexStr = cupsGetOption(CUPS_SIDES, dest->num_options, dest->options);

    bool duplex = false;
    duplex = duplex || QString(cupsGetOption(CUPS_SIDES, dest->num_options, dest->options)).toUpper().startsWith("TWO-");
    duplex = duplex || QString(cupsGetOption("Duplex", dest->num_options, dest->options)).toUpper().startsWith("DUPLEX-");
    duplex = duplex || QString(cupsGetOption("JCLDuplex", dest->num_options, dest->options)).toUpper().startsWith("DUPLEX-");
    duplex = duplex || QString(cupsGetOption("EFDuplex", dest->num_options, dest->options)).toUpper().startsWith("DUPLEX-");
    duplex = duplex || QString(cupsGetOption("KD03Duplex", dest->num_options, dest->options)).toUpper().startsWith("DUPLEX-");

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
                profile->setPaperSize(QSizeF(size->width, size->length), UnitPoint);
                profile->setLeftMargin(size->left, UnitPoint);
                profile->setRightMargin(size->width - size->right, UnitPoint);
                profile->setTopMargin(size->length - size->top, UnitPoint);
                profile->setBottomMargin(size->bottom, UnitPoint);
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
        profile->setDuplexType(DuplexAuto);

    bool ok;
    int n;

    n = QString(cupsGetOption("page-left", dest->num_options, dest->options)).toInt(&ok);
    if (ok)
        profile->setLeftMargin(n, UnitPoint);

    n = QString(cupsGetOption("page-right", dest->num_options, dest->options)).toInt(&ok);
    if (ok)
        profile->setRightMargin(n, UnitPoint);

    n = QString(cupsGetOption("page-top", dest->num_options, dest->options)).toInt(&ok);
    if (ok)
        profile->setTopMargin(n, UnitPoint);

    n = QString(cupsGetOption("page-bottom", dest->num_options, dest->options)).toInt(&ok);
    if (ok)
        profile->setBottomMargin(n, UnitPoint);

    cupsFreeDests(num_dests, dests);
}


/************************************************

 ************************************************/
PrinterProfile::PrinterProfile():
    mLeftMargin(0),
    mRightMargin(0),
    mTopMargin(0),
    mBottomMargin(0),
    mInternalMargin(14),
    mDuplexType(DuplexManualReverse),
    mDrawBorder(false),
    mReverseOrder(false),
    mPaperSize(QSizeF(A4_WIDTH_PT, A4_HEIGHT_PT))
{

}


/************************************************
 *
 ************************************************/
PrinterProfile &PrinterProfile::operator=(const PrinterProfile &other)
{
    mName           = other.mName;
    mLeftMargin     = other.mLeftMargin;
    mRightMargin    = other.mRightMargin;
    mTopMargin      = other.mTopMargin;
    mBottomMargin   = other.mBottomMargin;
    mInternalMargin = other.mInternalMargin;
    mDuplexType     = other.mDuplexType;
    mDrawBorder     = other.mDrawBorder;
    mReverseOrder   = other.mReverseOrder;
    mPaperSize      = other.mPaperSize;

    return *this;
}


/************************************************
 *
 ************************************************/
void PrinterProfile::setName(const QString &name)
{
    mName = name;
}


/************************************************

 ************************************************/
qreal PrinterProfile::leftMargin(Unit unit) const
{
    return toUnit(mLeftMargin, unit);
}


/************************************************

 ************************************************/
void PrinterProfile::setLeftMargin(qreal value, Unit unit)
{
    mLeftMargin = fromUnit(value, unit);
}


/************************************************

 ************************************************/
qreal PrinterProfile::rightMargin(Unit unit) const
{
    return toUnit(mRightMargin, unit);
}


/************************************************

 ************************************************/
void PrinterProfile::setRightMargin(qreal value, Unit unit)
{
    mRightMargin = fromUnit(value, unit);
}


/************************************************

 ************************************************/
qreal PrinterProfile::topMargin(Unit unit) const
{
    return toUnit(mTopMargin, unit);
}


/************************************************

 ************************************************/
void PrinterProfile::setTopMargin(qreal value, Unit unit)
{
    mTopMargin = fromUnit(value, unit);
}


/************************************************

 ************************************************/
qreal PrinterProfile::bottomMargin(Unit unit) const
{
    return toUnit(mBottomMargin, unit);
}


/************************************************

 ************************************************/
void PrinterProfile::setBottomMargin(qreal value, Unit unit)
{
    mBottomMargin = fromUnit(value, unit);
}


/************************************************

 ************************************************/
qreal PrinterProfile::internalMargin(Unit unit) const
{
    return toUnit(mInternalMargin, unit);
}


/************************************************

 ************************************************/
void PrinterProfile::setInternalMargin(qreal value, Unit unit)
{
    mInternalMargin = fromUnit(value, unit);
}


/************************************************
 *
 ************************************************/
void PrinterProfile::setDuplexType(DuplexType duplexType)
{
    mDuplexType = duplexType;
}


/************************************************
 *
 ************************************************/
void PrinterProfile::setDrawBorder(bool value)
{
    mDrawBorder = value;
}


/************************************************
 *
 ************************************************/
void PrinterProfile::setReverseOrder(bool value)
{
    mReverseOrder = value;
}


/************************************************

 ************************************************/
QSizeF PrinterProfile::paperSize(Unit unit) const
{
    return QSizeF(toUnit(mPaperSize.width(),  unit),
                  toUnit(mPaperSize.height(), unit));
}


/************************************************

     ************************************************/
void PrinterProfile::setPaperSize(const QSizeF &paperSize, Unit unit)
{
    mPaperSize.setWidth( fromUnit(paperSize.width(),  unit));
    mPaperSize.setHeight(fromUnit(paperSize.height(), unit));
}


/************************************************

 ************************************************/
void PrinterProfile::readSettings()
{
    mName           = settings->value(Settings::PrinterProfile_Name,            mName).toString();
    mLeftMargin     = settings->value(Settings::PrinterProfile_LeftMargin,      mLeftMargin).toDouble();
    mRightMargin    = settings->value(Settings::PrinterProfile_RightMargin,     mRightMargin).toDouble();
    mTopMargin      = settings->value(Settings::PrinterProfile_TopMargin,       mTopMargin).toDouble();
    mBottomMargin   = settings->value(Settings::PrinterProfile_BottomMargin,    mBottomMargin).toDouble();
    mInternalMargin = settings->value(Settings::PrinterProfile_InternalMargin,  mInternalMargin).toDouble();

    QString s = settings->value(Settings::PrinterProfile_DuplexType, duplexTypetoStr(mDuplexType)).toString();
    mDuplexType = strToDuplexType(s);

    mDrawBorder     = settings->value(Settings::PrinterProfile_DrawBorder,      mDrawBorder).toBool();
    mReverseOrder   = settings->value(Settings::PrinterProfile_ReverseOrder,    mReverseOrder).toBool();
}


/************************************************

 ************************************************/
void PrinterProfile::saveSettings() const
{
    settings->setValue(Settings::PrinterProfile_Name,           mName);
    settings->setValue(Settings::PrinterProfile_LeftMargin,     mLeftMargin);
    settings->setValue(Settings::PrinterProfile_RightMargin,    mRightMargin);
    settings->setValue(Settings::PrinterProfile_TopMargin,      mTopMargin);
    settings->setValue(Settings::PrinterProfile_BottomMargin,   mBottomMargin);
    settings->setValue(Settings::PrinterProfile_InternalMargin, mInternalMargin);

    settings->setValue(Settings::PrinterProfile_DuplexType,     duplexTypetoStr(mDuplexType));
    settings->setValue(Settings::PrinterProfile_DrawBorder,     mDrawBorder);
    settings->setValue(Settings::PrinterProfile_ReverseOrder,   mReverseOrder);
}


/************************************************

 ************************************************/
Printer::Printer(const QString &printerName):
    mCanChangeDuplexType(true),
    mShowProgressDialog(true),
    mPrinterName(printerName),
    mCurrentProfileIndex(-1),
    mCurrentProfile(0)
{
    initFromCups(mPrinterName, &mDeviceUri, &mCupsProfile);
    readSettings();
}


/************************************************
 *
 ************************************************/
Printer::~Printer()
{
}


/************************************************
 *
 ************************************************/
void Printer::setCurrentProfile(int index)
{
    mCurrentProfileIndex = index;
    mCurrentProfile = &mProfiles[mCurrentProfileIndex];
}


/************************************************

 ************************************************/
void Printer::readSettings()
{
    settings->beginGroup(QString("Printer_%1").arg(mPrinterName));
    mCurrentProfileIndex = settings->value(Settings::Printer_CurrentProfile, 0).toInt();
    int size = settings->beginReadArray("Profiles");
    for (int i=0; i<size; ++i)
    {
        PrinterProfile profile;
        profile = mCupsProfile;

        settings->setArrayIndex(i);
        profile.readSettings();
        mProfiles.append(profile);
    }
    settings->endArray();

    if (mProfiles.isEmpty())
    {
        PrinterProfile profile;
        profile = mCupsProfile;

        profile.readSettings();
        profile.setName(QObject::tr("Default", "Printer profile default name"));
        mProfiles.append(profile);
    }
    settings->endGroup();

    if (mCurrentProfileIndex < 0 || mCurrentProfileIndex >= mProfiles.count())
        mCurrentProfileIndex = 0;

    mCurrentProfile = &mProfiles[mCurrentProfileIndex];
}


/************************************************

 ************************************************/
void Printer::saveSettings()
{
    settings->beginGroup(QString("Printer_%1").arg(mPrinterName));
    settings->setValue(Settings::Printer_CurrentProfile, mCurrentProfileIndex);

    settings->beginWriteArray("Profiles");
    for (int i=0; i<mProfiles.count(); ++i)
    {
        settings->setArrayIndex(i);
        mProfiles[i].saveSettings();
    }
    settings->endArray();
    settings->endGroup();
}


/************************************************

 ************************************************/
QList<Printer*> Printer::availablePrinters()
{
    static QList<Printer*>* printers = 0;

    if (!printers)
    {
        printers = new QList<Printer*>;

        QList<QPrinterInfo> piList = QPrinterInfo::availablePrinters();
        foreach (const QPrinterInfo &pi, piList)
        {
            Printer *printer = new Printer(pi.printerName());
            if (printer->deviceUri() != CUPS_BACKEND_URI)
                *printers << printer;
            else
                delete printer;
        }
    }

    return QList<Printer *>(*printers);
}


/************************************************
 *
 ************************************************/
Printer *Printer::printerByName(const QString &printerName)
{
    foreach(Printer *printer, availablePrinters())
    {
        if (printer->name() == printerName)
            return printer;
    }

    return 0;
}


/************************************************
 *
 ************************************************/
Printer *Printer::nullPrinter()
{
    static Printer nullPrinter = Printer("Fake");
    return &nullPrinter;
}


/************************************************

 ************************************************/
QRectF Printer::paperRect(Unit unit) const
{
    return QRectF(QPointF(0, 0), paperSize(unit));
}


/************************************************

 ************************************************/
QRectF Printer::pageRect(Unit unit) const
{
    return paperRect(unit).adjusted(
                mCurrentProfile->leftMargin(),
                mCurrentProfile->topMargin(),
                -mCurrentProfile->rightMargin(),
                -mCurrentProfile->bottomMargin());
}


/************************************************

 ************************************************/
bool Printer::print(const QList<Sheet *> &sheets, const QString &jobName, bool duplex, int numCopies) const
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
    args << "-P" << name();                       // Prints files to the named printer.
    args << "-#" << QString("%1").arg(numCopies); // Sets the number of copies to print
    args << "-T" << jobName;                      // Sets the job name.
    args << "-r";                                 // The print files should be deleted after printing them
    if (duplexType() == DuplexAuto && !duplex)
        args << "-o sides=one-sided";             // Turn off duplex printing

    args << file.toLocal8Bit();

    QProcess proc;
    proc.startDetached("lpr", args);
    return true;
#endif
}

