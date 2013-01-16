/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2013 Alexander Sokoloff
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


#include "pssheet.h"
#include "psproject.h"


/************************************************

 ************************************************/
PsSheetPageSpec::PsSheetPageSpec()
{
    mRect = QRect(0, 0, 0, 0);
    mScale = 1;
    mRotate = NoRotate;
}


/************************************************

 ************************************************/
PsSheetPageSpec::PsSheetPageSpec(const QRectF &rect, double scale, Rotation rotete)
{
    mRect = rect;
    mScale = scale;
    mRotate = rotete;
}


/************************************************

 ************************************************/
PsSheet::PsSheet(PsProject *project, int count) :
    mProject(project),
    mSheetNum(0),
    mHints(0)
{
    mPages.resize(count);
    mSpecs.resize(count);
    for (int i=0; i<count; ++i)
        mPages[i] = 0;
}


/************************************************

 ************************************************/
void PsSheet::setPage(int index, PsProjectPage *page)
{
    mPages[index] = page;
}


/************************************************

 ************************************************/
void PsSheet::setPageSpec(int index, PsSheetPageSpec spec)
{
    mSpecs[index] = spec;
}


/************************************************

 ************************************************/
void PsSheet::setHints(Hints value)
{
    mHints = value;
}


/************************************************

 ************************************************/
void PsSheet::setHint(PsSheet::Hint hint, bool enable)
{
    if (enable)
        mHints = mHints | hint;
    else
        mHints = mHints & (~hint);
}


/************************************************

 ************************************************/
//void PsSheet::write(QTextStream *out) const
//{
//    QList<const PsSheet*> sheets;
//    sheets << this;
//    mProject->writeDocument(sheets, out);
//}


/************************************************

 ************************************************/
PsSheet::~PsSheet()
{
}


/************************************************

 ************************************************/
QDebug operator<<(QDebug dbg, const PsSheet &sheet)
{
    QString str;
    for (int p=0; p < sheet.count(); ++p)
    {
        const PsProjectPage *page = sheet.page(p);
        if (page)
            str += QString("[%1] ").arg(page->pageNum(), 3);
        else
            str += "[   ] ";
    }
    dbg << str;
    return dbg.space();
}


/************************************************

 ************************************************/
QDebug operator<<(QDebug dbg, const PsSheet *sheet)
{
    return operator<<(dbg, *sheet);
}


/************************************************

 ************************************************/
QDebug operator<<(QDebug dbg, const QList<PsSheet *> &sheets)
{
    for (int i=0; i<sheets.count(); ++i)
    {
        dbg << i << sheets.at(i) << endl;
    }
    return dbg.space();
}


/************************************************

 ************************************************/
QDebug operator<<(QDebug dbg, const QList<PsSheet *> *sheets)
{
    return operator<<(dbg, *sheets);
}


/************************************************

 ************************************************/
QDebug operator<<(QDebug dbg, const PsSheetPageSpec &spec)
{
    QString str;
    dbg << "Rect:  " << spec.rect() << endl;
    dbg << "Scale: " << spec.scale() << endl;
    dbg << "Rotate:" << (int)spec.rotate() << endl;
    return dbg.space();
}


/************************************************

 ************************************************/
QDebug operator<<(QDebug dbg, const PsSheetPageSpec *spec)
{
    return operator<<(dbg, *spec);
}
