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
#include "tools.h"


class TestReader: public PDF::Reader
{
public:
    TestReader(const QString data):
        PDF::Reader()
    {
        mByteArray.fill(' ', 100);
        mByteArray.insert(0, "%PDF-1.4\n");

        int obj1Pos = mByteArray.size();
        mByteArray.append("1 0 obj <</Type /Catalog /Pages 2 0 R>> endobj\n");

        int obj2Pos = mByteArray.size();
        mByteArray.append("2 0 obj <</Type /Pages /Kids [ ] /Count 0>>endobj\n");

        int xrefPos = mByteArray.size();
        mByteArray.append("xref\n");
        mByteArray.append("0 3\n");
        mByteArray.append("0000000000 65535 f \n");
        mByteArray.append(QString("%1 00000 n \n").arg(obj1Pos, 10, 10, QChar('0')));
        mByteArray.append(QString("%1 00000 n \n").arg(obj2Pos, 10, 10, QChar('0')));

        mByteArray.append("trailer\n<</Size 0>>\n");
        mByteArray.append(QString("startxref\n%1\n%%EOF\n").arg(xrefPos));

        open(mByteArray.constData(), mByteArray.length());
        mByteArray.replace(0, data.toLocal8Bit().length(), data.toLocal8Bit());
        mByteArray.replace(data.toLocal8Bit().length(), 5, "     ");
    }

private:
   QByteArray mByteArray;
};


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfReader_ReadName()
{
    QFETCH(QString, data);
    QFETCH(int,     pos);
    QFETCH(QString, expected);
    QFETCH(int,     expectedPos);

    try
    {

    TestReader reader(data);

    PDF::Value v;
    qint64 newPos = pos;
        v = reader.readValue(&newPos);

        QCOMPARE(newPos, expectedPos);
        QCOMPARE(v.isName(), true);
        QCOMPARE(v.asName().value(), expected);
    }
    catch (PDF::Error& e)
    {
        if (expectedPos > 0)
            FAIL_EXCEPTION(e);
    }
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfReader_ReadName_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<int>("pos");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<int>("expectedPos");

    QTest::newRow("01") << "/Name "     << 0 << "Name"  << 5;
    QTest::newRow("02") << "/Name/Val"  << 0 << "Name"  << 5;
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfReader_ReadLink()
{
    QFETCH(QString, data);
    QFETCH(int,     pos);
    QFETCH(int,     expectedObjNum);
    QFETCH(int,     expectedGenNum);
    QFETCH(int,     expectedPos);

    TestReader reader(data);

    PDF::Value v;
    qint64 newPos = pos;
    try
    {
        v = reader.readValue(&newPos);

        QCOMPARE(newPos, expectedPos);
        QCOMPARE(v.isLink(), true);
        QCOMPARE(v.asLink().objNum(), quint32(expectedObjNum));
        QCOMPARE(v.asLink().genNum(), quint16(expectedGenNum));
    }
    catch (PDF::Error& e)
    {
        if (expectedPos > 0)
            FAIL_EXCEPTION(e);
    }
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfReader_ReadLink_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<int>("pos");
    QTest::addColumn<int>("expectedObjNum");
    QTest::addColumn<int>("expectedGenNum");
    QTest::addColumn<int>("expectedPos");

    //                      data                    pos   objNum  genNum  newPos
    QTest::newRow("01") << "0 0 R"                  << 0    << 0    << 0  << 5;
    QTest::newRow("02") << "/Name 1 2 R"            << 6    << 1    << 2  << 11;
    QTest::newRow("03") << "/Name 1 2 R%Comment"    << 6    << 1    << 2  << 11;
    QTest::newRow("03") << "%Comment\n1 2 R"        << 0    << 1    << 2  << 14;
    QTest::newRow("03") << "%Comment\r1 2 R"        << 0    << 1    << 2  << 14;
    QTest::newRow("03") << "%Comment\r\n1 2 R"      << 0    << 1    << 2  << 15;
    QTest::newRow("03") << "%Comment\n\r1 2 R"      << 0    << 1    << 2  << 15;
    QTest::newRow("03") << "%Comment\n 1 2 R"       << 0    << 1    << 2  << 15;
    QTest::newRow("03") << "%Comment\n\t1 2 R"      << 0    << 1    << 2  << 15;
    QTest::newRow("03") << "%Comment\n\n1 2 R"      << 0    << 1    << 2  << 15;
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfReader_ReadNum()
{
    QString data = QTest::currentDataTag();
    QFETCH(double,  expected);
    QFETCH(int,  expectedPos);

    TestReader reader(data);
    PDF::Value v;
    qint64 newPos = 0;
    try
    {
        v = reader.readValue(&newPos);
    }
    catch (PDF::Error& e)
    {
        if (expectedPos > 0)
            FAIL_EXCEPTION(e);
    }

    QCOMPARE(newPos, expectedPos);

    QCOMPARE(v.isNumber(), true);

    double res = v.asNumber().value();
    QCOMPARE(res, expected);
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfReader_ReadNum_data()
{
    QTest::addColumn<double>("expected");
    QTest::addColumn<int>("expectedPos");

    //           data       expected       pos
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

    QTest::newRow("4.")      <<  4.0     <<  2;
    QTest::newRow("-.002")   << -.002    <<  5;
    QTest::newRow("0.0")     <<  0.0     <<  3;
    QTest::newRow("1.0004")  <<  1.0004  <<  6;
    QTest::newRow("42. 15")  <<  42.0    <<  3;
    QTest::newRow("42 15")   <<  42.0    <<  2;
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfReader_ReadString()
{
    QFETCH(QString, data);
    QFETCH(int,     pos);
    QFETCH(QString, expected);
    QFETCH(int,     expectedPos);

    TestReader reader(data);

    PDF::Value v;
    qint64 newPos = pos;
    try
    {
        v = reader.readValue(&newPos);

        QCOMPARE(v.type(),  PDF::Value::Type::HexString);
        QCOMPARE(v.asHexString().toString(), expected);
        QCOMPARE(newPos, expectedPos);
    }
    catch (PDF::Error& e)
    {
        if (expectedPos > 0)
            FAIL_EXCEPTION(e);
    }
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfReader_ReadString_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<int>("pos");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<int>("expectedPos");

    QTest::newRow("01")
            << "<>"             << 0
            << ""               << 2;

    QTest::newRow("02 UTF-8")
            << "<EFBBBF D0A0 D183 D181 D181 D0BA D0B8 D0B9>" << 0
            << "Русский"               << 43;

    QTest::newRow("02 Default")
            << "<D0A0 D183 D181 D181 D0BA D0B8 D0B9>" << 0
            << "Русский"               << 36;

    QTest::newRow("03 UTF-16-BE")
            << "<FEFF   0420 0443 0441 0441 043A 0438 0439>" << 0
            << "Русский"               << 43;

    QTest::newRow("03 UTF-16-LE")
            << "<FFFE   2004 4304 4104 4104 3A04 3804 3904>" << 0
            << "Русский"               << 43;

    QTest::newRow("04 UTF-32-BE")
            << "<0000FEFF 00000420 00000443 00000441 00000441 0000043A 00000438 00000439>" << 0
            << "Русский"               << 73;

    QTest::newRow("04 UTF-32-LE")
            << "<FFFE0000 20040000 43040000 41040000 41040000 3A040000 38040000 39040000>" << 0
            << "Русский"               << 73;

}
