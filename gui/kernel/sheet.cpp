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
#include "layout.h"

#include <QDebug>


/************************************************

 ************************************************/
Sheet::Sheet(int count, int sheetNum):
    mSheetNum(sheetNum),
    mHints(0),
    mRotation(project->rotation())
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
void Sheet::setPage(int index, Page *page)
{
    mPages[index] = page;
}


/************************************************

 ************************************************/
int Sheet::indexOfPage(const Page *page, int from) const
{
    return mPages.indexOf(const_cast<Page*>(page), from);
}


/************************************************
 *
 * ***********************************************/
void Sheet::setRotation(Rotation rotation)
{
    mRotation = rotation;
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


/************************************************

 ************************************************/
int SheetList::indexOfPage(const Page *page, int from) const
{
    for (int i=from; i<count(); ++i)
    {
        const Sheet *sheet = at(i);
        for (int p=0; p<sheet->count(); ++p)
        {
            if (sheet->page(p) == page)
                return i;
        }
    }

    return -1;
}

/************************************************
 *
 ************************************************/
int  SheetList::indexOfPage(int pageNum, int from) const
{
    return indexOfPage(project->page(pageNum), from);
}


/************************************************
 *
 ************************************************/
QDebug operator<<(QDebug dbg, const Sheet &sheet)
{
    dbg.space() << "{ Sheet: " << sheet.sheetNum() << "\n"
                << "  pages: " << sheet.count() << "\n";
    for (int i=0; i<sheet.count(); ++i)
    {
        const Page *page = sheet.page(i);
        if (page)
        {
            dbg.space() << "   * " << i << "---------\n"
                        << "       pageNum:"        << page->pageNum() << "\n"
                        << "       blankPage:"      << page->isBlankPage() << "\n"
                        << "       visible:"        << page->visible() << "\n"
                        << "       startBooklet:"   << page->isStartSubBooklet() << "\n"
                        << "\n";
        }
        else
        {
            dbg.space() << "   * " << i << "---------\n"
                        << "       NULL\n"
                        << "\n";
        }
    }
    dbg.space() << " }";
    return dbg;
}
