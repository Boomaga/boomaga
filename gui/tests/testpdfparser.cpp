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
#include "../pdfparser/pdfreader.h"
#include "../pdfparser/pdfdata.h"
#include "tools.h"
#include <QDebug>

using namespace PdfParser;


/************************************************
 *
 ************************************************/
void TestBoomaga::testCanReadName()
{
    QFETCH(QString, data);
    QFETCH(int,     pos);
    QFETCH(bool,    expected);

    Reader reader(data.toLocal8Bit().data(), data.toLocal8Bit().length());
    bool res = reader.canReadName(pos);

    QCOMPARE(res, expected);
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testCanReadName_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<int>("pos");
    QTest::addColumn<bool>("expected");

    QTest::newRow("01") << "/Name" << 0 << true;
    QTest::newRow("02") << "/Name" << 1 << false;
    QTest::newRow("03") << "/Name /Other" << 5 << false;
    QTest::newRow("04") << "/Name /Other" << 6 << true;
    QTest::newRow("05") << "/Name /Other" << 600 << false;
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testReadName()
{
    QFETCH(QString, data);
    QFETCH(int,     pos);
    QFETCH(QString, expected);
    QFETCH(int,     expectedPos);

    try
    {
        Reader reader(data.toLocal8Bit().data(), data.toLocal8Bit().length());
        Name name;
        int newPos = reader.readName(pos, &name);

        QCOMPARE(name.value(), expected);
        QCOMPARE(newPos, expectedPos);

    }
    catch (PdfParser::Error& e)
    {
        if (expectedPos > 0)
            FAIL_EXCEPTION(e);
    }

}


/************************************************
 *
 ************************************************/
void TestBoomaga::testReadName_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<int>("pos");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<int>("expectedPos");

    QTest::newRow("01") << "/Name"      << 0 << ""      << -1;
    QTest::newRow("02") << "/Name "     << 0 << "Name"  << 5;
    QTest::newRow("03") << "/Name/Val"  << 0 << "Name"  << 5;
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testReadLink()
{
    QFETCH(QString, data);
    QFETCH(int,     pos);
    QFETCH(int,     expectedObjNum);
    QFETCH(int,     expectedGenNum);
    QFETCH(int,     expectedPos);

    try
    {
        Reader reader(data.toLocal8Bit().data(), data.toLocal8Bit().length());
        Link link;
        int newPos = reader.readLink(pos, &link);

        QCOMPARE(link.objNum(), expectedObjNum);
        QCOMPARE(link.genNum(), expectedGenNum);
        QCOMPARE(newPos, expectedPos);

    }
    catch (PdfParser::Error& e)
    {
        if (expectedPos > 0)
            FAIL_EXCEPTION(e);
    }
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testReadLink_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<int>("pos");
    QTest::addColumn<int>("expectedObjNum");
    QTest::addColumn<int>("expectedGenNum");
    QTest::addColumn<int>("expectedPos");

    //                      data             pos   objNum  genNum  pos
    QTest::newRow("01") << "0 0 R"          << 0    << 0    << 0  << 5;
    QTest::newRow("02") << "/Name 1 2 R"    << 6    << 1    << 2  << 11;
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testReadNum()
{
    QString data = QTest::currentDataTag();
    QFETCH(double,  expected);
    QFETCH(int,  expectedPos);

    Data pdfData(data.toLocal8Bit().data(), data.toLocal8Bit().length());
    bool ok;
    qint64 pos = 0;
    double res;

    res = pdfData.readNum(&pos, &ok);
    QCOMPARE(res, expected);
    int newPos = pos;
    QCOMPARE(newPos, expectedPos);
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testReadNum_data()
{
    QTest::addColumn<double>("expected");
    QTest::addColumn<int>("expectedPos");

    //           data       expected       pos
    QTest::newRow("")       <<  0.0     <<  0;
    QTest::newRow("1")      <<  1.0     <<  1;
    QTest::newRow("0.2")    <<  0.2     <<  3;
    QTest::newRow("-0.2")   << -0.2     <<  4;
    QTest::newRow(".42")    <<  0.42    <<  3;
    QTest::newRow("123")    <<  123.0   <<  3;
    QTest::newRow("43445")  <<  43445.0 <<  5;
    QTest::newRow("+17")    << +17.0    <<  3;
    QTest::newRow("-98")    << -98.0    <<  3;
    QTest::newRow("0")      <<  0.0     <<  1;

    QTest::newRow("34.5")   <<  34.5    <<  4;
    QTest::newRow("-3.62")  << -3.62    <<  5;
    QTest::newRow("+123.6") <<  123.6   <<  6;

    QTest::newRow("4.")     <<  4.0     <<  2;
    QTest::newRow("-.002")  << -.002    <<  5;
    QTest::newRow("0.0")    <<  0.0     <<  3;
    QTest::newRow("1.0004") <<  1.0004  <<  6;
    QTest::newRow("42. 15") <<  42.0    <<  3;
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testSkipDict()
{
    QFETCH(QString, data);
    QFETCH(int,     expectedPos);

    Data pdfData(data.toLocal8Bit().data(), data.toLocal8Bit().length());
    int newPos = pdfData.skipDictBack(data.toLocal8Bit().length());
    QCOMPARE(newPos, expectedPos);
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testSkipDict_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<int>("expectedPos");

    QTest::newRow("01") << "" << -1;
    QTest::newRow("02") << "<</Root 1 0 R /Size 8>>" <<  -1;
    QTest::newRow("03") << " <</Root 1 0 R /Size 8>>" <<  0;
    QTest::newRow("04") << "SOME <</Root 1 0 R /Size 8>>" << 4;
    QTest::newRow("05") << " <</Root 1 0 R /Dict1 << /Name val>> /Size 8>>" <<  0;
    QTest::newRow("06") << " <</Root 1 0 R /Dict1 << /Name val /Dict2<</N v>>>> /Size 8>>" <<  0;
    QTest::newRow("07") << " <</Root 1 0 R /Dict1 << /Name val /Dict2<<>>>> /Size 8>>" <<  0;
}
