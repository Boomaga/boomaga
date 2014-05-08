/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2014 Boomaga team https://github.com/Boomaga
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


#include "testboomaga.h"

#include <QTest>
#include <QDebug>
#include <QRect>
#include <QString>

#define protected public
#include "../kernel/layout.h"
#include "../kernel/project.h"
#include "../kernel/sheet.h"

#define COMPARE(actual, expected) \
    do {\
        if (!QTest::qCompare(QString("%1").arg(actual), QString("%1").arg(expected), #actual, #expected, __FILE__, __LINE__))\
            return;\
    } while (0)


#define FAIL(message) \
do {\
    QTest::qFail(message, __FILE__, __LINE__);\
} while (0)


/************************************************
 *
 * ***********************************************/
TestBoomaga::TestBoomaga(QObject *parent)
{

}


/************************************************
 *
 * ***********************************************/
void TestBoomaga::test_Rotation()
{
    COMPARE((NoRotate  + NoRotate  ), NoRotate  );
    COMPARE((NoRotate  + Rotate90  ), Rotate90  );
    COMPARE((NoRotate  + Rotate180 ), Rotate180 );
    COMPARE((NoRotate  + Rotate270 ), Rotate270 );

    COMPARE((Rotate90  + NoRotate  ), Rotate90  );
    COMPARE((Rotate90  + Rotate90  ), Rotate180 );
    COMPARE((Rotate90  + Rotate180 ), Rotate270 );
    COMPARE((Rotate90  + Rotate270 ), NoRotate  );

    COMPARE((Rotate180 + NoRotate  ), Rotate180 );
    COMPARE((Rotate180 + Rotate90  ), Rotate270 );
    COMPARE((Rotate180 + Rotate180 ), NoRotate  );
    COMPARE((Rotate180 + Rotate270 ), Rotate90  );

    COMPARE((Rotate270 + NoRotate  ), Rotate270 );
    COMPARE((Rotate270 + Rotate90  ), NoRotate  );
    COMPARE((Rotate270 + Rotate180 ), Rotate90  );
    COMPARE((Rotate270 + Rotate270 ), Rotate180 );


    COMPARE((NoRotate  + 0   ), NoRotate  );
    COMPARE((NoRotate  + 90  ), Rotate90  );
    COMPARE((NoRotate  + 180 ), Rotate180 );
    COMPARE((NoRotate  + 270 ), Rotate270 );
    COMPARE((NoRotate  + 360 ), NoRotate  );

    COMPARE((Rotate90  + 0   ), Rotate90  );
    COMPARE((Rotate90  + 90  ), Rotate180 );
    COMPARE((Rotate90  + 180 ), Rotate270 );
    COMPARE((Rotate90  + 270 ), NoRotate  );
    COMPARE((Rotate90  + 360 ), Rotate90  );

    COMPARE((Rotate180 + 0   ), Rotate180 );
    COMPARE((Rotate180 + 90  ), Rotate270 );
    COMPARE((Rotate180 + 180 ), NoRotate  );
    COMPARE((Rotate180 + 270 ), Rotate90  );
    COMPARE((Rotate180 + 360 ), Rotate180 );

    COMPARE((Rotate270 + 0   ), Rotate270 );
    COMPARE((Rotate270 + 90  ), NoRotate  );
    COMPARE((Rotate270 + 180 ), Rotate90  );
    COMPARE((Rotate270 + 270 ), Rotate180 );
    COMPARE((Rotate270 + 360 ), Rotate270 );



    COMPARE((NoRotate  - NoRotate  ), NoRotate  );
    COMPARE((NoRotate  - Rotate90  ), Rotate270 );
    COMPARE((NoRotate  - Rotate180 ), Rotate180 );
    COMPARE((NoRotate  - Rotate270 ), Rotate90  );

    COMPARE((Rotate90  - NoRotate  ), Rotate90  );
    COMPARE((Rotate90  - Rotate90  ), NoRotate  );
    COMPARE((Rotate90  - Rotate180 ), Rotate270 );
    COMPARE((Rotate90  - Rotate270 ), Rotate180 );

    COMPARE((Rotate180 - NoRotate  ), Rotate180 );
    COMPARE((Rotate180 - Rotate90  ), Rotate90  );
    COMPARE((Rotate180 - Rotate180 ), NoRotate  );
    COMPARE((Rotate180 - Rotate270 ), Rotate270 );

    COMPARE((Rotate270 - NoRotate  ), Rotate270 );
    COMPARE((Rotate270 - Rotate90  ), Rotate180 );
    COMPARE((Rotate270 - Rotate180 ), Rotate90  );
    COMPARE((Rotate270 - Rotate270 ), NoRotate  );


    {
        Rotation r1 = NoRotate;
        int r2 = 0;
        Rotation add1 = Rotate90;
        int add2 = 90;
        for (int i=0; i<9; i++)
        {
            COMPARE(r1, r2);
            r1+=add1;
            r2 = (r2 + add2) %360;
        }
    }

    {
        Rotation r1 = NoRotate;
        int r2 = 0;
        Rotation add1 = Rotate180;
        int add2 = 180;
        for (int i=0; i<9; i++)
        {
            COMPARE(r1, r2);
            r1+=add1;
            r2 = (r2 + add2) %360;
        }
    }

    {
        Rotation r1 = NoRotate;
        int r2 = 0;
        Rotation add1 = Rotate270;
        int add2 = 270;
        for (int i=0; i<9; i++)
        {
            COMPARE(r1, r2);
            r1+=add1;
            r2 = (r2 + add2) %360;
        }
    }


    COMPARE((NoRotate  - 0   ), NoRotate  );
    COMPARE((NoRotate  - 90  ), Rotate270  );
    COMPARE((NoRotate  - 180 ), Rotate180 );
    COMPARE((NoRotate  - 270 ), Rotate90 );
    COMPARE((NoRotate  - 360 ), NoRotate  );

    COMPARE((Rotate90  - 0   ), Rotate90  );
    COMPARE((Rotate90  - 90  ), NoRotate  );
    COMPARE((Rotate90  - 180 ), Rotate270 );
    COMPARE((Rotate90  - 270 ), Rotate180 );
    COMPARE((Rotate90  - 360 ), Rotate90  );

    COMPARE((Rotate180 - 0   ), Rotate180 );
    COMPARE((Rotate180 - 90  ), Rotate90  );
    COMPARE((Rotate180 - 180 ), NoRotate  );
    COMPARE((Rotate180 - 270 ), Rotate270 );
    COMPARE((Rotate180 - 360 ), Rotate180 );

    COMPARE((Rotate270 - 0   ), Rotate270 );
    COMPARE((Rotate270 - 90  ), Rotate180 );
    COMPARE((Rotate270 - 180 ), Rotate90  );
    COMPARE((Rotate270 - 270 ), NoRotate  );
    COMPARE((Rotate270 - 360 ), Rotate270 );


    {
        Rotation r1 = NoRotate;
        int r2 = 0;
        Rotation add1 = Rotate90;
        int add2 = 90;
        for (int i=0; i<9; i++)
        {
            COMPARE(r1, r2);
            r1-=add1;
            r2 = (r2 - add2) %360;
            if (r2 < 0)
                r2 += 360;
        }
    }

    {
        Rotation r1 = NoRotate;
        int r2 = 0;
        Rotation add1 = Rotate180;
        int add2 = 180;
        for (int i=0; i<9; i++)
        {
            COMPARE(r1, r2);
            r1-=add1;
            r2 = (r2 - add2) %360;
            if (r2 < 0)
                r2 += 360;
        }
    }

    {
        Rotation r1 = NoRotate;
        int r2 = 0;
        Rotation add1 = Rotate270;
        int add2 = 270;
        for (int i=0; i<9; i++)
        {
            COMPARE(r1, r2);
            r1-=add1;
            r2 = (r2 - add2) %360;
            if (r2 < 0)
                r2 += 360;
        }
    }

}


/************************************************
 *
 * ***********************************************/
Layout *TestBoomaga::createLayout(const QString &name)
{
    if (name == "1up")       return new LayoutNUp(1, 1);
    if (name == "1upV")       return new LayoutNUp(1, 1);
    if (name == "2up")       return new LayoutNUp(2, 1);
    if (name == "4up_Horiz") return new LayoutNUp(2, 2, Qt::Horizontal);
    if (name == "4up_Vert")  return new LayoutNUp(2, 2, Qt::Vertical);
    if (name == "8up_Horiz") return new LayoutNUp(4, 2, Qt::Horizontal);
    if (name == "8up_Vert")  return new LayoutNUp(4, 2, Qt::Vertical);
    if (name == "booklet")   return new LayoutBooklet();
    FAIL(QString("Unknown layout %1").arg(name).toLocal8Bit());
    return 0;
}


/************************************************
 *
 * ***********************************************/
int TestBoomaga::StrToRotation(const QString &str)
{
    if (str.toUpper() == "NOROTATE" )   return NoRotate;
    if (str.toUpper() == "ROTATE90" )   return Rotate90;
    if (str.toUpper() == "ROTATE180")   return Rotate180;
    if (str.toUpper() == "ROTATE270")   return Rotate270;

    if (str.toUpper() == "0" )    return NoRotate;
    if (str.toUpper() == "90" )   return Rotate90;
    if (str.toUpper() == "180")   return Rotate180;
    if (str.toUpper() == "270")   return Rotate270;

    FAIL(QString("Unknown rotation %1").arg(str).toLocal8Bit());
    return 0;
}


/************************************************
 *
 * ***********************************************/
Sheet *TestBoomaga::createSheet(const QString &definition)
{
    QString def = definition.trimmed();
    if (def.startsWith('['))
        def = def.mid(1, def.length()-2);

    QStringList items = def.split(',');

    Sheet *sheet = new Sheet(items.count(), 0);
    for (int i=0; i<items.count(); ++i)
    {
        PdfPageInfo pdfInfo;
        QString s = items.at(i).trimmed().toUpper();
        if      (s.startsWith("0"))   pdfInfo.rotate = 0;
        else if (s.startsWith("90"))  pdfInfo.rotate = 90;
        else if (s.startsWith("180")) pdfInfo.rotate = 180;
        else if (s.startsWith("270")) pdfInfo.rotate = 270;
        else FAIL(QString("Unknown rotation %1").arg(s).toLocal8Bit());

        if (s.endsWith("P"))
        {
            pdfInfo.cropBox  = QRectF(0, 0, 100, 200);
            pdfInfo.mediaBox = QRectF(0, 0, 100, 200);
        }
        else if (s.endsWith("L"))
        {
            pdfInfo.cropBox  = QRectF(0, 0, 200, 100);
            pdfInfo.mediaBox = QRectF(0, 0, 200, 100);
        }
        else
        {
            FAIL(QString("Unknown orientation '%1'").arg(s).toLocal8Bit());
        }

        ProjectPage *page = new ProjectPage();
        page->setPdfInfo(pdfInfo);

        sheet->setPage(i, page);
    }

    return sheet;
}


/************************************************
 *
 * ***********************************************/
Sheet *TestBoomaga::createSheet(int pagePerSheet, int pageRotation, QRectF mediaBox, QRectF cropBox)
{
    Sheet *sheet = new Sheet(pagePerSheet, 0);
    for(int i=0; i<pagePerSheet; ++i)
    {
        PdfPageInfo pdfInfo;
        pdfInfo.rotate = pageRotation;
        pdfInfo.cropBox = cropBox;
        pdfInfo.mediaBox = mediaBox;

        ProjectPage *page = new ProjectPage();
        page->setPdfInfo(pdfInfo);

        sheet->setPage(i, page);
    }

    return sheet;
}




/************************************************
 *
 * ***********************************************/
void TestBoomaga::test_SheetRotation()
{
    QFETCH(int,      expected);

    QString dataTag = QTest::currentDataTag();
    QString layoutName = dataTag.section(";", 0, 0);
    QString sheetDef =  dataTag.section(";", 1);

    Layout *layout = createLayout(layoutName);
    Sheet *sheet = createSheet(sheetDef);

    int result = layout->calcSheetRotation(*sheet);
    QCOMPARE(result, expected);

    delete layout;
    delete sheet;
}


/************************************************
 *
 * ***********************************************/
void TestBoomaga::test_SheetRotation_data()
{
    QTest::addColumn<int>("expected");

    QTest::newRow("1up; [0P]"   ) << 0;
    QTest::newRow("1up; [0L]"   ) << 90;
    QTest::newRow("1up; [90P]"  ) << 90;
    QTest::newRow("1up; [90L]"  ) << 180;
    QTest::newRow("1up; [180P]" ) << 180;
    QTest::newRow("1up; [180L]" ) << 270;
    QTest::newRow("1up; [270P]" ) << 270;
    QTest::newRow("1up; [270L]" ) << 0;


    QTest::newRow("2up; [0P]"   ) << 270;
    QTest::newRow("2up; [0L]"   ) << 0;
    QTest::newRow("2up; [90P]"  ) << 0;
    QTest::newRow("2up; [90L]"  ) << 90;
    QTest::newRow("2up; [180P]" ) << 90;
    QTest::newRow("2up; [180L]" ) << 180;
    QTest::newRow("2up; [270P]" ) << 180;
    QTest::newRow("2up; [270L]" ) << 270;


    QTest::newRow("4up_Horiz; [0P]"   ) << 0;
    QTest::newRow("4up_Horiz; [0L]"   ) << 90;
    QTest::newRow("4up_Horiz; [90P]"  ) << 90;
    QTest::newRow("4up_Horiz; [90L]"  ) << 180;
    QTest::newRow("4up_Horiz; [180P]" ) << 180;
    QTest::newRow("4up_Horiz; [180L]" ) << 270;
    QTest::newRow("4up_Horiz; [270P]" ) << 270;
    QTest::newRow("4up_Horiz; [270L]" ) << 0;

    QTest::newRow("4up_Vert;  [0P]"   ) << 0;
    QTest::newRow("4up_Vert;  [0L]"   ) << 90;
    QTest::newRow("4up_Vert;  [90P]"  ) << 90;
    QTest::newRow("4up_Vert;  [90L]"  ) << 180;
    QTest::newRow("4up_Vert;  [180P]" ) << 180;
    QTest::newRow("4up_Vert;  [180L]" ) << 270;
    QTest::newRow("4up_Vert;  [270P]" ) << 270;
    QTest::newRow("4up_Vert;  [270L]" ) << 0;


    QTest::newRow("8up_Horiz; [0P]"   ) << 270;
    QTest::newRow("8up_Horiz; [0L]"   ) << 0;
    QTest::newRow("8up_Horiz; [90P]"  ) << 0;
    QTest::newRow("8up_Horiz; [90L]"  ) << 90;
    QTest::newRow("8up_Horiz; [180P]" ) << 90;
    QTest::newRow("8up_Horiz; [180L]" ) << 180;
    QTest::newRow("8up_Horiz; [270P]" ) << 180;
    QTest::newRow("8up_Horiz; [270L]" ) << 270;

    QTest::newRow("8up_Vert;  [0P]"   ) << 270;
    QTest::newRow("8up_Vert;  [0L]"   ) << 0;
    QTest::newRow("8up_Vert;  [90P]"  ) << 0;
    QTest::newRow("8up_Vert;  [90L]"  ) << 90;
    QTest::newRow("8up_Vert;  [180P]" ) << 90;
    QTest::newRow("8up_Vert;  [180L]" ) << 180;
    QTest::newRow("8up_Vert;  [270P]" ) << 180;
    QTest::newRow("8up_Vert;  [270L]" ) << 270;


    QTest::newRow("booklet; [0P]"   ) << 270;
    QTest::newRow("booklet; [0L]"   ) << 0;
    QTest::newRow("booklet; [90P]"  ) << 0;
    QTest::newRow("booklet; [90L]"  ) << 90;
    QTest::newRow("booklet; [180P]" ) << 90;
    QTest::newRow("booklet; [180L]" ) << 180;
    QTest::newRow("booklet; [270P]" ) << 180;
    QTest::newRow("booklet; [270L]" ) << 270;

}


/************************************************
 *
 * ***********************************************/
void TestBoomaga::test_PageRotation()
{
    QFETCH(int,      expected);

    QString dataTag = QTest::currentDataTag();
    QString layoutName = dataTag.section(";", 0, 0);
    QString sheetDef =  dataTag.section(";", 1);

    Layout *layout = createLayout(layoutName);
    Sheet *sheet = createSheet(sheetDef);

    TransformSpec spec = layout->transformSpec(sheet, 0);
    QCOMPARE((int)spec.rotation, expected);

    delete layout;
    delete sheet;
}


/************************************************
 *
 * ***********************************************/
void TestBoomaga::test_PageRotation_data()
{
    QTest::addColumn<int>("expected");

    QTest::newRow("1up; [0P]"   ) << 0;
    QTest::newRow("1up; [0L]"   ) << 270;
    QTest::newRow("1up; [90P]"  ) << 0;
    QTest::newRow("1up; [90L]"  ) << 270;
    QTest::newRow("1up; [180P]" ) << 0;
    QTest::newRow("1up; [180L]" ) << 270;
    QTest::newRow("1up; [270P]" ) << 0;
    QTest::newRow("1up; [270L]" ) << 270;


    QTest::newRow("2up; [0P]"   ) << 90;
    QTest::newRow("2up; [0L]"   ) << 180;
    QTest::newRow("2up; [90P]"  ) << 90;
    QTest::newRow("2up; [90L]"  ) << 180;
    QTest::newRow("2up; [180P]" ) << 90;
    QTest::newRow("2up; [180L]" ) << 180;
    QTest::newRow("2up; [270P]" ) << 90;
    QTest::newRow("2up; [270L]" ) << 180;


    QTest::newRow("4up_Horiz; [0P]"   ) << 0;
    QTest::newRow("4up_Horiz; [0L]"   ) << 270;
    QTest::newRow("4up_Horiz; [90P]"  ) << 0;
    QTest::newRow("4up_Horiz; [90L]"  ) << 270;
    QTest::newRow("4up_Horiz; [180P]" ) << 0;
    QTest::newRow("4up_Horiz; [180L]" ) << 270;
    QTest::newRow("4up_Horiz; [270P]" ) << 0;
    QTest::newRow("4up_Horiz; [270L]" ) << 270;

    QTest::newRow("4up_Vert;  [0P]"   ) << 0;
    QTest::newRow("4up_Vert;  [0L]"   ) << 270;
    QTest::newRow("4up_Vert;  [90P]"  ) << 0;
    QTest::newRow("4up_Vert;  [90L]"  ) << 270;
    QTest::newRow("4up_Vert;  [180P]" ) << 0;
    QTest::newRow("4up_Vert;  [180L]" ) << 270;
    QTest::newRow("4up_Vert;  [270P]" ) << 0;
    QTest::newRow("4up_Vert;  [270L]" ) << 270;


    QTest::newRow("8up_Horiz; [0P]"   ) << 90;
    QTest::newRow("8up_Horiz; [0L]"   ) << 180;
    QTest::newRow("8up_Horiz; [90P]"  ) << 90;
    QTest::newRow("8up_Horiz; [90L]"  ) << 180;
    QTest::newRow("8up_Horiz; [180P]" ) << 90;
    QTest::newRow("8up_Horiz; [180L]" ) << 180;
    QTest::newRow("8up_Horiz; [270P]" ) << 90;
    QTest::newRow("8up_Horiz; [270L]" ) << 180;

    QTest::newRow("8up_Vert;  [0P]"   ) << 90;
    QTest::newRow("8up_Vert;  [0L]"   ) << 180;
    QTest::newRow("8up_Vert;  [90P]"  ) << 90;
    QTest::newRow("8up_Vert;  [90L]"  ) << 180;
    QTest::newRow("8up_Vert;  [180P]" ) << 90;
    QTest::newRow("8up_Vert;  [180L]" ) << 180;
    QTest::newRow("8up_Vert;  [270P]" ) << 90;
    QTest::newRow("8up_Vert;  [270L]" ) << 180;


    QTest::newRow("booklet; [0P]"   ) << 90;
    QTest::newRow("booklet; [0L]"   ) << 180;
    QTest::newRow("booklet; [90P]"  ) << 90;
    QTest::newRow("booklet; [90L]"  ) << 180;
    QTest::newRow("booklet; [180P]" ) << 90;
    QTest::newRow("booklet; [180L]" ) << 180;
    QTest::newRow("booklet; [270P]" ) << 90;
    QTest::newRow("booklet; [270L]" ) << 180;
}


/************************************************
 *
 * ***********************************************/
void TestBoomaga::test_PagePosition()
{
    QFETCH(int,      expectedCol);
    QFETCH(int,      expectedRow);

    QString dataTag = QTest::currentDataTag();
    QString layoutName = dataTag.section(";", 0, 0);
    QString sheetDef =  dataTag.section(";", 1, 1);
    bool ok;
    int pageNum = dataTag.section(";", 2, 2).toInt(&ok);
    if (!ok)
        FAIL(QString("Incorrect page number '%1'").arg(dataTag.section(";", 2, 2)).toLocal8Bit());

    Layout *layout = createLayout(layoutName);
    Sheet *sheet = createSheet(sheetDef);

    Layout::PagePosition pos = layout->calcPagePosition(sheet, pageNum);
    QCOMPARE((int)pos.col, expectedCol);
    QCOMPARE((int)pos.row, expectedRow);

    delete layout;
    delete sheet;
}


/************************************************
 *
 * ***********************************************/
void TestBoomaga::test_PagePosition_data()
{
    QTest::addColumn<int>("expectedCol");
    QTest::addColumn<int>("expectedRow");

    //          Layout   Pages  pageNum   Col  Row
    QTest::newRow("1up; [   0P ]; 0"   )  << 0 << 0;
    QTest::newRow("1up; [   0L ]; 0"   )  << 0 << 0;
    QTest::newRow("1up; [  90P ]; 0"   )  << 0 << 0;
    QTest::newRow("1up; [  90L ]; 0"   )  << 0 << 0;
    QTest::newRow("1up; [ 180P ]; 0"   )  << 0 << 0;
    QTest::newRow("1up; [ 180L ]; 0"   )  << 0 << 0;
    QTest::newRow("1up; [ 270P ]; 0"   )  << 0 << 0;
    QTest::newRow("1up; [ 270L ]; 0"   )  << 0 << 0;

    //          Layout   Pages  pageNum   Col  Row
    QTest::newRow("2up; [   0P ]; 0"   )  << 0 << 0;
    QTest::newRow("2up; [   0P ]; 1"   )  << 0 << 1;

    QTest::newRow("2up; [   0L ]; 0"   )  << 0 << 0;
    QTest::newRow("2up; [   0L ]; 1"   )  << 0 << 1;

    QTest::newRow("2up; [  90P ]; 0"   )  << 0 << 0;
    QTest::newRow("2up; [  90P ]; 1"   )  << 0 << 1;

    QTest::newRow("2up; [  90L ]; 0"   )  << 0 << 0;
    QTest::newRow("2up; [  90L ]; 1"   )  << 0 << 1;

    QTest::newRow("2up; [ 180P ]; 0"   )  << 0 << 0;
    QTest::newRow("2up; [ 180P ]; 1"   )  << 0 << 1;

    QTest::newRow("2up; [ 180L ]; 0"   )  << 0 << 0;
    QTest::newRow("2up; [ 180L ]; 1"   )  << 0 << 1;

    QTest::newRow("2up; [ 270P ]; 0"   )  << 0 << 0;
    QTest::newRow("2up; [ 270P ]; 1"   )  << 0 << 1;

    QTest::newRow("2up; [ 270L ]; 0"   )  << 0 << 0;
    QTest::newRow("2up; [ 270L ]; 1"   )  << 0 << 1;


    //          Layout        Pages   pageNum   Col  Row
    QTest::newRow("4up_Horiz; [   0P ]; 0"   )  << 0 << 0;
    QTest::newRow("4up_Horiz; [   0P ]; 1"   )  << 1 << 0;
    QTest::newRow("4up_Horiz; [   0P ]; 2"   )  << 0 << 1;
    QTest::newRow("4up_Horiz; [   0P ]; 3"   )  << 1 << 1;

    QTest::newRow("4up_Horiz; [  90P ]; 0"   )  << 0 << 0;
    QTest::newRow("4up_Horiz; [  90P ]; 1"   )  << 1 << 0;
    QTest::newRow("4up_Horiz; [  90P ]; 2"   )  << 0 << 1;
    QTest::newRow("4up_Horiz; [  90P ]; 3"   )  << 1 << 1;

    QTest::newRow("4up_Horiz; [ 180P ]; 0"   )  << 0 << 0;
    QTest::newRow("4up_Horiz; [ 180P ]; 1"   )  << 1 << 0;
    QTest::newRow("4up_Horiz; [ 180P ]; 2"   )  << 0 << 1;
    QTest::newRow("4up_Horiz; [ 180P ]; 3"   )  << 1 << 1;

    QTest::newRow("4up_Horiz; [ 270P ]; 0"   )  << 0 << 0;
    QTest::newRow("4up_Horiz; [ 270P ]; 1"   )  << 1 << 0;
    QTest::newRow("4up_Horiz; [ 270P ]; 2"   )  << 0 << 1;
    QTest::newRow("4up_Horiz; [ 270P ]; 3"   )  << 1 << 1;


    //          Layout        Pages   pageNum   Col  Row
    QTest::newRow("4up_Vert;  [   0P ]; 0"   )  << 0 << 0;
    QTest::newRow("4up_Vert;  [   0P ]; 1"   )  << 0 << 1;
    QTest::newRow("4up_Vert;  [   0P ]; 2"   )  << 1 << 0;
    QTest::newRow("4up_Vert;  [   0P ]; 3"   )  << 1 << 1;

    QTest::newRow("4up_Vert;  [  90P ]; 0"   )  << 0 << 0;
    QTest::newRow("4up_Vert;  [  90P ]; 1"   )  << 0 << 1;
    QTest::newRow("4up_Vert;  [  90P ]; 2"   )  << 1 << 0;
    QTest::newRow("4up_Vert;  [  90P ]; 3"   )  << 1 << 1;

    QTest::newRow("4up_Vert;  [ 180P ]; 0"   )  << 0 << 0;
    QTest::newRow("4up_Vert;  [ 180P ]; 1"   )  << 0 << 1;
    QTest::newRow("4up_Vert;  [ 180P ]; 2"   )  << 1 << 0;
    QTest::newRow("4up_Vert;  [ 180P ]; 3"   )  << 1 << 1;

    QTest::newRow("4up_Vert;  [ 270P ]; 0"   )  << 0 << 0;
    QTest::newRow("4up_Vert;  [ 270P ]; 1"   )  << 0 << 1;
    QTest::newRow("4up_Vert;  [ 270P ]; 2"   )  << 1 << 0;
    QTest::newRow("4up_Vert;  [ 270P ]; 3"   )  << 1 << 1;


    //          Layout        Pages   pageNum   Col  Row
    QTest::newRow("8up_Horiz; [   0P ]; 0"   )  << 0 << 0;
    QTest::newRow("8up_Horiz; [   0P ]; 1"   )  << 1 << 0;
    QTest::newRow("8up_Horiz; [   0P ]; 2"   )  << 0 << 1;
    QTest::newRow("8up_Horiz; [   0P ]; 3"   )  << 1 << 1;
    QTest::newRow("8up_Horiz; [   0P ]; 4"   )  << 0 << 2;
    QTest::newRow("8up_Horiz; [   0P ]; 5"   )  << 1 << 2;
    QTest::newRow("8up_Horiz; [   0P ]; 6"   )  << 0 << 3;
    QTest::newRow("8up_Horiz; [   0P ]; 7"   )  << 1 << 3;

    QTest::newRow("8up_Horiz; [  90P ]; 0"   )  << 0 << 0;
    QTest::newRow("8up_Horiz; [  90P ]; 1"   )  << 1 << 0;
    QTest::newRow("8up_Horiz; [  90P ]; 2"   )  << 0 << 1;
    QTest::newRow("8up_Horiz; [  90P ]; 3"   )  << 1 << 1;
    QTest::newRow("8up_Horiz; [  90P ]; 4"   )  << 0 << 2;
    QTest::newRow("8up_Horiz; [  90P ]; 5"   )  << 1 << 2;
    QTest::newRow("8up_Horiz; [  90P ]; 6"   )  << 0 << 3;
    QTest::newRow("8up_Horiz; [  90P ]; 7"   )  << 1 << 3;

    QTest::newRow("8up_Horiz; [ 180P ]; 0"   )  << 0 << 0;
    QTest::newRow("8up_Horiz; [ 180P ]; 1"   )  << 1 << 0;
    QTest::newRow("8up_Horiz; [ 180P ]; 2"   )  << 0 << 1;
    QTest::newRow("8up_Horiz; [ 180P ]; 3"   )  << 1 << 1;
    QTest::newRow("8up_Horiz; [ 180P ]; 4"   )  << 0 << 2;
    QTest::newRow("8up_Horiz; [ 180P ]; 5"   )  << 1 << 2;
    QTest::newRow("8up_Horiz; [ 180P ]; 6"   )  << 0 << 3;
    QTest::newRow("8up_Horiz; [ 180P ]; 7"   )  << 1 << 3;

    QTest::newRow("8up_Horiz; [ 270P ]; 0"   )  << 0 << 0;
    QTest::newRow("8up_Horiz; [ 270P ]; 1"   )  << 1 << 0;
    QTest::newRow("8up_Horiz; [ 270P ]; 2"   )  << 0 << 1;
    QTest::newRow("8up_Horiz; [ 270P ]; 3"   )  << 1 << 1;
    QTest::newRow("8up_Horiz; [ 270P ]; 4"   )  << 0 << 2;
    QTest::newRow("8up_Horiz; [ 270P ]; 5"   )  << 1 << 2;
    QTest::newRow("8up_Horiz; [ 270P ]; 6"   )  << 0 << 3;
    QTest::newRow("8up_Horiz; [ 270P ]; 7"   )  << 1 << 3;

    //          Layout        Pages   pageNum   Col  Row
    QTest::newRow("8up_Vert;  [   0P ]; 0"   )  << 0 << 0;
    QTest::newRow("8up_Vert;  [   0P ]; 1"   )  << 0 << 1;
    QTest::newRow("8up_Vert;  [   0P ]; 2"   )  << 0 << 2;
    QTest::newRow("8up_Vert;  [   0P ]; 3"   )  << 0 << 3;
    QTest::newRow("8up_Vert;  [   0P ]; 4"   )  << 1 << 0;
    QTest::newRow("8up_Vert;  [   0P ]; 5"   )  << 1 << 1;
    QTest::newRow("8up_Vert;  [   0P ]; 6"   )  << 1 << 2;
    QTest::newRow("8up_Vert;  [   0P ]; 7"   )  << 1 << 3;

    QTest::newRow("8up_Vert;  [  90P ]; 0"   )  << 0 << 0;
    QTest::newRow("8up_Vert;  [  90P ]; 1"   )  << 0 << 1;
    QTest::newRow("8up_Vert;  [  90P ]; 2"   )  << 0 << 2;
    QTest::newRow("8up_Vert;  [  90P ]; 3"   )  << 0 << 3;
    QTest::newRow("8up_Vert;  [  90P ]; 4"   )  << 1 << 0;
    QTest::newRow("8up_Vert;  [  90P ]; 5"   )  << 1 << 1;
    QTest::newRow("8up_Vert;  [  90P ]; 6"   )  << 1 << 2;
    QTest::newRow("8up_Vert;  [  90P ]; 7"   )  << 1 << 3;

    QTest::newRow("8up_Vert;  [ 180P ]; 0"   )  << 0 << 0;
    QTest::newRow("8up_Vert;  [ 180P ]; 1"   )  << 0 << 1;
    QTest::newRow("8up_Vert;  [ 180P ]; 2"   )  << 0 << 2;
    QTest::newRow("8up_Vert;  [ 180P ]; 3"   )  << 0 << 3;
    QTest::newRow("8up_Vert;  [ 180P ]; 4"   )  << 1 << 0;
    QTest::newRow("8up_Vert;  [ 180P ]; 5"   )  << 1 << 1;
    QTest::newRow("8up_Vert;  [ 180P ]; 6"   )  << 1 << 2;
    QTest::newRow("8up_Vert;  [ 180P ]; 7"   )  << 1 << 3;

    QTest::newRow("8up_Vert;  [ 270P ]; 0"   )  << 0 << 0;
    QTest::newRow("8up_Vert;  [ 270P ]; 1"   )  << 0 << 1;
    QTest::newRow("8up_Vert;  [ 270P ]; 2"   )  << 0 << 2;
    QTest::newRow("8up_Vert;  [ 270P ]; 3"   )  << 0 << 3;
    QTest::newRow("8up_Vert;  [ 270P ]; 4"   )  << 1 << 0;
    QTest::newRow("8up_Vert;  [ 270P ]; 5"   )  << 1 << 1;
    QTest::newRow("8up_Vert;  [ 270P ]; 6"   )  << 1 << 2;
    QTest::newRow("8up_Vert;  [ 270P ]; 7"   )  << 1 << 3;


    //          Layout      Pages  pageNum   Col  Row
    QTest::newRow("booklet; [   0P ]; 0"   )  << 0 << 0;
    QTest::newRow("booklet; [   0P ]; 1"   )  << 0 << 1;

    QTest::newRow("booklet; [   0L ]; 0"   )  << 0 << 0;
    QTest::newRow("booklet; [   0L ]; 1"   )  << 0 << 1;

    QTest::newRow("booklet; [  90P ]; 0"   )  << 0 << 0;
    QTest::newRow("booklet; [  90P ]; 1"   )  << 0 << 1;

    QTest::newRow("booklet; [  90L ]; 0"   )  << 0 << 0;
    QTest::newRow("booklet; [  90L ]; 1"   )  << 0 << 1;

    QTest::newRow("booklet; [ 180P ]; 0"   )  << 0 << 0;
    QTest::newRow("booklet; [ 180P ]; 1"   )  << 0 << 1;

    QTest::newRow("booklet; [ 180L ]; 0"   )  << 0 << 0;
    QTest::newRow("booklet; [ 180L ]; 1"   )  << 0 << 1;

    QTest::newRow("booklet; [ 270P ]; 0"   )  << 0 << 0;
    QTest::newRow("booklet; [ 270P ]; 1"   )  << 0 << 1;

    QTest::newRow("booklet; [ 270L ]; 0"   )  << 0 << 0;
    QTest::newRow("booklet; [ 270L ]; 1"   )  << 0 << 1;
}



QTEST_MAIN(TestBoomaga)
