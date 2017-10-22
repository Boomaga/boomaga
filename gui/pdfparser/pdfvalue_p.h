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


#ifndef PDFVALUE_P_H
#define PDFVALUE_P_H

#include "pdfvalue.h"
#include <QSharedData>
#include <QIODevice>
#include <QDebug>
#include <QThreadStorage>

using namespace PDF;

//###############################################
// PDF Value
//###############################################
class PDF::ValueData: public QSharedData
{
public:
    ValueData(Value::Type type):
        mType(type),
        mNumberValue(0),
        mLinkObjNum(0),
        mLinkGenNum(0),
        mBoolValue(false),
        mValid(false)
    {
    }

    ValueData(const ValueData &other):
        QSharedData(other),
        mType        (other.mType),
        mArrayValues (other.mArrayValues),
        mDictValues  (other.mDictValues),
        mStringValue (other.mStringValue),
        mNumberValue (other.mNumberValue),
        mLinkObjNum  (other.mLinkObjNum),
        mLinkGenNum  (other.mLinkGenNum),
        mBoolValue   (other.mBoolValue),
        mValid       (other.mValid)
    {
    }

    ValueData &operator =(const ValueData &other)
    {
        mValid       = other.mValid;
        mType        = other.mType;
        mArrayValues = other.mArrayValues;
        mBoolValue   = other.mBoolValue;
        mDictValues  = other.mDictValues;
        mStringValue = other.mStringValue;
        mLinkObjNum  = other.mLinkObjNum;
        mLinkGenNum  = other.mLinkGenNum;
        mNumberValue = other.mNumberValue;

        return *this;
    }

    virtual ~ValueData()
    {
    }


    Value::Type mType;

    QVector<Value> mArrayValues;
    QMap<QString, Value> mDictValues;
    QByteArray mStringValue;
    double mNumberValue;
    quint32 mLinkObjNum;
    quint16 mLinkGenNum;
    bool mBoolValue;
    bool mValid;


    template <typename T>
    static T &emptyValue()
    {
        static QThreadStorage<T> stor;
        stor.localData().setValid(false);
        return stor.localData();
    }
};

#endif // PDFVALUE_P_H
