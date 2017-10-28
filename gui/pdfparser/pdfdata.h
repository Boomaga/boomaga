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


#ifndef PDFDATA_H
#define PDFDATA_H

#include <QtGlobal>
namespace PDF {


class Data
{
public:
    Data(const char * const data, qint64 size);

    bool isDelim(qint64 pos) const;

    qint64 indexOf(char c, qint64 from = 0) const;
    qint64 indexOf(const char *str, qint64 from = 0) const;
    qint64 indexOfBack(const char *str, qint64 from) const;

    qint64 skipSpace(qint64 pos) const;
    qint64 skipCRLF(qint64 pos) const;
    qint64 skipDictBack(qint64 pos) const;

    quint32 readUInt(qint64 *pos, bool *ok) const;

    double readNum(qint64 *pos, bool *ok) const;

    const char * data() const { return mData; }
    qint64 size() const { return mSize; }

    void dump(qint64 pos) const;

    char at(qint64 i) const { return mData[i]; }
    char operator[](qint64 i) const { return mData[i]; }

    bool compareStr(qint64 pos, const char *str) const;
    bool compareWord(qint64 pos, const char *str) const;

private:
    const char * const mData;
    const qint64 mSize;
};

} // namespace PDF
#endif // PDFDATA_H
