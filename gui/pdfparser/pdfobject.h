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

namespace PDF {

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

    const Dict &dict() const;
          Dict &dict();

    const Value &value() const;
    Value &value();
    void setValue(const Value &value);

    QByteArray stream() const;
    void setStream(const QByteArray &value);

    /// the Type entry identifies the type of object.
    QString type() const;

    /// In some cases, a Subtype entry is used to further
    /// identify a specialized subcategory of the general type.
    QString subType() const;

private:
    QExplicitlySharedDataPointer<ObjectData> d;
};

} // namespace PDF

QDebug operator<<(QDebug dbg, const PDF::Object &obj);

#endif // PDFOBJECT_H
