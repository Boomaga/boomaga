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


#include "psengine.h"
#include "pssheet.h"
#include "printer.h"


#include <math.h>
#include <QDebug>
#include <QRect>


/************************************************

 ************************************************/
PsEngine::PsEngine(PsProject *project) :
    mProject(project)
{
}


/************************************************

 ************************************************/
void PsEngine::fillPreviewSheets(QList<PsSheet *> *sheets)
{
    fillSheets(sheets);
}


/************************************************

 ************************************************/
void PsEngine::writeMatrixText(double xOffset, double yOffset, int rotate, double scale, QTextStream *out)
{
    *out << "userdict/PStoPSsaved save put\n";
    *out << "PStoPSmatrix setmatrix\n";
    *out << QString("%1 %2 translate\n").arg(xOffset, 0, 'f').arg(yOffset, 0, 'f');
    *out << QString("%1 rotate\n").arg(rotate);
    *out << QString("%1 dup scale\n").arg(scale, 0, 'f');
    *out << "userdict/PStoPSmatrix matrix currentmatrix put\n";
}


/************************************************

 ************************************************/
void PsEngine::writeClipText(double width, double height, QTextStream *out)
{
    if (width > 0 && height > 0)
    {
        *out << "userdict/PStoPSclip{0 0 moveto\n";
        *out << QString(" %1 0 rlineto 0 %2 rlineto -%1 0 rlineto\n").arg(width,  0, 'f').arg(height, 0, 'f');
        *out << " closepath}put initclip\n";
    }
}


/************************************************

 ************************************************/
void PsEngine::writeBorderText(double borderWidth, QTextStream *out)
{
    if (borderWidth > 0)
        *out << QString("gsave clippath 0 setgray %1 setlinewidth stroke grestore\n")
                .arg(borderWidth);
}


/************************************************

 ************************************************/
void PsEngine::writeProcSetText(QTextStream *out)
{
    *out << "%%BeginProcSet: PStoPS 1 15\n";
    *out << "userdict begin\n";
    *out << "[/showpage/erasepage/copypage]{dup where{pop dup load\n";
    *out << " type/operatortype eq{ /PStoPSenablepage cvx 1 index load 1 array astore cvx {} bind /ifelse cvx 4 array astore cvx def}{pop}ifelse}{pop}ifelse}forall /PStoPSenablepage true def\n";
    *out << "[/letter/legal/executivepage/a4/a4small/b5/com10envelope\n";
    *out << " /monarchenvelope/c5envelope/dlenvelope/lettersmall/note\n";
    *out << " /folio/quarto/a5]{dup where{dup wcheck{exch{}put}\n";
    *out << " {pop{}def}ifelse}{pop}ifelse}forall\n";
    *out << "/setpagedevice {pop}bind 1 index where{dup wcheck{3 1 roll put}\n";
    *out << " {pop def}ifelse}{def}ifelse\n";
    *out << "/PStoPSmatrix matrix currentmatrix def\n";
    *out << "/PStoPSxform matrix def/PStoPSclip{clippath}def\n";
    *out << "/defaultmatrix{PStoPSmatrix exch PStoPSxform exch concatmatrix}bind def\n";
    *out << "/initmatrix{matrix defaultmatrix setmatrix}bind def\n";
    *out << "/initclip[{matrix currentmatrix PStoPSmatrix setmatrix\n";
    *out << " [{currentpoint}stopped{$error/newerror false put{newpath}}\n";
    *out << " {/newpath cvx 3 1 roll/moveto cvx 4 array astore cvx}ifelse]\n";
    *out << " {[/newpath cvx{/moveto cvx}{/lineto cvx}\n";
    *out << " {/curveto cvx}{/closepath cvx}pathforall]cvx exch pop}\n";
    *out << " stopped{$error/errorname get/invalidaccess eq{cleartomark\n";
    *out << " $error/newerror false put cvx exec}{stop}ifelse}if}bind aload pop\n";
    *out << " /initclip dup load dup type dup/operatortype eq{pop exch pop}\n";
    *out << " {dup/arraytype eq exch/packedarraytype eq or\n";
    *out << "  {dup xcheck{exch pop aload pop}{pop cvx}ifelse}\n";
    *out << "  {pop cvx}ifelse}ifelse\n";
    *out << " {newpath PStoPSclip clip newpath exec setmatrix} bind aload pop]cvx def\n";
    *out << "/initgraphics{initmatrix newpath initclip 1 setlinewidth\n";
    *out << " 0 setlinecap 0 setlinejoin []0 setdash 0 setgray\n";
    *out << " 10 setmiterlimit}bind def\n";
    *out << "end\n";
    *out << "%%EndProcSet\n";
}


/************************************************

 ************************************************/
void PsEngine::writePage(const PsProjectPage *page, QTextStream *out)
{
    if (page)
        page->writePage(out);
    else
        *out << "showpage\n";
}


/************************************************

 ************************************************/
