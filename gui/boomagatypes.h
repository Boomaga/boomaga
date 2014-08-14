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

enum DuplexType
{
    DuplexAuto          = 1,
    DuplexManual        = 2,
    DuplexManualReverse = 3
};

Q_DECLARE_METATYPE(DuplexType)

QString duplexTypetoStr(DuplexType value);
DuplexType strToDuplexType(const QString &str);


enum Rotation
{
    NoRotate  = 0,
    Rotate90  = 90,
    Rotate180 = 180,
    Rotate270 = 270
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

#endif // BOOMAGATYPES_H
