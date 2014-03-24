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
#include <QList>
#include <QString>
#include <QPrinterInfo>
#include <QExplicitlySharedDataPointer>
#include <QIODevice>

class Sheet;

class Printer
{
public:

    enum Unit {
        Millimeter = 0,
        Point      = 1
        //Inch       = 2
    };

    //enum PaperSize { A4, B5, Letter, Legal, Executive,
    //                 A0, A1, A2, A3, A5, A6, A7, A8, A9, B0, B1,
    //                 B10, B2, B3, B4, B6, B7, B8, B9, C5E, Comm10E,
    //                 DLE, Folio, Ledger, Tabloid, Custom, NPageSize = Custom, NPaperSize = Custom };

    Printer();
    explicit Printer(QPrinterInfo printerInfo);
    virtual ~Printer();

    virtual QString printerName() const;

    QSizeF paperSize(Unit unit) const;

    void setPaperSize(const QSizeF & paperSize, Unit unit);

    QRectF paperRect(Unit unit=Point) const;

    QRectF pageRect(Unit unit=Point) const;

    qreal leftMargin(Unit unit=Point);
    void setLeftMargin(qreal value, Unit unit);

    qreal rightMargin(Unit unit=Point);
    void setRightMargin(qreal value, Unit unit);

    qreal topMargin(Unit unit=Point);
    void setTopMargin(qreal value, Unit unit);

    qreal bottomMargin(Unit unit=Point);
    void setBottomMargin(qreal value, Unit unit);

    qreal internalMarhin(Unit unit=Point);
    void setInternalMargin(qreal value, Unit unit);

    bool drawBorder() const { return mDrawBorder; }
    void setDrawBorder(bool value);

    DuplexType duplexType() const { return mDuplexType; }
    void setDuplexType(DuplexType duplexType);

    bool canChangeDuplexType() const { return mCanChangeDuplexType; }
    bool isShowProgressDialog() const { return mShowProgressDialog; }

    bool reverseOrder() const { return mReverseOrder; }
    void setReverseOrder(bool value);

    virtual bool print(const QList<Sheet*> &sheets, const QString &jobName, bool duplex, int numCopies = 1) const;

    QString deviceUri() const { return mDeviceUri; }

    void readSettings();
    void saveSettings();

    static QList<Printer*> availablePrinters();

protected:
    bool mCanChangeDuplexType;
    bool mShowProgressDialog;

private:
    QPrinterInfo mPrinterInfo;
    QString mDeviceUri;    QSizeF mPaperSize;
    qreal mLeftMargin;
    qreal mRightMargin;
    qreal mTopMargin;
    qreal mBottomMargin;
    qreal mInternalMargin;
    DuplexType mDuplexType;
    bool mDrawBorder;
    bool mReverseOrder;

    void init();
    void initFromCups();

};


#endif // PRINTER_H
