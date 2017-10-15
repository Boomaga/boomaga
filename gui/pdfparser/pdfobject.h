/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2017 Boomaga team https://github.com/Boomaga
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


#ifndef PDFOBJECT_H
#define PDFOBJECT_H

#include <QExplicitlySharedDataPointer>
#include <QVector>
#include "pdfvalue.h"

class QIODevice;

namespace PdfParser {

class ObjectData;

class Object
{
    friend class Reader;
    friend class Writer;
public:
    Object();
    Object(const Object &other);
    Object &operator =(const Object &other);

    virtual ~Object();

    quint32 objNum() const;
    void setObjNum(quint32 value);

    quint16 genNum() const;
    void setGenNum(quint16 value);

    Dict dictionary() const;

    Value value() const;
    void setValue(const Value &value);

    QByteArray stream() const;
    void setStream(const QByteArray &value);

private:
    QExplicitlySharedDataPointer<ObjectData> d;
};

} // namespace PdfParser

#endif // PDFOBJECT_H
