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


#include "sheet.h"


/************************************************

 ************************************************/
Sheet::Sheet(int count, int sheetNum):
    mSheetNum(sheetNum),
    mHints(0)
{
    mPages.resize(count);
    for (int i=0; i<count; ++i)
        mPages[i] = 0;
}


/************************************************

 ************************************************/
Sheet::~Sheet()
{
}


/************************************************

 ************************************************/
void Sheet::setPage(int index, ProjectPage *page)
{
    mPages[index] = page;
}


/************************************************

 ************************************************/
qint64 Sheet::genId()
{
    static qint64 id = 0;
    return ++id;
}


/************************************************

 ************************************************/
void Sheet::setHints(Sheet::Hints value)
{
    mHints = value;
}


/************************************************

 ************************************************/
void Sheet::setHint(Sheet::Hint hint, bool enable)
{
    if (enable)
        mHints = mHints | hint;
    else
        mHints = mHints & (~hint);
}

