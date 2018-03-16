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


#include "layout.h"
#include <QString>
#include <QList>
#include "project.h"
#include "sheet.h"
#include "printer.h"
#include "settings.h"

#include "math.h"

#include <QDebug>
/************************************************

 ************************************************/
Layout::Layout()
{
}


/************************************************

 ************************************************/
Layout::~Layout()
{
}


/************************************************

 ************************************************/
void Layout::fillPreviewSheets(QList<Sheet *> *sheets) const
{
    fillSheets(sheets);
}



/************************************************

 ************************************************/
LayoutNUp::LayoutNUp(int pageCountVert, int pageCountHoriz, Qt::Orientation orientation):
    Layout(),
    mPageCountVert(pageCountVert),
    mPageCountHoriz(pageCountHoriz),
    mOrientation(orientation)
{
}


/************************************************

 ************************************************/
QString LayoutNUp::id() const
{
    return QString("%1up%2")
            .arg(mPageCountVert * mPageCountHoriz)
            .arg((mOrientation == Qt::Horizontal ? "" : "V"));
}


/************************************************
 *
 * ***********************************************/
int LayoutNUp::calcSheetCount() const
{
    int pps = mPageCountVert * mPageCountHoriz;

    return ceil(project->pageCount() * 1.0 / pps);
}


/************************************************
 *
 * ***********************************************/
void LayoutNUp::fillSheets(QList<Sheet *> *sheets) const
{
    doFillSheets(sheets, false);
}


/************************************************
 *
 * ***********************************************/
void LayoutNUp::fillPreviewSheets(QList<Sheet *> *sheets) const
{
    doFillSheets(sheets, true);
}


/************************************************

 ************************************************/
void LayoutNUp::doFillSheets(QList<Sheet *> *sheets, bool) const
{
    int pps = mPageCountVert * mPageCountHoriz;
    bool rightToLeft = settings->value(Settings::RightToLeft).toBool();

    int i=0;
    while (i < project->pageCount())
    {
        Sheet *sheet = new Sheet(pps, sheets->count());

        for (int j=0; j<pps; ++j)
        {
            if (i<project->pageCount())
            {
                ProjectPage *page = project->page(i);
                sheet->setPage(j, page);
            }
            ++i;
        }

        sheet->setRotation(project->rotation());
        sheets->append(sheet);
    }
}


/************************************************
 * In this place sheet is always has portrait orientation.
 * It will be rotated later based on sheet.rotation() value.
 ************************************************/
TransformSpec LayoutNUp::transformSpec(const Sheet *sheet, int pageNumOnSheet, Rotation sheetRotation) const
{
    Printer *printer = project->printer();
    const QRectF printerRect = printer->pageRect();
    const qreal margin = printer->internalMarhin();

    const qreal colWidth  = (printerRect.width()  - margin * (mPageCountHoriz - 1)) / mPageCountHoriz;
    const qreal rowHeight = (printerRect.height() - margin * (mPageCountVert  - 1)) / mPageCountVert;

    uint col, row;
    {
        Direction direction = settings->value(Settings::RightToLeft).toBool() ? RightToLeft : LeftToRight;
        PagePosition colRow = calcPagePosition(pageNumOnSheet, sheetRotation, direction);
        col = colRow.col;
        row = colRow.row;
    }

    TransformSpec spec;
    QSizeF pageSize;
    {
        const ProjectPage *page = sheet->page(pageNumOnSheet);
        spec.rotation = calcPageRotation(page, sheetRotation);
        if (page)
        {
            pageSize = page->rect().size();
            spec.rotation += page->manualRotation();
        }
        else
        {
            pageSize = printer->paperRect().size();
        }
    }

    //spec.rotation += pageManualRotation;

    if (isLandscape(spec.rotation))
         pageSize.transpose();

    spec.scale = qMin(colWidth  / pageSize.width(),
                      rowHeight / pageSize.height());

    QRectF placeRect;
    placeRect.setWidth( pageSize.width()  * spec.scale);
    placeRect.setHeight(pageSize.height() * spec.scale);
    QPointF center;
    //            |  TopLeft page    |  margins       | full pages        | center of current page
    center.rx() = printerRect.left() + (margin * col) + (colWidth  * col) + (colWidth  * 0.5);
    center.ry() = printerRect.top()  + (margin * row) + (rowHeight * row) + (rowHeight * 0.5);
    placeRect.moveCenter(center);

    spec.rect = placeRect;

    return spec;
 }


/************************************************

 ************************************************/
