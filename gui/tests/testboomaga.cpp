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

#include "../boomagatypes.h"

#define COMPARE(actual, expected) \
    do {\
        if (!QTest::qCompare(QString("%1").arg(actual), QString("%1").arg(expected), #actual, #expected, __FILE__, __LINE__))\
            return;\
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
//void TestBoomaga::test_Rotation_data()
//{
//    QTest::addColumn<QString>("cueFile");
//    QTest::addColumn<QString>("pattern");
//    QTest::addColumn<QString>("expected");
//}


QTEST_MAIN(TestBoomaga)
