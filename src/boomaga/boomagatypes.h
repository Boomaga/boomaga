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

#ifndef BOOMAGATYPES_H
#define BOOMAGATYPES_H
#include <QMetaType>
#include <QDataStream>
#include <QVariant>
#include <QRectF>
#include <QApplication>

enum DuplexType
{
    DuplexAuto          = 1,
    DuplexManual        = 2,
    DuplexManualReverse = 3
};

Q_DECLARE_METATYPE(DuplexType)

QString duplexTypeToStr(DuplexType value);
DuplexType strToDuplexType(const QString &str);


enum class FlipType
{
    ShortEdge,
    LongEdge
};

Q_DECLARE_METATYPE(FlipType)

QString flipTypeToStr(FlipType value);
FlipType strToFlipType(const QString &str);

enum ColorMode
{
    ColorModeAuto       = 0,
    ColorModeColor      = 1,
    ColorModeGrayscale  = 2
};

Q_DECLARE_METATYPE(ColorMode)

QString colorModeToStr(ColorMode value);
ColorMode strToColorMode(const QString &str);


enum Rotation
{
    NoRotate  = 0,
    Rotate90  = 90,
    Rotate180 = 180,
    Rotate270 = 270
};

enum Direction
{
    LeftToRight = 0,
    RightToLeft = 1
};

inline bool isLandscape(Rotation rotation)  { return (int)rotation % 180; }
inline bool isLandscape(const QSizeF &size) { return size.width() > size.height(); }
inline bool isLandscape(const QSize &size)  { return size.width() > size.height(); }
inline bool isLandscape(const QRectF &rect) { return isLandscape(rect.size()); }
inline bool isLandscape(const QRect &rect)  { return isLandscape(rect.size()); }

inline bool isPortrate(Rotation rotation)  { return ! isLandscape(rotation); }
inline bool isPortrate(const QSizeF &size) { return ! isLandscape(size); }
inline bool isPortrate(const QSize &size)  { return ! isLandscape(size); }
inline bool isPortrate(const QRectF &rect) { return ! isLandscape(rect); }
inline bool isPortrate(const QRect &rect)  { return ! isLandscape(rect); }


inline Rotation intToRotation(int r) { return (Rotation)((360 + (r % 360)) % 360); }

inline Rotation operator+(Rotation r1, int r2)      { return intToRotation((int)r1 + (int)r2); }
inline Rotation operator+(Rotation r1, Rotation r2) { return intToRotation((int)r1 + (int)r2); }
inline Rotation operator-(Rotation r1, int r2)      { return intToRotation((int)r1 - (int)r2); }
inline Rotation operator-(Rotation r1, Rotation r2) { return intToRotation((int)r1 - (int)r2); }

inline Rotation &operator+=(Rotation &r1, int r2)      { r1 = intToRotation((int)r1 + (int)r2); return r1; }
inline Rotation &operator+=(Rotation &r1, Rotation r2) { r1 = intToRotation((int)r1 + (int)r2); return r1; }

inline Rotation &operator-=(Rotation &r1, int r2)      { r1 = intToRotation((int)r1 - (int)r2); return r1; }
inline Rotation &operator-=(Rotation &r1, Rotation r2) { r1 = intToRotation((int)r1 - (int)r2); return r1; }

enum Unit {
    UnitMillimeter = 0,
    UnitPoint      = 1
    //UnitInch       = 2
};


//enum PaperSize { A4, B5, Letter, Legal, Executive,
//                 A0, A1, A2, A3, A5, A6, A7, A8, A9, B0, B1,
//                 B10, B2, B3, B4, B6, B7, B8, B9, C5E, Comm10E,
//                 DLE, Folio, Ledger, Tabloid, Custom, NPageSize = Custom, NPaperSize = Custom };

template<class T>
T* findExistingForm()
{
    foreach(QWidget *widget, qApp->topLevelWidgets())
    {
        T* res = qobject_cast<T*>(widget);
        if (res)
            return res;
    }
    return 0;
}

QString safeFileName(const QString &str);

QString expandHomeDir(const QString &fileName);
QString shrinkHomeDir(const QString &fileName);

#endif // BOOMAGATYPES_H