Rotation LayoutNUp::rotation() const
{
    if (mPageCountVert != mPageCountHoriz)
        return Rotate90;
    else
        return NoRotate;
}


/************************************************
 *
 ************************************************/
FlipType LayoutNUp::flipType(FlipType printerFlipType) const
{
    return printerFlipType;
}


/************************************************

 ************************************************/
void LayoutNUp::updatePages(QList<ProjectPage *> pages) const
{
    foreach(ProjectPage *page, pages)
        page->setAutoStartSubBooklet(false);

    pages.first()->setAutoStartSubBooklet(false);
}


/************************************************

 ************************************************/
int LayoutNUp::previewPageNum(int sheetNum) const
{
    return sheetNum * pagePerSheet();
}


/************************************************

 ************************************************/
Rotation LayoutNUp::calcPageRotation(const ProjectPage *page, Rotation sheetRotation) const
{
    if (!page)
        return this->rotation();

    Rotation res;
    bool isSheetLandscape = isLandscape(this->rotation())  ^ isLandscape(sheetRotation);

    bool isPageLandscape = isLandscape(page->pdfRotation()) ^ isLandscape(page->rect());
        res = page->pdfRotation();

    if (isPageLandscape != isSheetLandscape)
        res -= 90;

    return res - sheetRotation;
}



/************************************************
 * In this place sheet is always has portrait orientation.
 * It will be rotated later based on sheet.rotation() value
 *
 *
 *  h - mPageCountHoriz
 *  v - mPageCountVert
 *
 *  Left-To-Right direction _________________________
 *  +------+ Rotate: 0           +------+ Rotate: 0
 *  | 0  1 | Horiz               | 0  4 | Vert
 *  | 2  3 |                     | 1  5 |
 *  | 4  5 | r = i / h           | 2  6 | r = i % v
 *  | 6  7 | c = i % h           | 3  7 | c = i / v
 *  +------+                     +------+
 *
 *  +------+ Rotate: 90          +------+ Rotate: 90
 *  | 3  7 | Horiz               | 6  7 | Vert
 *  | 2  6 |                     | 4  5 |
 *  | 1  5 | r = (v-1)-(i%v)     | 2  3 | r = (v-1)-(i/h)
 *  | 0  4 | c = i / v           | 0  1 | c = i % h
 *  +------+                     +------+
 *
 *  +------+ Rotate: 180         +------+ Rotate: 180
 *  | 7  6 | Horiz               | 7  3 | Vert
 *  | 5  4 |                     | 6  2 |
 *  | 3  2 | r = (v-1)-(i/h)     | 5  1 | r = (v-1)-(i%v)
 *  | 1  0 | c = (h-1)-(i%h)     | 4  0 | c = (h-1)-(i/v)
 *  +------+                     +------+
 *
 *  +------+ Rotate: 270         +------+ Rotate: 270
 *  | 4  0 | Horiz               | 1  0 | Vert
 *  | 5  1 |                     | 3  2 |
 *  | 6  2 | r = i % v           | 5  4 | r = i / h
 *  | 7  3 | c = (h-1)-(i/v)     | 7  6 | c = (h-1)-(i%v)
 *  +------+                     +------+
 *
 *
 *  Right-To-Right direction _________________________
 *  +------+ Rotate: 0           +------+ Rotate: 0
 *  | 1  0 | Horiz               | 4  0 | Vert
 *  | 3  2 |                     | 5  1 |
 *  | 5  4 | r = i / h           | 6  2 | r = i % v
 *  | 7  6 | c = (h-1)-(i%v)     | 7  3 | c = (h-1)-(i/v)
 *  +------+                     +------+
 *
 *  +------+ Rotate: 90          +------+ Rotate: 90
 *  | 0  4 | Horiz               | 0  1 | Vert
 *  | 1  5 |                     | 2  3 |
 *  | 2  6 | r = i % v           | 4  5 | r = i / h
 *  | 3  7 | c = i / v           | 6  7 | c = i % h
 *  +------+                     +------+
 *
 *  +------+ Rotate: 180         +------+ Rotate: 180
 *  | 6  7 | Horiz               | 3  7 | Vert
 *  | 4  5 |                     | 2  6 |
 *  | 2  3 | r = (v-1)-(i/h)     | 1  5 | r = (v-1)-(i%v)
 *  | 0  1 | c = i % h           | 0  4 | c = i / v
 *  +------+                     +------+
 *
 *  +------+ Rotate: 270         +------+ Rotate: 270
 *  | 7  3 | Horiz               | 7  6 | Vert
 *  | 6  2 |                     | 5  4 |
 *  | 5  1 | r = (v-1)-(i%v)     | 3  2 | r = (v-1)-(i/h)
 *  | 4  0 | c = (h-1)-(i/v)     | 1  0 | c = (h-1)-(i%v)
 *  +------+                     +------+
 *
 ************************************************/
