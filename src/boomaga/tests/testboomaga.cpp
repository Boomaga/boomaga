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
#include <QStringList>
#include <QList>
#include <QTemporaryFile>
#include <QUrl>

#define protected public
#include "../kernel/layout.h"
#include "../kernel/project.h"
#include "../kernel/sheet.h"
#include "iofiles/boofile.h"
#include "../boomagatypes.h"
#include "../kernel/projectpage.h"
#include "../settings.h"
#include "../../common.h"


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
TestBoomaga::TestBoomaga(QObject *parent):
    QObject(parent),
    mDataDir(TEST_DATA_DIR),
    mTmpDir(TEST_OUT_DIR)
{
}


/************************************************
 *
 ************************************************/
void TestBoomaga::initTestCase()
{
    QTemporaryFile *tmpFile = new QTemporaryFile(this);
    if (!tmpFile->open())
        QFAIL("Can't create temporary file");

    Settings::setFileName(tmpFile->fileName());
}


/************************************************
 *
 * ***********************************************/
void TestBoomaga::test_RotationType()
{
    COMPARE(intToRotation(0),          NoRotate);
    COMPARE(intToRotation(90),         Rotate90);
    COMPARE(intToRotation(180),        Rotate180);
    COMPARE(intToRotation(270),        Rotate270);

    COMPARE(intToRotation(360 + 0),    NoRotate);
    COMPARE(intToRotation(360 + 90),   Rotate90);
    COMPARE(intToRotation(360 + 180),  Rotate180);
    COMPARE(intToRotation(360 + 270),  Rotate270);

    COMPARE(intToRotation(720 + 0),    NoRotate);
    COMPARE(intToRotation(720 + 90),   Rotate90);
    COMPARE(intToRotation(720 + 180),  Rotate180);
    COMPARE(intToRotation(720 + 270),  Rotate270);

    COMPARE(intToRotation(-0),         NoRotate);
    COMPARE(intToRotation(-90),        Rotate270);
    COMPARE(intToRotation(-180),       Rotate180);
    COMPARE(intToRotation(-270),       Rotate90);

    COMPARE(intToRotation(360 - 0),    NoRotate);
    COMPARE(intToRotation(360 - 90),   Rotate270);
    COMPARE(intToRotation(360 - 180),  Rotate180);
    COMPARE(intToRotation(360 - 270),  Rotate90);

    COMPARE(intToRotation(720 - 0),    NoRotate);
    COMPARE(intToRotation(720 - 90),   Rotate270);
    COMPARE(intToRotation(720 - 180),  Rotate180);
    COMPARE(intToRotation(720 - 270),  Rotate90);



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


    COMPARE((NoRotate  +     0 ), NoRotate  );
    COMPARE((NoRotate  +    90 ), Rotate90  );
    COMPARE((NoRotate  +   180 ), Rotate180 );
    COMPARE((NoRotate  +   270 ), Rotate270 );
    COMPARE((NoRotate  +   360 ), NoRotate  );
    COMPARE((NoRotate  +   450 ), Rotate90  );

    COMPARE((NoRotate  +  (-90)), Rotate270 );
    COMPARE((NoRotate  + (-180)), Rotate180 );
    COMPARE((NoRotate  + (-270)), Rotate90  );
    COMPARE((NoRotate  + (-360)), NoRotate  );
    COMPARE((NoRotate  + (-450)), Rotate270 );


    COMPARE((Rotate90  +     0 ), Rotate90  );
    COMPARE((Rotate90  +    90 ), Rotate180 );
    COMPARE((Rotate90  +   180 ), Rotate270 );
    COMPARE((Rotate90  +   270 ), NoRotate  );
    COMPARE((Rotate90  +   360 ), Rotate90  );
    COMPARE((Rotate90  +   450 ), Rotate180 );

    COMPARE((Rotate90  +  (-90)), NoRotate  );
    COMPARE((Rotate90  + (-180)), Rotate270 );
    COMPARE((Rotate90  + (-270)), Rotate180 );
    COMPARE((Rotate90  + (-360)), Rotate90  );
    COMPARE((Rotate90  + (-450)), NoRotate  );


    COMPARE((Rotate180 +     0 ), Rotate180 );
    COMPARE((Rotate180 +    90 ), Rotate270 );
    COMPARE((Rotate180 +   180 ), NoRotate  );
    COMPARE((Rotate180 +   270 ), Rotate90  );
    COMPARE((Rotate180 +   360 ), Rotate180 );
    COMPARE((Rotate180 +   450 ), Rotate270 );

    COMPARE((Rotate180 +  (-90)), Rotate90  );
    COMPARE((Rotate180 + (-180)), NoRotate  );
    COMPARE((Rotate180 + (-270)), Rotate270 );
    COMPARE((Rotate180 + (-360)), Rotate180 );
    COMPARE((Rotate180 + (-450)), Rotate90  );


    COMPARE((Rotate270 +     0 ), Rotate270 );
    COMPARE((Rotate270 +    90 ), NoRotate  );
    COMPARE((Rotate270 +   180 ), Rotate90  );
    COMPARE((Rotate270 +   270 ), Rotate180 );
    COMPARE((Rotate270 +   360 ), Rotate270 );
    COMPARE((Rotate270 +   450 ), NoRotate  );

    COMPARE((Rotate270 +  (-90)), Rotate180 );
    COMPARE((Rotate270 + (-180)), Rotate90  );
    COMPARE((Rotate270 + (-270)), NoRotate  );
    COMPARE((Rotate270 + (-360)), Rotate270 );
    COMPARE((Rotate270 + (-450)), Rotate180 );


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

 ************************************************/
void TestBoomaga::test_ProjectRotation()
{
    QFETCH(int,      exp);
    Rotation expected = NoRotate + exp;

    QString dataTag = QTest::currentDataTag();
    QString layoutName = dataTag.section(";", 0, 0);
    QString pagesDef =  dataTag.section(";", 1);

    Layout *layout = createLayout(layoutName);
    QList<ProjectPage*> pages = createPages(pagesDef);

    Rotation result = project->calcRotation(pages, layout);
    QCOMPARE((int)result, (int)expected);

    delete layout;
    qDeleteAll(pages);

}

/************************************************

 ************************************************/
void TestBoomaga::test_ProjectRotation_data()
{
    QTest::addColumn<int>("exp");

    QTest::newRow("1up; [  , 0P]"   ) <<    0;
    QTest::newRow("1up; [  , 0L]"   ) <<   90;
    QTest::newRow("1up; [0P, 0P]"   ) <<    0;
    QTest::newRow("1up; [0P, 0L]"   ) <<    0;
    QTest::newRow("1up; [0L, 0P]"   ) <<   90;
    QTest::newRow("1up; [0L, 0L]"   ) <<   90;

    QTest::newRow("2up; [  , 0P]"   ) <<   90;
    QTest::newRow("2up; [  , 0L]"   ) <<    0;
    QTest::newRow("2up; [0P, 0P]"   ) <<   90;
    QTest::newRow("2up; [0P, 0L]"   ) <<   90;
    QTest::newRow("2up; [0L, 0P]"   ) <<    0;
    QTest::newRow("2up; [0L, 0L]"   ) <<    0;
}



/************************************************
 *
 * ***********************************************/
LayoutNUp *TestBoomaga::createLayout(const QString &name)
{
    if (name == "1up")       return new LayoutNUp(1, 1);
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
Rotation TestBoomaga::StrToRotation(const QString &str)
{
    QString s=str.toUpper().trimmed();
    if (s == "NOROTATE" )   return NoRotate;
    if (s == "ROTATE90" )   return Rotate90;
    if (s == "ROTATE180")   return Rotate180;
    if (s == "ROTATE270")   return Rotate270;

    if (s == "0" )    return NoRotate;
    if (s == "90" )   return Rotate90;
    if (s == "180")   return Rotate180;
    if (s == "270")   return Rotate270;

    if (s == "P" )    return NoRotate;
    if (s == "L" )    return Rotate90;


    FAIL(QString("Unknown rotation '%1'").arg(str).toLocal8Bit());
    return NoRotate;
}


/************************************************

 ************************************************/
QList<ProjectPage *> TestBoomaga::createPages(const QString &definition)
{
    QString def = definition.trimmed();
    if (def.startsWith('['))
        def = def.mid(1, def.length()-2);

    QStringList items = def.split(',', QString::SkipEmptyParts);
    QList<ProjectPage *> res;
    for (int i=0; i<items.count(); ++i)
    {
        PdfPageInfo pdfInfo;
        QString s = items.at(i).trimmed().toUpper();
        if (!s.isEmpty())
        {
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

            res << page;
        }
        else
        {
            res << 0;
        }
    }

    return res;
}


/************************************************
 *
 * ***********************************************/
Sheet *TestBoomaga::createSheet(const QString &definition)
{
    QList<ProjectPage *> pages = createPages(definition);

    Sheet *sheet = new Sheet(pages.count(), 0);
    for(int i=0; i< pages.count(); ++i)
    {
        sheet->setPage(i, pages.at(i));
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
void TestBoomaga::test_PageRotation()
{
    QFETCH(int,      exp);
    Rotation expected = NoRotate + exp;

    QString dataTag = QTest::currentDataTag();
    QString layoutName = dataTag.section(";", 0, 0);
    QString sheetRotationName = dataTag.section(";", 1, 1);
    QString sheetDef =  dataTag.section(";", 2);

    Rotation sheetRotation = StrToRotation(sheetRotationName);


    Layout *layout = createLayout(layoutName);
    Sheet *sheet = createSheet(sheetDef);

    Rotation result = layout->calcPageRotation(sheet->page(0), sheetRotation);
    QCOMPARE((int)result, (int)expected);

    delete layout;
    delete sheet;
}


/************************************************
 *
 * ***********************************************/
void TestBoomaga::test_PageRotation_data()
{
    QTest::addColumn<int>("exp");

    // :::::::::::::::::::::::::::::::::::::
    QTest::newRow("1up; P; [0P]"   ) <<    0;
    QTest::newRow("1up; P; [90P]"  ) <<    0;
    QTest::newRow("1up; P; [180P]" ) <<  180;
    QTest::newRow("1up; P; [270P]" ) <<  180;

    QTest::newRow("1up; P; [0L]"   ) <<  -90;
    QTest::newRow("1up; P; [90L]"  ) <<   90;
    QTest::newRow("1up; P; [180L]" ) <<   90;
    QTest::newRow("1up; P; [270L]" ) <<  -90;

    QTest::newRow("1up; L; [0P]"   ) << -180;
    QTest::newRow("1up; L; [90P]"  ) <<    0;
    QTest::newRow("1up; L; [180P]" ) <<    0;
    QTest::newRow("1up; L; [270P]" ) <<  180;

    QTest::newRow("1up; L; [0L]"   ) <<  -90;
    QTest::newRow("1up; L; [90L]"  ) <<  -90;
    QTest::newRow("1up; L; [180L]" ) <<   90;
    QTest::newRow("1up; L; [270L]" ) <<   90;

    // :::::::::::::::::::::::::::::::::::::
    QTest::newRow("2up; P; [0P]"   ) <<  -90;
    QTest::newRow("2up; P; [90P]"  ) <<   90;
    QTest::newRow("2up; P; [180P]" ) <<   90;
    QTest::newRow("2up; P; [270P]" ) <<  -90;

    QTest::newRow("2up; P; [0L]"   ) <<    0;
    QTest::newRow("2up; P; [90L]"  ) <<    0;
    QTest::newRow("2up; P; [180L]" ) <<  180;
    QTest::newRow("2up; P; [270L]" ) <<  180;

    QTest::newRow("2up; L; [0P]"   ) <<  -90;
    QTest::newRow("2up; L; [90P]"  ) <<  -90;
    QTest::newRow("2up; L; [180P]" ) <<   90;
    QTest::newRow("2up; L; [270P]" ) <<   90;

    QTest::newRow("2up; L; [0L]"   ) << -180;
    QTest::newRow("2up; L; [90L]"  ) <<    0;
    QTest::newRow("2up; L; [180L]" ) <<    0;
    QTest::newRow("2up; L; [270L]" ) <<  180;

    // :::::::::::::::::::::::::::::::::::::
    QTest::newRow("4up_Horiz; P; [0P]"   ) <<    0;
    QTest::newRow("4up_Horiz; P; [90P]"  ) <<    0;
    QTest::newRow("4up_Horiz; P; [180P]" ) <<  180;
    QTest::newRow("4up_Horiz; P; [270P]" ) <<  180;

    QTest::newRow("4up_Horiz; P; [0L]"   ) <<  -90;
    QTest::newRow("4up_Horiz; P; [90L]"  ) <<   90;
    QTest::newRow("4up_Horiz; P; [180L]" ) <<   90;
    QTest::newRow("4up_Horiz; P; [270L]" ) <<  -90;

    QTest::newRow("4up_Horiz; L; [0P]"   ) << -180;
    QTest::newRow("4up_Horiz; L; [90P]"  ) <<    0;
    QTest::newRow("4up_Horiz; L; [180P]" ) <<    0;
    QTest::newRow("4up_Horiz; L; [270P]" ) <<  180;

    QTest::newRow("4up_Horiz; L; [0L]"   ) <<  -90;
    QTest::newRow("4up_Horiz; L; [90L]"  ) <<  -90;
    QTest::newRow("4up_Horiz; L; [180L]" ) <<   90;
    QTest::newRow("4up_Horiz; L; [270L]" ) <<   90;

    // :::::::::::::::::::::::::::::::::::::
    QTest::newRow("4up_Vert;  P; [0P]"   ) <<    0;
    QTest::newRow("4up_Vert;  P; [90P]"  ) <<    0;
    QTest::newRow("4up_Vert;  P; [180P]" ) <<  180;
    QTest::newRow("4up_Vert;  P; [270P]" ) <<  180;

    QTest::newRow("4up_Vert;  P; [0L]"   ) <<  -90;
    QTest::newRow("4up_Vert;  P; [90L]"  ) <<   90;
    QTest::newRow("4up_Vert;  P; [180L]" ) <<   90;
    QTest::newRow("4up_Vert;  P; [270L]" ) <<  -90;

    QTest::newRow("4up_Vert;  L; [0P]"   ) << -180;
    QTest::newRow("4up_Vert;  L; [90P]"  ) <<    0;
    QTest::newRow("4up_Vert;  L; [180P]" ) <<    0;
    QTest::newRow("4up_Vert;  L; [270P]" ) <<  180;

    QTest::newRow("4up_Vert;  L; [0L]"   ) <<  -90;
    QTest::newRow("4up_Vert;  L; [90L]"  ) <<  -90;
    QTest::newRow("4up_Vert;  L; [180L]" ) <<   90;
    QTest::newRow("4up_Vert;  L; [270L]" ) <<   90;

    // :::::::::::::::::::::::::::::::::::::
    QTest::newRow("8up_Horiz; P; [0P]"   ) <<  -90;
    QTest::newRow("8up_Horiz; P; [90P]"  ) <<   90;
    QTest::newRow("8up_Horiz; P; [180P]" ) <<   90;
    QTest::newRow("8up_Horiz; P; [270P]" ) <<  -90;

    QTest::newRow("8up_Horiz; P; [0L]"   ) <<    0;
    QTest::newRow("8up_Horiz; P; [90L]"  ) <<    0;
    QTest::newRow("8up_Horiz; P; [180L]" ) <<  180;
    QTest::newRow("8up_Horiz; P; [270L]" ) <<  180;

    QTest::newRow("8up_Horiz; L; [0P]"   ) <<  -90;
    QTest::newRow("8up_Horiz; L; [90P]"  ) <<  -90;
    QTest::newRow("8up_Horiz; L; [180P]" ) <<   90;
    QTest::newRow("8up_Horiz; L; [270P]" ) <<   90;

    QTest::newRow("8up_Horiz; L; [0L]"   ) << -180;
    QTest::newRow("8up_Horiz; L; [90L]"  ) <<    0;
    QTest::newRow("8up_Horiz; L; [180L]" ) <<    0;
    QTest::newRow("8up_Horiz; L; [270L]" ) <<  180;

    // :::::::::::::::::::::::::::::::::::::
    QTest::newRow("8up_Vert;  P; [0P]"   ) <<  -90;
    QTest::newRow("8up_Vert;  P; [90P]"  ) <<   90;
    QTest::newRow("8up_Vert;  P; [180P]" ) <<   90;
    QTest::newRow("8up_Vert;  P; [270P]" ) <<  -90;

    QTest::newRow("8up_Vert;  P; [0L]"   ) <<    0;
    QTest::newRow("8up_Vert;  P; [90L]"  ) <<    0;
    QTest::newRow("8up_Vert;  P; [180L]" ) <<  180;
    QTest::newRow("8up_Vert;  P; [270L]" ) <<  180;

    QTest::newRow("8up_Vert;  L; [0P]"   ) <<  -90;
    QTest::newRow("8up_Vert;  L; [90P]"  ) <<  -90;
    QTest::newRow("8up_Vert;  L; [180P]" ) <<   90;
    QTest::newRow("8up_Vert;  L; [270P]" ) <<   90;

    QTest::newRow("8up_Vert;  L; [0L]"   ) << -180;
    QTest::newRow("8up_Vert;  L; [90L]"  ) <<    0;
    QTest::newRow("8up_Vert;  L; [180L]" ) <<    0;
    QTest::newRow("8up_Vert;  L; [270L]" ) <<  180;

    // :::::::::::::::::::::::::::::::::::::
    QTest::newRow("booklet;   P; [0P]"   ) <<  -90;
    QTest::newRow("booklet;   P; [90P]"  ) <<   90;
    QTest::newRow("booklet;   P; [180P]" ) <<   90;
    QTest::newRow("booklet;   P; [270P]" ) <<  -90;

    QTest::newRow("booklet;   P; [0L]"   ) <<    0;
    QTest::newRow("booklet;   P; [90L]"  ) <<    0;
    QTest::newRow("booklet;   P; [180L]" ) <<  180;
    QTest::newRow("booklet;   P; [270L]" ) <<  180;

    QTest::newRow("booklet;   L; [0P]"   ) <<  -90;
    QTest::newRow("booklet;   L; [90P]"  ) <<  -90;
    QTest::newRow("booklet;   L; [180P]" ) <<   90;
    QTest::newRow("booklet;   L; [270P]" ) <<   90;

    QTest::newRow("booklet;   L; [0L]"   ) << -180;
    QTest::newRow("booklet;   L; [90L]"  ) <<    0;
    QTest::newRow("booklet;   L; [180L]" ) <<    0;
    QTest::newRow("booklet;   L; [270L]" ) <<  180;
}


/************************************************

 ************************************************/
void TestBoomaga::test_PagePosition()
{
    QFETCH(QString, expected);

    QString dataTag = QTest::currentDataTag();
    QString layoutName = dataTag.section(";", 0, 0);
    QString sheetRotationName = dataTag.section(";", 1, 1);
    QString directionName  = dataTag.section(";", 2, 2).trimmed();

    Rotation sheetRotation = StrToRotation(sheetRotationName);
    LayoutNUp *layout = createLayout(layoutName);
    Direction direction = directionName == "L>R" ? LeftToRight : RightToLeft;

    QStringList result;
    result <<     "+-" + QString("--").repeated(layout->mPageCountHoriz) + "+";
    for(int i=0; i< layout->mPageCountVert; ++i)
        result << "| " + QString(". ").repeated(layout->mPageCountHoriz) + "|";
    result <<     "+-" + QString("--").repeated(layout->mPageCountHoriz) + "+";


    for(int i=0; i<layout->mPageCountHoriz * layout->mPageCountVert; ++i)
    {
        Layout::PagePosition pos = layout->calcPagePosition(i, sheetRotation, direction);
        result[pos.row + 1].replace(pos.col * 2 + 2, 1, QString("%1").arg(i));
    }

    if (result.join("") != expected)
    {
        QString r=result.join("\n");
        QString msg = QString("Positions are not the same\n  Actual:\n%1\n   Expected:\n%2")
                    .arg(r.replace("+|", "+\n|").replace("||","|\n|").replace("|+", "|\n+"))
                    .arg(expected.replace("+|", "+\n|").replace("||","|\n|").replace("|+", "|\n+"));
        QFAIL(msg.toLocal8Bit());
    }
    delete layout;
}


/************************************************
 * Landscape is L pages xor pages rotated on 90 or 270
 ************************************************/
void TestBoomaga::test_PagePosition_data()
{
    QTest::addColumn<QString>("expected");

    // Left-To-Right ===========================================
    QTest::newRow("1up; 0; L>R"   ) // Layout; Rotation; Direction
            << "+---+"
               "| 0 |"
               "+---+";

    QTest::newRow("1up; 90; L>R"  ) // Layout; Rotation; Direction
            << "+---+"
               "| 0 |"
               "+---+";


    QTest::newRow("2up; 90; L>R"  ) // Layout; Rotation; Direction
            << "+---+"     // +------+
               "| 1 |"     // | 0  1 | Rotate
               "| 0 |"     // +------+
               "+---+";
            //  Expected      On screen


    QTest::newRow("2up; 0; L>R"   ) // Layout; Rotation; Direction
            << "+---+"     // +---+
               "| 0 |"     // | 0 | No rotate
               "| 1 |"     // | 1 |
               "+---+";    // +---+
            //  Expected      On screen


    QTest::newRow("4up_Horiz; 0; L>R") // Layout; Rotation; Direction
            << "+-----+"   // +-----+
               "| 0 1 |"   // | 0 1 | No rotate
               "| 2 3 |"   // | 2 3 |
               "+-----+";  // +-----+
            //  Expected      On screen


    QTest::newRow("4up_Horiz; 90; L>R") // Layout; Rotation; Direction
            << "+-----+"   // +-----+
               "| 1 3 |"   // | 0 1 | Rotate
               "| 0 2 |"   // | 2 3 |
               "+-----+";  // +-----+
            //  Expected      On screen


    QTest::newRow("4up_Vert;  0; L>R") // Layout; Rotation; Direction
            << "+-----+"   // +-----+
               "| 0 2 |"   // | 0 2 | No rotate
               "| 1 3 |"   // | 1 3 |
               "+-----+";  // +-----+
            //  Expected      On screen


    QTest::newRow("4up_Vert; 90; L>R") // Layout; Rotation; Direction
            << "+-----+"   // +-----+
               "| 2 3 |"   // | 0 2 | Rotate
               "| 0 1 |"   // | 1 3 |
               "+-----+";  // +-----+
            //  Expected      On screen


    QTest::newRow("8up_Horiz; 90; L>R") // Layout; Rotation; Direction
            << "+-----+"   // +---------+
               "| 3 7 |"   // | 0 1 2 3 | Rotate
               "| 2 6 |"   // | 4 5 6 7 |
               "| 1 5 |"   // +---------+
               "| 0 4 |"
               "+-----+";
            //  Expected      On screen

    QTest::newRow("8up_Horiz;  0; L>R") // Layout; Rotation; Direction
            << "+-----+"   // +-----+
               "| 0 1 |"   // | 0 1 | No rotate
               "| 2 3 |"   // | 2 3 |
               "| 4 5 |"   // | 4 5 |
               "| 6 7 |"   // | 6 7 |
               "+-----+";  // +-----+
            //  Expected      On screen


    QTest::newRow("8up_Vert;  90; L>R") // Layout; Rotation; Direction
            <<  "+-----+"   // +---------+
                "| 6 7 |"   // | 0 2 4 6 | Rotate
                "| 4 5 |"   // | 1 3 5 7 |
                "| 2 3 |"   // +---------+
                "| 0 1 |"
                "+-----+";
            //  Expected      On screen

    QTest::newRow("8up_Vert;   0; L>R")  // Layout; Rotation; Direction
            << "+-----+"   // +-----+
               "| 0 4 |"   // | 0 4 | No rotate
               "| 1 5 |"   // | 1 5 |
               "| 2 6 |"   // | 2 6 |
               "| 3 7 |"   // | 3 7 |
               "+-----+";  // +-----+
            //  Expected      On screen

    QTest::newRow("booklet; 90; L>R" )   // Layout; Rotation; Direction
            << "+---+"     // +------+
               "| 1 |"     // | 0  1 | Rotate
               "| 0 |"     // +------+
               "+---+";
            //  Expected      On screen


    QTest::newRow("booklet; 0; L>R"  )   // Layout; Rotation; Direction
            << "+---+"     // +---+
               "| 0 |"     // | 0 | No rotate
               "| 1 |"     // | 1 |
               "+---+";    // +---+
            //  Expected      On screen


    // Right-To-Left ===========================================
    QTest::newRow("1up; 0; R>L"   ) // Layout; Rotation; Direction
            << "+---+"
               "| 0 |"
               "+---+";

    QTest::newRow("1up; 90; R>L"  ) // Layout; Rotation; Direction
            << "+---+"
               "| 0 |"
               "+---+";


    QTest::newRow("2up; 90; R>L"  ) // Layout; Rotation; Direction
            << "+---+"     // +------+
               "| 0 |"     // | 1  0 | Rotate
               "| 1 |"     // +------+
               "+---+";
            //  Expected      On screen


    QTest::newRow("2up; 0; R>L"   ) // Layout; Rotation; Direction
            << "+---+"     // +---+
               "| 0 |"     // | 0 | No rotate
               "| 1 |"     // | 1 |
               "+---+";    // +---+
            //  Expected      On screen


    QTest::newRow("4up_Horiz; 0; R>L") // Layout; Rotation; Direction
            << "+-----+"   // +-----+
               "| 1 0 |"   // | 1 0 | No rotate
               "| 3 2 |"   // | 3 2 |
               "+-----+";  // +-----+
            //  Expected      On screen


    QTest::newRow("4up_Horiz; 90; R>L") // Layout; Rotation; Direction
            << "+-----+"   // +-----+
               "| 0 2 |"   // | 1 0 | Rotate
               "| 1 3 |"   // | 3 2 |
               "+-----+";  // +-----+
            //  Expected      On screen


    QTest::newRow("4up_Vert;  0; R>L") // Layout; Rotation; Direction
            << "+-----+"   // +-----+
               "| 2 0 |"   // | 2 0 | No rotate
               "| 3 1 |"   // | 3 1 |
               "+-----+";  // +-----+
            //  Expected      On screen


    QTest::newRow("4up_Vert; 90; R>L") // Layout; Rotation; Direction
            << "+-----+"   // +-----+
               "| 0 1 |"   // | 2 0 | Rotate
               "| 2 3 |"   // | 3 1 |
               "+-----+";  // +-----+
            //  Expected      On screen


    QTest::newRow("8up_Horiz; 90; R>L") // Layout; Rotation; Direction
            << "+-----+"   // +---------+
               "| 0 4 |"   // | 3 2 1 0 | Rotate
               "| 1 5 |"   // | 7 6 5 4 |
               "| 2 6 |"   // +---------+
               "| 3 7 |"
               "+-----+";
            //  Expected      On screen

    QTest::newRow("8up_Horiz;  0; R>L") // Layout; Rotation; Direction
            << "+-----+"   // +-----+
               "| 1 0 |"   // | 1 0 | No rotate
               "| 3 2 |"   // | 3 2 |
               "| 5 4 |"   // | 5 4 |
               "| 7 6 |"   // | 7 6 |
               "+-----+";  // +-----+
            //  Expected      On screen


    QTest::newRow("8up_Vert;  90; R>L") // Layout; Rotation; Direction
            <<  "+-----+"   // +---------+
                "| 0 1 |"   // | 6 4 2 0 | Rotate
                "| 2 3 |"   // | 7 5 3 1 |
                "| 4 5 |"   // +---------+
                "| 6 7 |"
                "+-----+";
            //  Expected      On screen

    QTest::newRow("8up_Vert;   0; R>L")  // Layout; Rotation; Direction
            << "+-----+"   // +-----+
               "| 4 0 |"   // | 4 0 | No rotate
               "| 5 1 |"   // | 5 1 |
               "| 6 2 |"   // | 6 2 |
               "| 7 3 |"   // | 7 3 |
               "+-----+";  // +-----+
            //  Expected      On screen

    QTest::newRow("booklet; 90; R>L" )   // Layout; Rotation; Direction
            << "+---+"     // +------+
               "| 0 |"     // | 1  0 | Rotate
               "| 1 |"     // +------+
               "+---+";
            //  Expected      On screen


    QTest::newRow("booklet; 0; R>L"  )   // Layout; Rotation; Direction
            << "+---+"     // +---+
               "| 0 |"     // | 0 | No rotate
               "| 1 |"     // | 1 |
               "+---+";    // +---+
            //  Expected      On screen

}


/************************************************
 *
 * ***********************************************/
void TestBoomaga::test_BooFilePageSpec()
{
    QString string = QTest::currentDataTag();
    QFETCH(QString, direction);
    QFETCH(int, pageNum);
    QFETCH(bool, hidden);
    QFETCH(int, rotation);
    QFETCH(bool, startBooklet);

    if (direction == ">")
    {
        BooFile::PageSpec spec(string);

        QCOMPARE(spec.pageNum, pageNum);
        QCOMPARE(spec.hidden, hidden);
        QCOMPARE((int)spec.rotation, rotation);
    }
    else
    {
        BooFile::PageSpec spec(pageNum,
                                   hidden,
                                   (Rotation)rotation,
                                   startBooklet);
        QString result = spec.toString();
        QCOMPARE(result, string);
    }
}


/************************************************
 *
 * ***********************************************/
void TestBoomaga::test_BooFilePageSpec_data()
{
    QTest::addColumn<QString>("direction");
    QTest::addColumn<int>("pageNum");
    QTest::addColumn<bool>("hidden");
    QTest::addColumn<int>("rotation");
    QTest::addColumn<bool>("startBooklet");

    QTest::newRow("1")          << ">" <<  0 << false <<   0  << false;
    QTest::newRow("17::")       << ">" << 16 << false <<   0  << false;
    QTest::newRow("B::")        << ">" << -1 << false <<   0  << false;
    QTest::newRow("17:H:")      << ">" << 16 << true  <<   0  << false;
    QTest::newRow("2::0")       << ">" <<  1 << false <<   0  << false;
    QTest::newRow("2::90")      << ">" <<  1 << false <<  90  << false;
    QTest::newRow("2::180")     << ">" <<  1 << false << 180  << false;
    QTest::newRow("2::270")     << ">" <<  1 << false << 270  << false;
    QTest::newRow("2:H:0")      << ">" <<  1 << true  <<   0  << false;
    QTest::newRow("2:H:90")     << ">" <<  1 << true  <<  90  << false;
    QTest::newRow("2:H:180")    << ">" <<  1 << true  << 180  << false;
    QTest::newRow("2:H:270")    << ">" <<  1 << true  << 270  << false;

    QTest::newRow("1")          << "<" <<  0 << false <<   0  << false;
    QTest::newRow("1::90")      << "<" <<  0 << false <<  90  << false;
    QTest::newRow("1::180")     << "<" <<  0 << false << 180  << false;
    QTest::newRow("1::270")     << "<" <<  0 << false << 270  << false;

    QTest::newRow("3:H")        << "<" <<  2 << true  <<   0  << false;
    QTest::newRow("3:H:90")     << "<" <<  2 << true  <<  90  << false;
    QTest::newRow("3:H:180")    << "<" <<  2 << true  << 180  << false;
    QTest::newRow("3:H:270")    << "<" <<  2 << true  << 270  << false;

    QTest::newRow("B")          << "<" << -1 << false <<   0  << false;
    QTest::newRow("B::90")      << "<" << -1 << false <<  90  << false;
    QTest::newRow("B::180")     << "<" << -1 << false << 180  << false;
    QTest::newRow("B::270")     << "<" << -1 << false << 270  << false;

    QTest::newRow("B:H")        << "<" << -1 << true  <<   0  << false;
    QTest::newRow("B:H:90")     << "<" << -1 << true  <<  90  << false;
    QTest::newRow("B:H:180")    << "<" << -1 << true  << 180  << false;
    QTest::newRow("B:H:270")    << "<" << -1 << true  << 270  << false;

    QTest::newRow("1:::S")      << ">" <<  0 << false <<   0  << true;
    QTest::newRow("2:H:90:S")   << ">" <<  1 << true  <<  90  << true;

    QTest::newRow("1:::S")      << "<" <<  0 << false <<   0  << true;
    QTest::newRow("2:H::S")     << "<" <<  1 << true  <<   0  << true;
    QTest::newRow("2:H:90:S")   << "<" <<  1 << true  <<  90  << true;

}


/************************************************
 *
 * ***********************************************/
void TestBoomaga::test_BooklesSplit()
{
    settings->setValue(Settings::SubBookletsEnabled, true);
    settings->setValue(Settings::SubBookletSize, 2);

    QStringList tags = QString(QTest::currentDataTag()).split(":", QString::SkipEmptyParts);
    QString pageSpec = tags.count() > 2 ? tags.at(2): "";

    QList<ProjectPage*> pages;
    foreach (QString spec, pageSpec.split(" ", QString::SkipEmptyParts))
    {
        ProjectPage *page = new ProjectPage();
        page->setManualStartSubBooklet(spec.contains("M"));
        //qDebug() << spec << spec.contains("M") << ;
        pages << page;
    }

    LayoutNUp *layout = createLayout(tags.at(0));
    layout->updatePages(pages);

    QFETCH(QString, expected);
    expected = expected.simplified();
    QString result;

    for(int i=0; i< pages.count(); ++i)
    {
        if (pages.at(i)->isStartSubBooklet())
            result += QString(" %1").arg(i+1);
    }
    result = result.simplified();

    QCOMPARE(result, expected);


    qDeleteAll(pages);
    delete layout;
}

/************************************************
 *
 * ***********************************************/
void TestBoomaga::test_BooklesSplit_data()
{
    QTest::addColumn<QString>("expected");
    //QTest::newRow("booklet: 4 pps:") << "";
    QTest::newRow("booklet: 4 pps: 1") << "1";

    QTest::newRow("booklet: 4 pps: 1 2 3 4 5 6 7") << "1";
    QTest::newRow("booklet: 4 pps: 1 2 3 4 5 6 7 8") << "1";
    QTest::newRow("booklet: 4 pps: 1 2 3 4 5 6 7 8   9 10") << "1 9";
    QTest::newRow("booklet: 4 pps: 1 2 3 4 5 6 7 8   9 10 11 12 13 14 15 16") << "1 9";
    QTest::newRow("booklet: 4 pps: 1 2 3 4 5 6 7 8   9 10 11 12 13 14 15 16  17") << "1 9 17";

    QTest::newRow("booklet: 4 pps: 1 2M 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18") << "1 2 10 18";
    QTest::newRow("booklet: 4 pps: 1 2M 3 4 5 6M 7 8 9 10 11 12 13 14 15 16 17 18") << "1 2 6 14";
    QTest::newRow("booklet: 4 pps: 1 2M 3 4 5 6 7 8 9 10M 11 12 13 14 15 16 17 18") << "1 2 10 18";
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testEscapeString()
{
    QFETCH(QString, in);
    QString escaped  = QString::fromStdString(escapeString(in.toStdString()));
    QString expected = QUrl::fromPercentEncoding(escaped.toLocal8Bit());
    QCOMPARE(in, expected);
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testEscapeString_data()
{
    QTest::addColumn<QString>("in");
    QTest::newRow("1") << "";
    QTest::newRow("2") << "1234567890";
    QTest::newRow("3") << "Multi\nline";
    QTest::newRow("3") << " '\"\\\" {}()\t\r\b\x0 ";
    QTest::newRow("4") << "qwertyuiopasdfghjklzxcvbnm";
    QTest::newRow("5") << "QWERTYUIOPASDFGHJKLZXCVBNM";
    QTest::newRow("6") << "`-=;'\\,./";
    QTest::newRow("7") << "йцукенгшщзхъфывапролджэячсмитьбюё";
    QTest::newRow("8") << "ЙЦУКЕНГШЩЗХЪФЫВАПРОЛДЖЭЯЧСМИТЬБЮЁ";
}


/************************************************
 *
 ************************************************/
static QString safePath(const QString &path)
{
    QString res = path;
    res = res.replace(' ', '_');
    res = res.replace('\t', '_');
    res = res.replace('\n', '_');
    res = res.replace('/', '_');
    return res;
}


/************************************************
 *
 ************************************************/
QString TestBoomaga::dir(const QString &subTest)
{
    QString test    = QString::fromLocal8Bit(QTest::currentTestFunction());
    QString subtest = subTest.isEmpty() ? QString::fromLocal8Bit(QTest::currentDataTag()) : subTest;


    return QDir::cleanPath(QString("%1/%2/%3")
                    .arg(TEST_OUT_DIR)
                    .arg(safePath(test))
                    .arg(safePath(subtest)));
}


QTEST_GUILESS_MAIN(TestBoomaga)
