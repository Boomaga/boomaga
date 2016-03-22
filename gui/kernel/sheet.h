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


#ifndef SHEET_H
#define SHEET_H

#include <QtGlobal>
#include <QVector>
#include <QRectF>
#include <QDebug>
#include "boomagatypes.h"

class Page;


class Sheet
{
public:
    enum Hint{
        HintOnlyLeft    = 1,
        HintOnlyRight   = 2,
        HintDrawFold    = 4,
        HintSubBooklet  = 8
    };

    Q_DECLARE_FLAGS(Hints, Hint)

    Hints hints() const { return mHints; }
    void setHints(Hints value);
    void setHint(Hint hint, bool enable);

    explicit Sheet(int count, int sheetNum);
    virtual ~Sheet();

    Page *page(int index) { return mPages[index]; }
    const Page *page(int index) const { return mPages.at(index); }
    void setPage(int index, Page *page);

    int count() const { return mPages.count(); }
    int indexOfPage(const Page *page, int from = 0) const;

    Rotation rotation() const { return mRotation; }
    void setRotation(Rotation rotation);

    int sheetNum() const { return mSheetNum; }
private:
    QVector<Page*> mPages;
    int mSheetNum;
    Hints mHints;
    Rotation mRotation;
};

class SheetList: public QList<Sheet*>
{
public:
    int indexOfPage(const Page *page, int from = 0) const;
    int indexOfPage(int pageNum, int from = 0) const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Sheet::Hints)

QDebug operator<<(QDebug dbg, const Sheet &sheet);
#endif // SHEET_H
