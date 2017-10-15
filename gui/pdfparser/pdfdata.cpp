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


#include "pdfdata.h"
#include <ctgmath>
#include <QDebug>

using namespace PdfParser;


/************************************************
 *
 ************************************************/
Data::Data(const char * const data, qint64 size):
    mData(data),
    mSize(size)
{

}

/************************************************
 *
 ************************************************/
bool Data::isDelim(qint64 pos) const
{
    return isspace(mData[pos]) ||
           strchr("()<>[]{}/%", mData[pos]);
}


/************************************************
 *
 ************************************************/
qint64 Data::indexOf(char c, qint64 from) const
{
    for (qint64 i = from; i<mSize; i++)
    {
        if (mData[i] == c)
            return i;
    }

    return -1;
}


/************************************************
 *
 ************************************************/
qint64 Data::indexOf(const char *str, qint64 from) const
{
    qint64 len = strlen(str);

    for (qint64 i = from; i<mSize-len; i++)
    {
        if (strncmp(mData + i, str, len) == 0)
            return i;
    }

    return -1;
}


/************************************************
 *
 ************************************************/
qint64 Data::indexOfBack(const char *str, qint64 from) const
{
    qint64 len = strlen(str);

    for (qint64 i = from - len + 1 ; i>0; i--)
    {
        if (strncmp(mData + i, str, len) == 0)
            return i;
    }

    return -1;
}


/************************************************
 *
 ************************************************/
qint64 Data::skipSpace(qint64 pos) const
{
    while (pos < mSize && isspace(mData[pos]))
        pos++;

    return pos;
}


/************************************************
 *
 ************************************************/
qint64 Data::skipCRLF(qint64 pos) const
{
    for (; pos >= 0; ++pos)
    {
        if (mData[pos] != '\n' && mData[pos] != '\r')
            return pos;
    }

    return 0;
}


/************************************************
 *
 ************************************************/
qint64 Data::skipDictBack(qint64 pos) const
{
    int level = 0;
    for (; pos>0; --pos)
    {
        if (mData[pos] == '>' && mData[pos-1] == '>')
        {
            ++level;
            --pos;
        }
        else if (mData[pos] == '<' && mData[pos-1] == '<')
        {
            --level;
            if (level == 0)
            {
                return pos-2;
            }
            --pos;
        }
    }

    return -1;
}


/************************************************
 *
 ************************************************/
quint32 Data::readUInt(qint64 *pos, bool *ok) const
{
    char *end;
    quint32 res = strtoul(mData + *pos, &end, 10);
    *ok = end != mData + *pos;
    *pos = end - mData;
    return res;
}


/************************************************
 *
 ************************************************/
double Data::readNum(qint64 *pos, bool *ok) const
{
    const char * str = mData + *pos;
    int sign = 1;
    if (str[0] == '-')
    {
        ++str;
        sign = -1;
    }

    char *end;
    double res = strtoll(str, &end, 10);
    if (end[0] == '.')
    {
        ++end;
        if (isdigit(end[0]))
        {
            str = end;
            long long fract = strtoll(str, &end, 10);
            if (str < end)
                res += fract / pow(10.0, end - str);
        }
    }
    *ok = end != mData + *pos;
    *pos = end - mData;
    return res * sign;
}


/************************************************
 *
 ************************************************/
void Data::dump(qint64 pos) const
{
    qWarning() << QString::fromLatin1(reinterpret_cast<const char*>(mData) + pos, qMin(20ll, mSize - pos));
}


/************************************************
 *
 ************************************************/
bool Data::compareStr(qint64 pos, const char *str) const
{
    int len = strlen(str);
    return (mSize - pos > len) && strncmp(mData + pos, str, len) == 0;
}


/************************************************
 *
 ************************************************/
bool Data::compareWord(qint64 pos, const char *str) const
{
    int len = strlen(str);
    return (mSize - pos > len + 1) &&
            strncmp(mData + pos, str, len) == 0 &&
            isDelim(pos + len);
}
