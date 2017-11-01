/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2017 Boomaga team https://github.com/Boomaga
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


#define protected public
#include "testboomaga.h"

#include <QTest>
#include "../pdfparser/pdfwriter.h"

uint sPrintUint(char *s,   quint64 value);
uint sPrintInt(char *s,    qint64 value);
uint sPrintDouble(char *s, double value);

/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfWriter_SprintUint()
{
    QFETCH(int,     value);
    QFETCH(QString, expected);
    QFETCH(int,     expectedLen);

    char buf[128];
    int len = sPrintUint(buf, value);
    QCOMPARE(buf, expected.toLatin1().constData());
    QCOMPARE(len, expectedLen);
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfWriter_SprintUint_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<int>("expectedLen");

    QTest::newRow("01") <<  0    << "0"     << 1;
    QTest::newRow("02") <<  1    << "1"     << 1;
    QTest::newRow("03") <<  42   << "42"    << 2;
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfWriter_SprintInt()
{
    QFETCH(int,     value);
    QFETCH(QString, expected);
    QFETCH(int,     expectedLen);

    char buf[128];
    int len = sPrintInt(buf, value);
    QCOMPARE(buf, expected.toLatin1().constData());
    QCOMPARE(len, expectedLen);
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfWriter_SprintInt_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<int>("expectedLen");

    QTest::newRow("01") <<  0    << "0"     << 1;
    QTest::newRow("02") <<  1    << "1"     << 1;
    QTest::newRow("03") <<  42   << "42"    << 2;
    QTest::newRow("04") << -42   << "-42"   << 3;
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfWriter_SprintDouble()
{
    QFETCH(double,  value);
    QFETCH(QString, expected);
    QFETCH(int,     expectedLen);

    char buf[21] = "                    ";
    int len = sPrintDouble(buf, value);
    QCOMPARE(buf, expected.toLatin1().constData());
    QCOMPARE(len, expectedLen);
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfWriter_SprintDouble_data()
{
    QTest::addColumn<double>("value");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<int>("expectedLen");

    QTest::newRow("01") <<  0.0       << "0"          << 1;
    QTest::newRow("02") <<  1.0       << "1"          << 1;
    QTest::newRow("03") <<  42.0      << "42"         << 2;
    QTest::newRow("04") << -42.0      << "-42"        << 3;
    QTest::newRow("05") <<   0.1      << "0.1"        << 3;
    QTest::newRow("06") <<   0.0001   << "0.0001"     << 6;
    QTest::newRow("07") <<   1.0001   << "1.0001"     << 6;
    QTest::newRow("08") << 999.0001   << "999.0001"   << 8;
    QTest::newRow("07") <<  -1.0001   << "-1.0001"    << 7;
    QTest::newRow("07") << 123.456789 << "123.456789" << 10;

}
