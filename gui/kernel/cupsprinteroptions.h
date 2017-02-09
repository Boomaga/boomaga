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


#ifndef CUPSPRINTEROPTIONS_H
#define CUPSPRINTEROPTIONS_H

#include <QString>
#include <QSizeF>


class CupsPrinterOptions
{
public:
    CupsPrinterOptions(const QString &printerName);

    QString printerName() const {return  mPrinterName; }
    QString deviceURI() const {return  mDeviceURI; }

    QString grayScaleOption() const { return mGrayScaleOption; }
    QString colorOption() const { return mColorOption; }
    bool duplex() const { return mDuplex; }


    QSizeF paperSize() const { return mPaperSize; }

    qreal leftMargin()   const { return mLeftMargin; }
    qreal rightMargin()  const { return mRightMargin; }
    qreal topMargin()    const { return mTopMargin; }
    qreal bottomMargin() const { return mBottomMargin; }

private:
    QString mPrinterName;
    QString mDeviceURI;

    QString mColorOption;
    QString mGrayScaleOption;
    bool mDuplex;
    QSizeF mPaperSize;
    qreal mLeftMargin;
    qreal mRightMargin;
    qreal mTopMargin;
    qreal mBottomMargin;
};

#endif // CUPSPRINTEROPTIONS_H