void PsEngine::writeMatrix(const PsSheetPageSpec &spec, QTextStream *out)
{
    QPointF basePoint;
    switch (spec.rotate())
    {
    case PsSheetPageSpec::NoRotate:
        basePoint = spec.rect().topLeft();
        break;

    case PsSheetPageSpec::Rotate90:
        basePoint = spec.rect().topRight();
        break;

    case PsSheetPageSpec::Rotate180:
        basePoint = spec.rect().bottomRight();
        break;

    case PsSheetPageSpec::Rotate270:
        basePoint = spec.rect().bottomLeft();
        break;
    }

    writeMatrixText(
    /* xOffset */   basePoint.x(),
    /* yOffset */   basePoint.y(),
    /* rotate  */   spec.rotate(),
    /* scale   */   spec.scale(),
                    out);
}


/************************************************

 ************************************************/
void PsEngine::writeSheet(const PsSheet &sheet, QTextStream *out)
{
    int lastPage = sheet.count()-1;

    for (int i=0; i<sheet.count(); ++i)
    {
        const PsProjectPage *page = sheet.page(i);
        const PsSheetPageSpec spec = sheet.pageSpec(i);

        writeMatrix(spec, out);

        if (page)
        {
            writeClipText(page->rect().width(), page->rect().height(), out);
            if (project()->mPrinter->drawBorder())
                writeBorderText(1, out);
        }
        if (i != lastPage)
            *out << "/PStoPSenablepage false def\n";

        *out << "PStoPSxform concat\n";


        writePage(page, out);
        *out << "PStoPSsaved restore\n";
    }
}


/************************************************

 ************************************************/
void PsEngine::writeDocument(const QList<const PsSheet*> &sheets, QTextStream *out)
{
    *out << "%!PS-Adobe-3.0\n";
    *out << "%%BoundingBox: (atend)\n";
    *out << "%%Creator: (boomaga)\n";
    *out << "%%LanguageLevel: 2\n";
    *out << QString("%%Pages: %1\n").arg(sheets.count());
    *out << "%%EndComments\n";

    writeProcSetText(out);

    mProject->psFile()->writeProlog(out);
    mProject->psFile()->writeSetup(out);

    int n=1;
    foreach(const PsSheet *sheet, sheets)
    {
        *out << QString("%%Page: %1\n").arg(n++);
        writeSheet(*sheet, out);
    }
    mProject->psFile()->writeTrailer(out);
}


/************************************************

 ************************************************/
EngineNUp::EngineNUp(PsProject *project, int pageCountVert, int pageCountHoriz):
    PsEngine(project),
    mPageCountVert(pageCountVert),
    mPageCountHoriz(pageCountHoriz)
{
    mRotate = mPageCountVert != mPageCountHoriz;
}


/************************************************

 ************************************************/
void EngineNUp::fillSheets(QList<PsSheet *> *sheets)
{
    PsSheet *sheet;

    int pps = mPageCountVert * mPageCountHoriz;

    int s=0;
    int i=0;
    while (i < mProject->pageCount())
    {
        sheet = new PsSheet(mProject, pps);
        sheet->setSheetNum(sheets->count());
        sheet->setHint(PsSheet::HintLandscapePreview, mRotate);

        for (int j=0; j<pps; ++j)
        {
            if (i<mProject->pageCount())
            {
                PsProjectPage *page = mProject->page(i);
                sheet->setPage(j, page);
                sheet->setPageSpec(j, pageSpecForPage(page, j));
            }
            ++i;
        }
        sheets->append(sheet);
    }
}


/************************************************
    Landscape = true
    +------------------+
    | .......  ....... |
    | :     :  :     : |
    | :  0  :  :  1  : |
    | :     :  :     : |
    | .......  ....... |
    +------------------+

    Landscape = false
    +------------------+
    | .......  ....... |
    | :     :  :     : |
    | :  0  :  :  1  : |
    | :     :  :     : |
    | .......  ....... |
    | .......  ....... |
    | :     :  :     : |
    | :  2  :  :  3  : |
    | :     :  :     : |
    | .......  ....... |
    +------------------+

 ************************************************/
PsSheetPageSpec EngineNUp::pageSpecForPage(const PsProjectPage *page, int pageNumOnSheet)
{
    Printer *printer = mProject->printer();
    const qreal m = printer->internalMarhin();
    const QRectF printerRect = printer->pageRect();

    QRectF pageRect;
    pageRect.setWidth((printerRect.width()   - m * (mPageCountHoriz - 1)) / mPageCountHoriz);
    pageRect.setHeight((printerRect.height() - m * (mPageCountVert  - 1)) / mPageCountVert);

    qreal scale;
    PsSheetPageSpec::Rotation rotation;

    int col = pageNumOnSheet % mPageCountHoriz;
    int row = pageNumOnSheet / mPageCountHoriz;

    if (mRotate)
    {
        scale = qMin(
                pageRect.width()  / page->rect().height(),
                pageRect.height() / page->rect().width());

        pageRect.setWidth(page->rect().height() * scale);
        pageRect.setHeight(page->rect().width() * scale);
        rotation = PsSheetPageSpec::Rotate90;
    }
    else
    {
        scale = qMin(
                pageRect.width()  / page->rect().width(),
                pageRect.height() / page->rect().height());

        pageRect.setWidth(page->rect().width()   * scale);
        pageRect.setHeight(page->rect().height() * scale);
        rotation = PsSheetPageSpec::NoRotate;
        row = mPageCountVert - 1 - row;
    }

    QRectF r = printerRect.adjusted(-m * 0.5 , -m * 0.5, m * 0.5, m * 0.5);
    qreal x = r.left() + (r.width()  / (mPageCountHoriz * 2.0 ) * (col * 2.0 + 1));
    qreal y = r.top()  + (r.height() / (mPageCountVert  * 2.0 ) * (row * 2.0 + 1));

    pageRect.moveCenter(QPointF(x, y));
    return PsSheetPageSpec(pageRect, scale, rotation);
}