Layout::PagePosition LayoutNUp::calcPagePosition(int pageNumOnSheet, Rotation sheetRotation, Direction direction) const
{
    if (direction == LeftToRight)
    {
        // Left-To-Right direction ..................................
        PagePosition res;

        if (isLandscape(sheetRotation))
        {
            switch (mOrientation)
            {
            case Qt::Horizontal:
                res.row = (mPageCountVert - 1) - pageNumOnSheet % mPageCountVert;
                res.col = pageNumOnSheet / mPageCountVert;
                break;

            case Qt::Vertical:
                res.row = (mPageCountVert - 1) - pageNumOnSheet / mPageCountHoriz;
                res.col = pageNumOnSheet % mPageCountHoriz;
                break;
            }
        }
        else
        {
            switch (mOrientation)
            {
            case Qt::Horizontal:
                res.row = pageNumOnSheet / mPageCountHoriz;
                res.col = pageNumOnSheet % mPageCountHoriz;
                break;

            case Qt::Vertical:
                res.row = pageNumOnSheet % mPageCountVert;
                res.col = pageNumOnSheet / mPageCountVert;
                break;
            }
        }

        return res;
    }
    else
    {
        // Right-To-Left direction ..................................
        PagePosition res;

        if (isLandscape(sheetRotation))
        {
            switch (mOrientation)
            {
            case Qt::Horizontal:
                res.row = pageNumOnSheet % mPageCountVert;
                res.col = pageNumOnSheet / mPageCountVert;
                break;

            case Qt::Vertical:
                res.row = pageNumOnSheet / mPageCountHoriz;
                res.col = pageNumOnSheet % mPageCountHoriz;
                break;
            }
        }
        else
        {
            switch (mOrientation)
            {
            case Qt::Horizontal:
                res.row = pageNumOnSheet / mPageCountHoriz;
                res.col = (mPageCountHoriz - 1) - pageNumOnSheet % mPageCountHoriz;
                break;

            case Qt::Vertical:
                res.row = pageNumOnSheet % mPageCountVert;
                res.col = (mPageCountHoriz - 1) - pageNumOnSheet / mPageCountVert;
                break;
            }
        }

        return res;
    }
}


/************************************************

 ************************************************/
LayoutBooklet::LayoutBooklet():
    LayoutNUp(2, 1)
{
}


/************************************************
 *
 * ***********************************************/
int LayoutBooklet::calcSheetCount() const
{
    int result = 0;
    QList<BookletInfo> booklets = split(project->pages());

    foreach (BookletInfo booklet, booklets)
    {
        result += ceil((booklet.end - booklet.start + 1) / 4.0 ) * 2;
    }
    return result;
}


/************************************************

 ************************************************/
void LayoutBooklet::fillSheets(QList<Sheet *> *sheets) const
{
    QList<BookletInfo> booklets = split(project->pages());
    foreach (BookletInfo booklet, booklets)
    {
        fillSheetsForBook(booklet.start,
                          booklet.end - booklet.start + 1,
                          sheets);
    }
}


/************************************************

  +-----------+  +-----------+
  |     :     |  |     :     |
  |  N  :  0  |  |  1  : N-1 |
  |     :     |  |     :     |
  +-----------+  +-----------+
     sheet 0        sheet 1

  +-----------+  +-----------+
  |     :     |  |     :     |
  | N-3 :  3  |  |  4  : N-4 |
  |     :     |  |     :     |
  +-----------+  +-----------+
     sheet 2        sheet 3
               ...
 ************************************************/
