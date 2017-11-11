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


#ifndef PDFTYPES_H
#define PDFTYPES_H

#include <QtGlobal>
#include <QString>

namespace PDF {

class Error: public std::exception
{
public:
    Error(const QString &description):
        std::exception(),
        mDescription(description)
    {
    }

    const char* what() const noexcept override
    {
        return mDescription.toLocal8Bit().data();
    }

    //QString description() const { return mDescription; }

protected:
    QString mDescription;
};

class ReaderError: public Error
{
public:
    ReaderError(const QString &description, quint64 pos):
        Error(description),
        mPos(pos)
    {
    }

    const char* what() const noexcept override
    {
        return QString("Error on %1: %2")
                .arg(mPos)
                .arg(mDescription)
                .toLocal8Bit().data();
    }

private:
    quint64 mPos;
};

class ObjectError: public Error
{
public:
    ObjectError(const QString &description, uint objNum, uint genNum):
        Error(description),
        mObjNum(objNum),
        mGenNum(genNum)
    {
    }

    const char* what() const noexcept override
    {
        return QString("Error in object (%1 %2 obj): %3")
                .arg(mObjNum).arg(mGenNum)
                .arg(mDescription)
                .toLocal8Bit().data();
    }

private:
    uint mObjNum;
    uint mGenNum;
};

} // namespace PDF

#endif // PDFTYPES_H
