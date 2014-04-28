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
#include "project.h"
#include "sheet.h"
#include "printer.h"

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

Layout::PagePosition Layout::calcPagePosition(const Sheet *sheet, int pageNumOnSheet, int pageCountHoriz, int pageCountVert, Qt::Orientation orientation) const
{
    Rotation rotatePage = project->layout()->rotate();

    PagePosition res;

    if (orientation == Qt::Horizontal)
    {
        if (isLandscape(rotatePage))
        {
            res.col = pageCountHoriz - 1 - pageNumOnSheet / pageCountVert;
            res.row = pageNumOnSheet % pageCountVert;
        }
        else
        {
            res.col = pageNumOnSheet % pageCountHoriz;
            res.row = pageNumOnSheet / pageCountHoriz;
        }

    }
    else
    {
        if (isLandscape(rotatePage))
        {
            res.col = pageCountHoriz - 1 - pageNumOnSheet % pageCountHoriz;
            res.row = pageNumOnSheet / pageCountHoriz;
        }
        else
        {
            res.col = pageNumOnSheet / pageCountVert;
            res.row = pageNumOnSheet % pageCountVert;
        }
    }

    return res;
}


/************************************************
  Landscape = true      Landscape = false
  +---------------+    +-----------------+
  | ............. |    | ....... ....... |
  | :           : |    | :     : :     : |
  | :     0     : |    | :  0  : :  1  : |
  | :           : |    | :     : :     : |
  | ............. |    | ....... ....... |
  | ............. |    | ....... ....... |
  | :           : |    | :     : :     : |
  | :     1     : |    | :  2  : :  3  : |
  | :           : |    | :     : :     : |
  | ............. |    | ....... ....... |
  +---------------+    +-----------------+
 ************************************************/
TransformSpec Layout::calcTransformSpec(const Sheet *sheet, int pageNumOnSheet,
                                        int pageCountHoriz, int pageCountVert,
                                        Qt::Orientation orientation) const
{
    Printer *printer = project->printer();
    const QRectF printerRect = printer->pageRect();
    const qreal margin = printer->internalMarhin();

    const qreal colWidth  = (printerRect.width()  - margin * (pageCountHoriz - 1)) / pageCountHoriz;
    const qreal rowHeight = (printerRect.height() - margin * (pageCountVert  - 1)) / pageCountVert;

    uint col, row;
    {
        PagePosition colRow = calcPagePosition(sheet, pageNumOnSheet, pageCountHoriz, pageCountVert, orientation);
        col = colRow.col;
        row = colRow.row;
    }

    // ................................
    QSizeF pdfPageSize;
    if (sheet && sheet->page(pageNumOnSheet))
        pdfPageSize = sheet->page(pageNumOnSheet)->rect().size();
    else
         pdfPageSize = printer->paperRect().size();

    Rotation rotatePage = project->layout()->rotate();

    if (isLandscape(rotatePage))
         pdfPageSize.transpose();
    // ................................

    qreal scale = qMin(colWidth  / pdfPageSize.width(),
                       rowHeight / pdfPageSize.height());

    QRectF placeRect;
    placeRect.setWidth( pdfPageSize.width() * scale);
    placeRect.setHeight(pdfPageSize.height() * scale);
    QPointF center;
    //            |  TopLeft page    |  margins       | full pages        | center of current page
    center.rx() = printerRect.left() + (margin * col) + (colWidth  * col) + (colWidth  * 0.5);
    center.ry() = printerRect.top()  + (margin * row) + (rowHeight * row) + (rowHeight * 0.5);
    placeRect.moveCenter(center);

    TransformSpec res;
    res.rect = placeRect;
    res.rotation = rotatePage;
    res.scale = scale;
    return res;
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

 ************************************************/
void LayoutNUp::fillSheets(QList<Sheet *> *sheets) const
{
    int pps = mPageCountVert * mPageCountHoriz;

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
        sheets->append(sheet);
    }
}


/************************************************
  Landscape = true      Landscape = false
  +---------------+    +-----------------+
  | ............. |    | ....... ....... |
  | :           : |    | :     : :     : |
  | :     0     : |    | :  0  : :  2  : |
  | :           : |    | :     : :     : |
  | ............. |    | ....... ....... |
  | ............. |    | ....... ....... |
  | :           : |    | :     : :     : |
  | :     1     : |    | :  1  : :  3  : |
  | :           : |    | :     : :     : |
  | ............. |    | ....... ....... |
  +---------------+    +-----------------+
 ************************************************/
TransformSpec LayoutNUp::transformSpec(const Sheet *sheet, int pageNumOnSheet) const
{
    return calcTransformSpec(sheet, pageNumOnSheet, mPageCountHoriz, mPageCountVert, mOrientation);
}


/************************************************

 ************************************************/
Rotation LayoutNUp::rotate() const
{
    if (mPageCountVert != mPageCountHoriz)
        return Rotate90;
    else
        return NoRotate;
}


/************************************************

 ************************************************/
LayoutBooklet::LayoutBooklet():
    Layout()
{
}


/************************************************

 ************************************************/
void LayoutBooklet::fillSheets(QList<Sheet *> *sheets) const
{
    fillSheetsForBook(0, project->pageCount(), sheets);
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
        //sheet->setHints(Sheet::HintDrawFold | Sheet::HintLandscapePreview);
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
        //sheet->setHints(Sheet::HintDrawFold | Sheet::HintLandscapePreview);
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
    fillPreviewSheetsForBook(0, project->pageCount(), sheets);
    if (sheets->count() > 1)
    {
        sheets->first()->setHint(Sheet::HintOnlyRight, true);
        sheets->last()->setHint(Sheet::HintOnlyLeft, true);
    }
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
        Sheet *sheet = new Sheet(2, sheets->count());
        sheet->setHints(Sheet::HintDrawFold);

        sheets->append(sheet);
        if (i>-1)
        {
            if (i<project->pageCount())
            {
                ProjectPage *page = project->page(i + bookStart);
                sheet->setPage(0, page);
            }
        }

        if (i+1<cnt)
        {
            if (i+1<project->pageCount())
            {
                ProjectPage *page = project->page(i + 1 + bookStart);
                sheet->setPage(1, page);
            }
        }
    }
}


/************************************************
    +---------------------------+
    |  .......................  |
    |  :                     :  |
    |  :                     :  |
    |  :                     :  |
    |  :       Page 0        :  |
    |  :                     :  |
    |  :                     :  |
    |  .......................  |
    |  :                     :  |
    |  :                     :  |
    |  :                     :  |
    |  :       Page 1        :  |
    |  :                     :  |
    |  :                     :  |
    |  .......................  |
    +---------------------------+
 ************************************************/
TransformSpec LayoutBooklet::transformSpec(const Sheet *sheet, int pageNumOnSheet) const
{
    return calcTransformSpec(sheet, pageNumOnSheet, 1, 2, Qt::Horizontal);
}
