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


#ifndef PRINTER_H
#define PRINTER_H

#include "boomagatypes.h"
#include <QObject>
#include <QVector>
#include <QString>
#include <QPrinterInfo>
#include <QExplicitlySharedDataPointer>
#include <QIODevice>

class Sheet;


class PrinterProfile
{
public:
    explicit PrinterProfile();
    PrinterProfile &operator=(const PrinterProfile &other);

    QString name() const { return mName; }
    void setName(const QString &name);

    qreal leftMargin(Unit unit=UnitPoint) const;
    void setLeftMargin(qreal value, Unit unit);

    qreal rightMargin(Unit unit=UnitPoint) const;
    void setRightMargin(qreal value, Unit unit);

    qreal topMargin(Unit unit=UnitPoint) const;
    void setTopMargin(qreal value, Unit unit);

    qreal bottomMargin(Unit unit=UnitPoint) const;
    void setBottomMargin(qreal value, Unit unit);

    qreal internalMargin(Unit unit=UnitPoint) const;
    void setInternalMargin(qreal value, Unit unit);

    DuplexType duplexType() const { return mDuplexType; }
    void setDuplexType(DuplexType duplexType);

    bool drawBorder() const { return mDrawBorder; }
    void setDrawBorder(bool value);

    bool reverseOrder() const { return mReverseOrder; }
    void setReverseOrder(bool value);

    QSizeF paperSize(Unit unit) const;
    void setPaperSize(const QSizeF & paperSize, Unit unit);

    QString grayscaleOption() const { return mGrayscaleOption; }
    void setGrayscaleOption(const QString &value);

    void readSettings();
    void saveSettings() const;

private:
    QString mName;
    qreal mLeftMargin;
    qreal mRightMargin;
    qreal mTopMargin;
    qreal mBottomMargin;
    qreal mInternalMargin;
    DuplexType mDuplexType;
    bool mDrawBorder;
    bool mReverseOrder;
    QSizeF mPaperSize;
    QString mGrayscaleOption;
};


class Printer
{
public:
    explicit Printer(const QString &name);
    virtual ~Printer();

    virtual QString name() const { return mPrinterName; }

    QVector<PrinterProfile> *profiles() { return &mProfiles; }
    const QVector<PrinterProfile> *profiles() const { return &mProfiles; }

    int currentProfile() const { return mCurrentProfileIndex; }
    void setCurrentProfile(int index);

    const PrinterProfile *cupsProfile() const { return &mCupsProfile; }

    QSizeF paperSize(Unit unit) const { return mCurrentProfile->paperSize(unit); }
    void setPaperSize(const QSizeF & paperSize, Unit unit) { mCurrentProfile->setPaperSize(paperSize, unit); }

    QRectF paperRect(Unit unit=UnitPoint) const;
    QRectF pageRect(Unit unit=UnitPoint) const;

    qreal leftMargin(Unit unit=UnitPoint) const { return mCurrentProfile->leftMargin(unit); }
    void setLeftMargin(qreal value, Unit unit)  { mCurrentProfile->setLeftMargin(value, unit); }

    qreal rightMargin(Unit unit=UnitPoint)      { return mCurrentProfile->rightMargin(unit); }
    void setRightMargin(qreal value, Unit unit) { mCurrentProfile->setRightMargin(value, unit); }

    qreal topMargin(Unit unit=UnitPoint)        { return mCurrentProfile->topMargin(unit); }
    void setTopMargin(qreal value, Unit unit)   { mCurrentProfile->setTopMargin(value, unit); }

    qreal bottomMargin(Unit unit=UnitPoint)     { return mCurrentProfile->bottomMargin(unit); }
    void setBottomMargin(qreal value, Unit unit){ mCurrentProfile->setBottomMargin(value, unit); }

    qreal internalMarhin(Unit unit=UnitPoint)   { return mCurrentProfile->internalMargin(unit); }
    void setInternalMargin(qreal value, Unit unit) { mCurrentProfile->setInternalMargin(value, unit); }

    bool drawBorder() const        { return mCurrentProfile->drawBorder(); }
    void setDrawBorder(bool value) { mCurrentProfile->setDrawBorder(value); }

    DuplexType duplexType() const             { return mCurrentProfile->duplexType(); }
    void setDuplexType(DuplexType duplexType) { mCurrentProfile->setDuplexType(duplexType); }

    bool reverseOrder() const        { return mCurrentProfile->reverseOrder(); }
    void setReverseOrder(bool value) { mCurrentProfile->setReverseOrder(value); }

    bool canChangeDuplexType() const { return mCanChangeDuplexType; }
    bool isShowProgressDialog() const { return mShowProgressDialog; }


    virtual bool print(const QList<Sheet*> &sheets, const QString &jobName, bool duplex, int numCopies, bool collate) const;

    QString deviceUri() const { return mDeviceUri; }

    void readSettings();
    void saveSettings();

    static QList<Printer*> availablePrinters();
    static Printer *printerByName(const QString &name);
    static Printer *nullPrinter();

protected:
    bool mCanChangeDuplexType;
    bool mShowProgressDialog;

private:
    const QString mPrinterName;
    QString mDeviceUri;
    QVector<PrinterProfile> mProfiles;
    int mCurrentProfileIndex;
    PrinterProfile *mCurrentProfile;
    PrinterProfile mCupsProfile;
};

#endif // PRINTER_H
