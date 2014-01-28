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
TransformSpec Layout::calcTransformSpec(const Sheet *sheet, int pageNumOnSheet, int pageCountHoriz, int pageCountVert) const
{
    bool rotate = pageCountVert != pageCountHoriz;
    Printer *printer = project->printer();

    QRectF pageRect;
    if (sheet && sheet->page(pageNumOnSheet))
        pageRect = sheet->page(pageNumOnSheet)->rect();
    else
        pageRect = printer->paperRect();

    const qreal m = printer->internalMarhin();
    const QRectF printerRect = printer->pageRect();

    QRectF placeRect;
    placeRect.setWidth((printerRect.width()   - m * (pageCountHoriz - 1)) / pageCountHoriz);
    placeRect.setHeight((printerRect.height() - m * (pageCountVert  - 1)) / pageCountVert);

    qreal scale;
    TransformSpec::Rotation rotation;

    int col;
    int row;

    if (rotate)
    {
        col = pageCountHoriz - 1 - pageNumOnSheet / pageCountVert;
        row = pageNumOnSheet % pageCountVert;


        scale = qMin(
                placeRect.width()  / pageRect.height(),
                placeRect.height() / pageRect.width());

        placeRect.setHeight(pageRect.width() * scale);
        placeRect.setWidth(pageRect.height() * scale);
        rotation = TransformSpec::Rotate90;
    }
    else
    {
        col = pageNumOnSheet % pageCountHoriz;
        row = pageNumOnSheet / pageCountHoriz;


        scale = qMin(
                placeRect.width()  / pageRect.width(),
                placeRect.height() / pageRect.height());

        placeRect.setWidth(pageRect.width()   * scale);
        placeRect.setHeight(pageRect.height() * scale);
        rotation = TransformSpec::NoRotate;
    }

    QRectF r = printerRect.adjusted(-m * 0.5 , -m * 0.5, m * 0.5, m * 0.5);
    qreal x = r.left() + (r.width()  / (pageCountHoriz * 2.0 ) * (col * 2.0 + 1));
    qreal y = r.top()  + (r.height() / (pageCountVert  * 2.0 ) * (row * 2.0 + 1));

    placeRect.moveCenter(QPointF(x, y));

    TransformSpec res;
    res.rect = placeRect;
    res.rotation = rotation;
    res.scale = scale;
    return res;
}


/************************************************

 ************************************************/
LayoutNUp::LayoutNUp(int pageCountVert, int pageCountHoriz):
    Layout(),
    mPageCountVert(pageCountVert),
    mPageCountHoriz(pageCountHoriz)
{
    mRotate = mPageCountVert != mPageCountHoriz;
}


/************************************************

 ************************************************/
QString LayoutNUp::id() const
{
    return QString("%1up").arg(mPageCountVert * mPageCountHoriz);
}


/************************************************

 ************************************************/
void LayoutNUp::fillSheets(QList<Sheet *> *sheets) const
{
    Sheet *sheet;

    int pps = mPageCountVert * mPageCountHoriz;

    int i=0;
    while (i < project->pageCount())
    {
        sheet = new Sheet(pps, sheets->count());
        sheet->setHint(Sheet::HintLandscapePreview, mRotate);

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
    return calcTransformSpec(sheet, pageNumOnSheet, mPageCountHoriz, mPageCountVert);
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
    int n;
    Sheet *sheet;

    for (int i = 0; i < cnt / 2; i+=2)
    {
        // Sheet 0 **************************
        sheet = new Sheet(2, sheets->count());
        sheet->setHints(Sheet::HintDrawFold | Sheet::HintLandscapePreview);
        sheets->append(sheet);

        n = (cnt - 1) - i;
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
        sheet->setHints(Sheet::HintDrawFold | Sheet::HintLandscapePreview);
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

    Sheet *sheet;
    for (int i = -1; i < cnt; i+=2)
    {
        sheet = new Sheet(2, sheets->count());
        sheet->setHints(Sheet::HintDrawFold | Sheet::HintLandscapePreview);

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
    return calcTransformSpec(sheet, pageNumOnSheet, 1, 2);
}
