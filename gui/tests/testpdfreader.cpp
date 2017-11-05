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
void TestBoomaga::testPdfReader_ReadStringHex()
{
    QFETCH(int,     pos);
    QFETCH(int,     expectedPos);
    QFETCH(QString, data);
    QFETCH(QString, expected);

    TestReader reader(data);

    PDF::Value v;
    qint64 newPos = pos;
    try
    {
        v = reader.readValue(&newPos);

        QCOMPARE(v.type(),  PDF::Value::Type::String);
        QCOMPARE(v.asString().value(), expected);
        QCOMPARE(v.asString().encodingType(), PDF::String::HexEncoded);
        if (expectedPos)
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
void TestBoomaga::testPdfReader_ReadStringHex_data()
{
    QTest::addColumn<int>("pos");
    QTest::addColumn<int>("expectedPos");
    QTest::addColumn<QString>("data");
    QTest::addColumn<QString>("expected");


    QTest::newRow("01") << 0 << 2
            << "<>"
            << "";

    QTest::newRow("02") << 0 << 14
            << "<54 45 53 54 >"
            << "TEST";

    QTest::newRow("03") << 0 << 19
            << "< 5 4 4 5 5 3 5 4 >"
            << "TEST";

    QTest::newRow("04") << 0 << 18
            << "<5420450A53095420>"
            << "T E\nS\tT ";

    QTest::newRow("05") << 0 << 19
            << "<54 \n 45 \n 53 \n 54>"
            << "TEST";

    QTest::newRow("06") << 3 << 17
            << "PDF<537472696E67>Next"
            << "String";

    QTest::newRow("07") << 3 << 24
            << "PDF< 53 74 72 69 6E 67 >Next"
            << "String";


    QTest::newRow("08 UTF-8") << 0 << 43
            << "<EFBBBF D0A0 D183 D181 D181 D0BA D0B8 D0B9>"
            << "Русский";

    QTest::newRow("08 UTF-8") << 0 << 43
            << "<efbbbf d0a0 d183 d181 d181 d0ba d0b8 d0b9>"
            << "Русский";

    QTest::newRow("08 Default") << 0 << 36
            << "<D0A0 D183 D181 D181 D0BA D0B8 D0B9>"
            << "Русский";

    QTest::newRow("08 UTF-16-BE") << 0 << 43
            << "<FEFF   0420 0443 0441 0441 043A 0438 0439>"
            << "Русский";

    QTest::newRow("08 UTF-16-LE") << 0 << 43
            << "<FFFE   2004 4304 4104 4104 3A04 3804 3904>"
            << "Русский";

    QTest::newRow("08 UTF-32-BE") << 0 << 73
            << "<0000FEFF 00000420 00000443 00000441 00000441 0000043A 00000438 00000439>"
            << "Русский";

    QTest::newRow("08 UTF-32-LE") << 0 << 73
            << "<FFFE0000 20040000 43040000 41040000 41040000 3A040000 38040000 39040000>"
            << "Русский";

}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfReader_ReadStringLiteral()
{
    QFETCH(int,     pos);
    QFETCH(int,     expectedPos);
    QFETCH(QString, data);
    QFETCH(QString, expected);

    TestReader reader(data);

    PDF::Value v;
    qint64 newPos = pos;
    try
    {
        v = reader.readValue(&newPos);

        QCOMPARE(v.type(),  PDF::Value::Type::String);
        QCOMPARE(v.asString().value(), expected);
        QCOMPARE(v.asString().encodingType(), PDF::String::LiteralEncoded);
        if (expectedPos)
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
void TestBoomaga::testPdfReader_ReadStringLiteral_data()
{
    QTest::addColumn<int>("pos");
    QTest::addColumn<int>("expectedPos");
    QTest::addColumn<QString>("data");
    QTest::addColumn<QString>("expected");


    QTest::newRow("Literal 01") << 0 << 2
            << "()" << "";

    QTest::newRow("Literal 02") << 0 << 6
            << "(Test)" << "Test";

    QTest::newRow("Literal 03") << 0 << 0
            << "(Strings may contain balanced parentheses ( ) and special characters ( * ! & } ^ % and so on)"
            << "Strings may contain balanced parentheses ( ) and special characters ( * ! & } ^ % and so on";

    QTest::newRow("Literal 04") << 0 << 0
            << "(\\n\\r\\t\\b\\f\\(\\)\\\\)"
            << "\n\r\t\b\f()\\";

    QTest::newRow("Literal 05") << 0 << 0
            << "(These \ntwo strings \nare the same.)"
            << "These \ntwo strings \nare the same.";

    QTest::newRow("Literal 06") << 0 << 0
            << "(These \\ \ntwo strings \\ \nare the same.)"
            << "These  \ntwo strings  \nare the same.";

    QTest::newRow("Literal 07") << 0 << 0
            << "(These \\\ntwo strings \\\nare the same.)"
            << "These two strings are the same.";

    QTest::newRow("Literal 08") << 0 << 0
            << "(These \\\rtwo strings \\\rare the same.)"
            << "These two strings are the same.";

    QTest::newRow("Literal 09") << 0 << 0
            << "(These \\\r\ntwo strings \\\r\nare the same.)"
            << "These two strings are the same.";

    QTest::newRow("Literal 10") << 0 << 0
            << "(These \\\n\rtwo strings \\\n\rare the same.)"
            << "These two strings are the same.";

    QTest::newRow("Literal 11") << 0 << 0
            << "(This string has an end-of-line at the end of it.\n)"
            << "This string has an end-of-line at the end of it.\n";

    QTest::newRow("Literal 12") << 0 << 0
            << "(\\053)"    << "+";

    QTest::newRow("Literal 13") << 0 << 0
            << "\\53"       << "+";

    QTest::newRow("Literal 14") << 0 << 0
            << "\\0533"     << "+3";

    QTest::newRow("Literal 15") << 0 << 0
            << "\\0073"     << "\a3";

    QTest::newRow("Literal 16") << 0 << 0
            << "\\7"        << "\a";

    QTest::newRow("Literal 17") << 0 << 0
            << "\\07"       << "\a";

    QTest::newRow("Literal 18") << 0 << 0
            << "\\007"      << "\a";

    QTest::newRow("Literal 19") << 0 << 0
            << "A\\7B"      << "A\aB";

    QTest::newRow("Literal 20") << 0 << 0
            << "A\\07B"     << "A\aB";

    QTest::newRow("Literal 21") << 0 << 0
            << "A\\007B"    << "A\aB";

    QTest::newRow("Literal 22") << 0 << 0
            << "\\245"      << "\245";

    QTest::newRow("Literal 23") << 0 << 0
            << "\\8"        << "8";

    QTest::newRow("Literal 24") << 0 << 0
            << "\\9"        << "9";

    QTest::newRow("Literal 25") << 0 << 0
            << "a\\aa"      << "aaa";

    QTest::newRow("Literal 26") << 0 << 0
            << "a\\za"      << "aza";

    QTest::newRow("Literal 27") << 0 << 0
            << "((These (two) ((strings) are) \\(the \\))same.)Not string)"
            << "(These (two) ((strings) are) (the ))same.";

    QTest::newRow("Literal 28") << 0 << 0
            << "(These \\(two (\\(strings are) \\(the \\)same.)Not string"
            << "These (two ((strings are) (the )same.";
}