void LayoutBooklet::fillSheetsForBook(int bookStart, int bookLength, QList<Sheet *> *sheets) const
{
    int cnt = ceil(bookLength / 4.0 ) * 4;

    for (int i = 0; i < cnt / 2; i+=2)
    {
        // Sheet 0 **************************
        Sheet *sheet = new Sheet(2, sheets->count());
        sheet->setHints(Sheet::HintDrawFold);
        sheets->append(sheet);

        int n = (cnt - 1) - i;
        if (n < bookLength)
        {
            ProjectPage *page = project->page(n + bookStart);
            sheet->setPage(0, page);
        }


        n = i;
        if (n < bookLength)
        {
            ProjectPage *page = project->page(n + bookStart);
            sheet->setPage(1, page);
        }


        // Sheet 1 **************************
        sheet = new Sheet(2, sheets->count());
        sheet->setHints(Sheet::HintDrawFold);
        sheet->setRotation(project->rotation());
        sheets->append(sheet);

        n = i + 1;
        if (n < bookLength)
        {
            ProjectPage *page = project->page(n + bookStart);
            sheet->setPage(0, page);
        }

        n = (cnt - 1) - (i + 1);
        if (n < bookLength)
        {
            ProjectPage *page = project->page(n + bookStart);
            sheet->setPage(1, page);
        }
    }
}


/************************************************

 ************************************************/
void LayoutBooklet::fillPreviewSheets(QList<Sheet *> *sheets) const
{
    QList<BookletInfo> booklets = split(project->pages());
    foreach (BookletInfo booklet, booklets)
    {
        fillPreviewSheetsForBook(booklet.start,
                                 booklet.end - booklet.start + 1,
                                 sheets);
    }

    if (sheets->count() > 1)
    {
        sheets->first()->setHint(Sheet::HintOnlyRight, true);
        sheets->first()->setHint(Sheet::HintDrawFold, false);
        sheets->last()->setHint(Sheet::HintOnlyLeft, true);
        sheets->last()->setHint(Sheet::HintDrawFold, false);
    }
}


/************************************************

 ************************************************/
void LayoutBooklet::updatePages(QList<ProjectPage *> pages) const
{
    if (!settings->value(Settings::SubBookletsEnabled).toBool())
    {
        foreach (ProjectPage *page, pages)
            page->setAutoStartSubBooklet(false);

    }
    else
    {

        int pagePerBook = settings->value(Settings::SubBookletSize).toInt() * 4;

        int n = pagePerBook;
        foreach (ProjectPage *page, pages)
        {
            if (page->isManualStartSubBooklet())
                n = pagePerBook;

            page->setAutoStartSubBooklet(!(n % pagePerBook));
            n--;
        }
    }

    pages.first()->setAutoStartSubBooklet(true);
}


/************************************************
 *
 ************************************************/
FlipType LayoutBooklet::flipType(FlipType) const
{
    return FlipType::ShortEdge;
}


/************************************************
  : - - +-----+  +-----------+
  :     |     |  |     :     |
  : -1  |  0  |  |  1  :  2  |
  :     |     |  |     :     |
  : - - +-----+  +-----------+
     sheet 0        sheet 1

  +-----------+  +-----+ - - :
  |     :     |  |     |     :
  |  3  :  4  |  |  5  |     :
  |     :     |  |     |     :
  +-----------+  +-----+ - - :
     sheet 2        sheet 3

 ************************************************/
void LayoutBooklet::fillPreviewSheetsForBook(int bookStart, int bookLength, QList<Sheet *> *sheets) const
{
    int cnt = ceil(bookLength / 4.0 ) * 4;

    for (int i = -1; i < cnt; i+=2)
    {
        Sheet *sheet;
        if (i<0 && !sheets->isEmpty())
        {
            sheet = sheets->last();
            sheet->setHints(Sheet::HintDrawFold | Sheet::HintSubBooklet);
        }
        else
        {
            sheet = new Sheet(2, sheets->count());
            sheet->setHints(Sheet::HintDrawFold);
            *sheets << sheet;
        }

        if (i>-1)
        {
            if (i < bookLength)
            {
                ProjectPage *page = project->page(i + bookStart);
                sheet->setPage(0, page);
            }
        }

        if (i+1<cnt)
        {
            if (i +1 < bookLength)
            {
                ProjectPage *page = project->page(i + 1 + bookStart);
                sheet->setPage(1, page);
            }
        }
    }
}


/************************************************
 *
 ************************************************/
QList<LayoutBooklet::BookletInfo> LayoutBooklet::split(const QList<ProjectPage *> &pages) const
{
    QList<BookletInfo> res;
    int end = pages.count()-1;
    for (int i=pages.count()-1; i>=0; i--)
    {
        const ProjectPage *page = pages.at(i);
        if (page->isStartSubBooklet())
        {
            BookletInfo booklet;
            booklet.start = i;
            booklet.end = end;
            res.prepend(booklet);
            end = i-1;
        }
    }
    return res;
}


/************************************************
 *
 ************************************************/
int LayoutBooklet::previewPageNum(int sheetNum) const
{
    return sheetNum * 2 -1;
}
