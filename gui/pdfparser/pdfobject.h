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
    Object(ObjNum objNum = 0, GenNum genNum = 0, const Value &value = Dict());
    Object(const Object &other);
    Object &operator =(const Object &other);

    virtual ~Object();

    PDF::ObjNum objNum() const { return mObjNum; }
    void setObjNum(PDF::ObjNum value);

    PDF::GenNum genNum() const { return mGenNum; }
    void setGenNum(PDF::GenNum value);

    const Dict &dict() const { return mValue.asDict(); }
          Dict &dict()       { return mValue.asDict(); }

    const Value &value() const { return mValue; }
    Value &value()             { return mValue; }
    void setValue(const Value &value);

    QByteArray stream() const { return mStream; }
    void setStream(const QByteArray &value);

    QByteArray decodedStream() const;

    /// the Type entry identifies the type of object.
    QString type() const;

    /// In some cases, a Subtype entry is used to further
    /// identify a specialized subcategory of the general type.
    QString subType() const;

    bool isValid() const { return !mValue.isUndefined(); }

    quint64 pos() const { return mPos; }
    quint64 len() const { return mLen; }

private:
    QByteArray streamFlateDecode(const QByteArray &source) const;

    PDF::ObjNum mObjNum;
    PDF::GenNum mGenNum;
    Value mValue;
    QByteArray mStream;
    quint64 mPos;
    quint64 mLen;
};

} // namespace PDF

QDebug operator<<(QDebug dbg, const PDF::Object &obj);

#endif // PDFOBJECT_H