/************************************************

 ************************************************/
void EngineBooklet::fillSheets(QList<PsSheet *> *sheets)
{
    fillSheetsForBook(0, mProject->pageCount(), sheets);
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
void EngineBooklet::fillSheetsForBook(int bookStart, int bookLength, QList<PsSheet *> *sheets)
{
    int cnt = ceil(bookLength / 4.0 ) * 4;
    int n;
    PsSheet *sheet;

    for (int i = 0; i < cnt / 2; i+=2)
    {
        // Sheet 0 **************************
        sheet = new PsSheet(mProject, 2);
        sheet->setHints(PsSheet::HintDrawFold | PsSheet::HintLandscapePreview);
        sheet->setSheetNum(sheets->count());
        sheets->append(sheet);

        n = (cnt - 1) - i;
        if (n < bookLength)
        {
            PsProjectPage *page = mProject->page(n + bookStart);
            sheet->setPage(0, page);
            sheet->setPageSpec(0, pageSpecForPage(page, 0));
        }


        n = i;
        if (n < bookLength)
        {
            PsProjectPage *page = mProject->page(n + bookStart);
            sheet->setPage(1, page);
            sheet->setPageSpec(1, pageSpecForPage(page, 1));
        }


        // Sheet 0 **************************
        sheet = new PsSheet(mProject, 2);
        sheet->setHints(PsSheet::HintDrawFold | PsSheet::HintLandscapePreview);
        sheet->setSheetNum(sheets->count());
        sheets->append(sheet);

        n = i + 1;
        if (n < bookLength)
        {
            PsProjectPage *page = mProject->page(n + bookStart);
            sheet->setPage(0, page);
            sheet->setPageSpec(0, pageSpecForPage(page, 0));
        }

        n = (cnt - 1) - (i + 1);
        if (n < bookLength)
        {
            PsProjectPage *page = mProject->page(n + bookStart);
            sheet->setPage(1, page);
            sheet->setPageSpec(1, pageSpecForPage(page, 1));
        }
    }
}


/************************************************

 ************************************************/
void EngineBooklet::fillPreviewSheets(QList<PsSheet *> *sheets)
{
    fillPreviewSheetsForBook(0, mProject->pageCount(), sheets);
    if (sheets->count() > 1)
    {
        sheets->first()->setHint(PsSheet::HintOnlyRight, true);
        sheets->last()->setHint(PsSheet::HintOnlyLeft, true);
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
void EngineBooklet::fillPreviewSheetsForBook(int bookStart, int bookLength, QList<PsSheet *> *sheets)
{
    int cnt = ceil(bookLength / 4.0 ) * 4;

    PsSheet *sheet;
    for (int i = -1; i < cnt; i+=2)
    {
        sheet = new PsSheet(mProject, 2);
        sheet->setHints(PsSheet::HintDrawFold | PsSheet::HintLandscapePreview);
        sheet->setSheetNum(sheets->count());
        sheets->append(sheet);
        if (i>-1)
        {
            if (i<mProject->pageCount())
            {
                PsProjectPage *page = mProject->page(i + bookStart);
                sheet->setPage(0, page);
                sheet->setPageSpec(0, pageSpecForPage(page, 0));
            }
        }

        if (i+1<cnt)
        {
            if (i+1<mProject->pageCount())
            {
                PsProjectPage *page = mProject->page(i + 1 + bookStart);
                sheet->setPage(1, page);
                sheet->setPageSpec(1, pageSpecForPage(page, 1));
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
PsSheetPageSpec EngineBooklet::pageSpecForPage(const PsProjectPage *page, int pageNumOnSheet)
{
    Printer *printer = mProject->printer();
    QRectF printerRect = printer->pageRect();

    double scale = qMin(
                1.0 * printerRect.width() / page->rect().height(),
                (0.5 * printerRect.height() - printer->internalMarhin()) / page->rect().width()
                );

    QRectF pageRect= QRect(0, 0, page->rect().height() * scale, page->rect().width() * scale);
    pageRect.moveCenter(printerRect.center());

    if (pageNumOnSheet == 0)
        pageRect.moveBottom(printerRect.center().y() - printer->internalMarhin());
    else
        pageRect.moveTop(printerRect.center().y() + printer->internalMarhin());

    return PsSheetPageSpec(pageRect, scale, PsSheetPageSpec::Rotate90);
}




