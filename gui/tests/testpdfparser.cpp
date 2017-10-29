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
#include <QDebug>

using namespace PDF;


/************************************************
 *
 ************************************************/
void TestBoomaga::testReadName()
{
    QFETCH(QString, data);
    QFETCH(int,     pos);
    QFETCH(QString, expected);
    QFETCH(int,     expectedPos);


    const QByteArray byteArray = data.toLocal8Bit();
    Reader reader(byteArray.data(), byteArray.length());

    PDF::Value v;
    qint64 newPos = pos;
    try
    {
        v = reader.readValue(&newPos);

        QCOMPARE(newPos, expectedPos);
        QCOMPARE(v.isName(), true);
        QCOMPARE(v.toName().value(), expected);
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

    const QByteArray byteArray = data.toLocal8Bit();
    Reader reader(byteArray.data(), byteArray.length());

    PDF::Value v;
    qint64 newPos = pos;
    try
    {
        v = reader.readValue(&newPos);

        QCOMPARE(newPos, expectedPos);
        QCOMPARE(v.isLink(), true);
        QCOMPARE(v.toLink().objNum(), quint32(expectedObjNum));
        QCOMPARE(v.toLink().genNum(), quint16(expectedGenNum));
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

    const QByteArray byteArray = data.toLocal8Bit();
    Reader reader(byteArray.data(), byteArray.length());

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

    double res = v.toNumber().value();
    QCOMPARE(res, expected);
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testReadNum_data()
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
void TestBoomaga::testPdfArray()
{
    //......................................
    {
        Array a1;
        QCOMPARE(a1.isValid(), true);
        QCOMPARE(a1.count(),   0);

        Value v;
        QCOMPARE(v.toArray().isValid(), false);
        QCOMPARE(v.toArray().count(),   0);

        Array &la1 = a1;
        QCOMPARE(la1.isValid(), true);
        QCOMPARE(la1.count(),   0);

        Array &la2 = v.toArray();
        QCOMPARE(la2.isValid(), false);
        QCOMPARE(la2.count(),   0);
    }
    //......................................

    //......................................
    {
        Array a1;
        Array a2 = a1;
        Array &la1 = a1;

        a1.append(Number(42));

        QCOMPARE(a1.count(),   1);
        QCOMPARE(a2.count(),   0);
        QCOMPARE(la1.count(),  1);

    }
    //......................................

    //......................................
    {
        Array a1;
        QCOMPARE(a1.count(),   0);

        a1.append(Number(42));
        a1.append(Number(55));
        a1.append(Number(33));

        QCOMPARE(a1.count(),   3);
        QCOMPARE(a1.at(0).toNumber().value(),     42.0);
        QCOMPARE(a1.at(1).toNumber().value(),     55.0);
        QCOMPARE(a1.at(2).toNumber().value(),     33.0);

        QCOMPARE(a1[0].toNumber().value(),        42.0);
        QCOMPARE(a1[1].toNumber().value(),        55.0);
        QCOMPARE(a1[2].toNumber().value(),        33.0);

        a1.remove(1);
        QCOMPARE(a1.count(),   2);
        QCOMPARE(a1.at(0).toNumber().value(),     42.0);
        QCOMPARE(a1.at(1).toNumber().value(),     33.0);

    }
    //......................................


}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfBool()
{
    {
        Bool b;
        QCOMPARE(b.isValid(), true);
        QCOMPARE(b.value(),   false);
    }
    //......................................
    {
        Bool b(false);
        QCOMPARE(b.isValid(), true);
        QCOMPARE(b.value(),   false);
    }
    //......................................
    {
        Bool b(true);
        QCOMPARE(b.isValid(), true);
        QCOMPARE(b.value(),   true);
    }
    //......................................
    {
        Bool b = false;
        QCOMPARE(b.isValid(), true);
        QCOMPARE(b.value(),   false);

        b.setValue(true);
        QCOMPARE(b.value(),   true);
    }
    //......................................
    {
        Bool b = true;
        QCOMPARE(b.isValid(), true);
        QCOMPARE(b.value(),   true);

        b.setValue(false);
        QCOMPARE(b.value(),   false);
    }
    //......................................
    {
        Bool b = true;
        Bool &lb = b;
        QCOMPARE(lb.isValid(), true);

        QCOMPARE(b.value(),   true);
        QCOMPARE(lb.value(),  true);

        b.setValue(false);

        QCOMPARE(b.value(),   false);
        QCOMPARE(lb.value(),  false);
    }
    //......................................
    {
        Bool b = false;
        Bool &lb = b;

        QCOMPARE(b.value(),   false);
        QCOMPARE(lb.value(),  false);

        lb.setValue(true);

        QCOMPARE(b.value(),   true);
        QCOMPARE(lb.value(),  true);
    }
    //......................................
    {
        Bool b = true;
        Bool b2 = b;
        QCOMPARE(b2.isValid(), true);

        QCOMPARE(b.value(),   true);
        QCOMPARE(b2.value(),  true);

        b.setValue(false);

        QCOMPARE(b.value(),   false);
        QCOMPARE(b2.value(),  true);
    }
    //......................................
    {
        Value v;
        QCOMPARE(v.toBool().isValid(), false);

        Bool &lb = v.toBool();
        QCOMPARE(lb.isValid(), false);

        lb.setValue(true);
        QCOMPARE(lb.value(),  false);
        QCOMPARE(v.toBool().value(),  false);
    }
    //......................................
    {
        Value v;
        QCOMPARE(v.toBool().isValid(), false);


        Bool b = v.toBool();
        QCOMPARE(b.isValid(), true);

        b.setValue(true);
        QCOMPARE(b.value(),           true);
        QCOMPARE(v.toBool().value(),  false);

    }
    //......................................

}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfNumber()
{
    //......................................
    {
        Number n0;
        QCOMPARE(n0.isValid(), true);
        QCOMPARE(n0.value(), 0.0);

        Number n1;
        n1.setValue(42);
        QCOMPARE(n1.isValid(), true);
        QCOMPARE(n1.value(), 42.0);

        Number n2(33);
        QCOMPARE(n2.isValid(), true);
        QCOMPARE(n2.value(), 33.0);

        Number n3(n1);
        QCOMPARE(n3.isValid(), true);
        QCOMPARE(n3.value(), 42.0);

        Number n4 = n1;
        QCOMPARE(n4.isValid(), true);
        QCOMPARE(n4.value(), 42.0);


    }

    //......................................
    {
        Number n = 42;
        Number n2 = n;
        n.setValue(12);
        QCOMPARE(n.value(),  12.0);
        QCOMPARE(n2.value(), 42.0);
    }

    //......................................
    {
        Number n = 42;
        Number &ln = n;
        ln.setValue(15);
        QCOMPARE(n.value(),  15.0);
        QCOMPARE(ln.value(), 15.0);

        n.setValue(33);
        QCOMPARE(n.value(),  33.0);
        QCOMPARE(ln.value(), 33.0);

    }

    //......................................
    {
        Number n(42);
        Value v = n;

        Number &ln = v.toNumber();
        QCOMPARE(ln.value(), 42.0);

        ln.setValue(10);
        QCOMPARE(ln.value(), 10.0);
        QCOMPARE(n.value(),  42.0);
    }

    //......................................
    {
        Number n(42);
        Value &v = n;

        Number &ln = v.toNumber();
        QCOMPARE(ln.value(), 42.0);

        ln.setValue(10);
        QCOMPARE(ln.value(), 10.0);
        QCOMPARE(n.value(),  10.0);
    }

    //......................................
    {
        Number n(42);
        Value v = n;

        Number &n2 = v.toNumber();
        QCOMPARE(n2.value(), 42.0);

        n2.setValue(10);
        QCOMPARE(n.value(),  42.0);
        QCOMPARE(n2.value(), 10.0);
    }

    //......................................

    //......................................
    {
        Value v;
        Number &ln1 = v.toNumber();
        ln1.setValue(42);
        QCOMPARE(ln1.value(),  0.0);

        Number &ln2 = v.toNumber();
        QCOMPARE(ln2.value(),  0.0);

        ln2.setValue(33);
        QCOMPARE(ln1.value(),  0.0);
        QCOMPARE(ln2.value(),  0.0);

    }
    //......................................

    //......................................
    {
        Value v;
        Number &ln = v.toNumber();
        QCOMPARE(ln.isValid(),  false);

        ln.setValue(11);
        QCOMPARE(ln.value(), 0.0);

        Number n = v.toNumber();
        QCOMPARE(n.isValid(),  true);

        n.setValue(11);
        QCOMPARE(n.value(), 11.0);

    }
    //......................................
}

void TestBoomaga::testPdfDict()
{
    //......................................
    {
        Dict d1;
        QCOMPARE(d1.isValid(),  true);
        QCOMPARE(d1.count(),    0);
        QCOMPARE(d1.isEmpty(),  true);

        d1.insert("Test", Number(42));
        QCOMPARE(d1.count(),  1);
        QCOMPARE(d1.isEmpty(),  false);
        bool ok;
        double n = d1.value("Test").toNumber(&ok).value();
        QCOMPARE(ok,  true);
        QCOMPARE(n,   42.0);
    }
    //......................................

    //......................................
    {
        Dict d1;
        Dict &ld1 = d1;

        d1.insert("value_D1", Number(42));
        QCOMPARE(d1.count(),  1);
        QCOMPARE(d1.isEmpty(),  false);

        bool ok;
        double n = ld1.value("value_D1").toNumber(&ok).value();
        QCOMPARE(ok,  true);
        QCOMPARE(n,   42.0);

        Dict d2 = d1;
        d2.insert("value_D1", Number(55));
        d2.insert("value_D2", Number(17));

        QCOMPARE(d1.value("value_D1").toNumber(&ok).value(),   42.0);

        QCOMPARE(d2.value("value_D1").toNumber(&ok).value(),   55.0);
        QCOMPARE(d2.value("value_D2").toNumber(&ok).value(),   17.0);
    }
    //......................................

    //......................................
    {
        bool ok;

        Dict d1;
        d1.insert("value_D1", Number(42));

        Value v = d1;

        Dict &lv= v.toDict(&ok);
        QCOMPARE(ok,  true);

        Dict d2 = v.toDict(&ok);
        QCOMPARE(ok,  true);

        d2.insert("value_D2", Number(55));

        QCOMPARE(d1.value("value_D1").toNumber().value(),   42.0);
        QCOMPARE(lv.value("value_D1").toNumber().value(),   42.0);

        QCOMPARE(d2.value("value_D1").toNumber().value(),   42.0);
        QCOMPARE(d2.value("value_D2").toNumber().value(),   55.0);

        lv.insert("value_D1", Number(10));
        QCOMPARE(lv.value("value_D1").toNumber().value(),   10.0);
        QCOMPARE(v.toDict().value("value_D1").toNumber().value(),   10.0);
        QCOMPARE(d1.value("value_D1").toNumber().value(),   42.0);


        QCOMPARE(d2.value("value_D1").toNumber().value(),   42.0);
    }
    //......................................

    //......................................
    {
        bool ok;
        Value v;
        QCOMPARE(v.toDict(&ok).count(), 0);
        QCOMPARE(ok,  false);

        v.toDict().insert("KEY", 11);
        QCOMPARE(v.toDict().count(), 0);

    }
    //......................................

    //......................................
    {
        Value v;
        QCOMPARE(v.isValid(),   false);
        QCOMPARE(v.type(),      Value::Type::Undefined);

        Dict &ld = v.toDict();
        QCOMPARE(ld.isValid(),  false);
        QCOMPARE(ld.type(),     Value::Type::Dict);

        Dict d1;
        QCOMPARE(d1.isValid(),  true);
        QCOMPARE(d1.type(),     Value::Type::Dict);

        Dict d2(v.toDict());
        QCOMPARE(d2.isValid(),  true);
        QCOMPARE(d2.type(),     Value::Type::Dict);

        Dict d3 = v.toDict();
        QCOMPARE(d3.isValid(),  true);
        QCOMPARE(d3.type(),     Value::Type::Dict);

    }
    //......................................

    //......................................
    {
        Value v;
        Dict &ld = v.toDict();
        QCOMPARE(ld.isValid(),  false);

        ld.toDict().insert("KEY", 11);
        QCOMPARE(ld.count(), 0);

        Dict d = v.toDict();
        QCOMPARE(d.isValid(),  true);

        d.toDict().insert("KEY", 11);
        QCOMPARE(d.count(), 1);

    }
    //......................................

}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfHexString()
{
    {
        HexString s;
        QCOMPARE(s.isValid(), true);
        QCOMPARE(s.value(),   QByteArray(""));
    }
    //......................................
    {
        HexString s("Test");
        QCOMPARE(s.isValid(), true);
        QCOMPARE(s.value(),   QByteArray("54657374"));
    }
    //......................................
    {
        HexString s("First");
        HexString &l = s;
        QCOMPARE(l.isValid(), true);

        QCOMPARE(s.value(),  QString("First").toLocal8Bit().toHex());
        QCOMPARE(l.value(),  QString("First").toLocal8Bit().toHex());

        s.setValue(QString("Second").toLocal8Bit().toHex());

        QCOMPARE(s.value(),   QString("Second").toLocal8Bit().toHex());
        QCOMPARE(l.value(),   QString("Second").toLocal8Bit().toHex());
    }
    //......................................
    {
        HexString s("First");
        HexString s2 = s;
        QCOMPARE(s2.isValid(), true);

        QCOMPARE(s.value(),  QString("First").toLocal8Bit().toHex());
        QCOMPARE(s2.value(), QString("First").toLocal8Bit().toHex());

        s.setValue(QString("Second").toLocal8Bit().toHex());

        QCOMPARE(s.value(),   QString("Second").toLocal8Bit().toHex());
        QCOMPARE(s2.value(),  QString("First").toLocal8Bit().toHex());
    }
    //......................................
    {
        Value v;
        QCOMPARE(v.toHexString().isValid(), false);

        HexString &l = v.toHexString();
        QCOMPARE(l.isValid(), false);

        l.setValue(QString("Test").toLocal8Bit().toHex());
        QCOMPARE(l.value(),                 QByteArray(""));
        QCOMPARE(v.toHexString().value(),   QByteArray(""));
    }
    //......................................
    {
        Value v;
        QCOMPARE(v.toHexString().isValid(), false);

        HexString s = v.toHexString();
        QCOMPARE(s.isValid(), true);

        s.setValue(QString("Test").toLocal8Bit().toHex());
        QCOMPARE(s.value(),                 QString("Test").toLocal8Bit().toHex());
        QCOMPARE(v.toHexString().value(),   QByteArray(""));
    }
    //......................................
}

void TestBoomaga::testPdfLiteralString()
{
    {
        LiteralString s;
        QCOMPARE(s.isValid(), true);
        QCOMPARE(s.value(),   QByteArray(""));
    }
    //......................................
    {
        LiteralString s("Test");
        QCOMPARE(s.isValid(), true);
        QCOMPARE(s.value(),   QByteArray("Test"));
    }
    //......................................
    {
        LiteralString s("First");
        LiteralString &l = s;
        QCOMPARE(l.isValid(), true);

        QCOMPARE(s.value(),  QString("First").toLocal8Bit());
        QCOMPARE(l.value(),  QString("First").toLocal8Bit());

        s.setValue(QString("Second").toLocal8Bit());

        QCOMPARE(s.value(),   QString("Second").toLocal8Bit());
        QCOMPARE(l.value(),   QString("Second").toLocal8Bit());
    }
    //......................................
    {
        LiteralString s("First");
        LiteralString s2 = s;
        QCOMPARE(s2.isValid(), true);

        QCOMPARE(s.value(),  QString("First").toLocal8Bit());
        QCOMPARE(s2.value(), QString("First").toLocal8Bit());

        s.setValue(QString("Second").toLocal8Bit());

        QCOMPARE(s.value(),   QString("Second").toLocal8Bit());
        QCOMPARE(s2.value(),  QString("First").toLocal8Bit());
    }
    //......................................
    {
        Value v;
        QCOMPARE(v.toLiteralString().isValid(), false);

        LiteralString &l = v.toLiteralString();
        QCOMPARE(l.isValid(), false);

        l.setValue(QString("Test").toLocal8Bit());
        QCOMPARE(l.value(),                 QByteArray(""));
        QCOMPARE(v.toLiteralString().value(),   QByteArray(""));
    }
    //......................................
    {
        Value v;
        QCOMPARE(v.toLiteralString().isValid(), false);

        LiteralString s = v.toLiteralString();
        QCOMPARE(s.isValid(), true);

        s.setValue(QString("Test").toLocal8Bit());
        QCOMPARE(s.value(),                 QString("Test").toLocal8Bit());
        QCOMPARE(v.toLiteralString().value(),   QByteArray(""));
    }
    //......................................
}

void TestBoomaga::testPdfName()
{
    {
        Name s;
        QCOMPARE(s.isValid(), true);
        QCOMPARE(s.value(),   QString(""));
    }
    //......................................
    {
        Name s("Test");
        QCOMPARE(s.isValid(), true);
        QCOMPARE(s.value(),   QString("Test"));
    }
    //......................................
    {
        Name s("First");
        Name &l = s;
        QCOMPARE(l.isValid(), true);

        QCOMPARE(s.value(),  QString("First"));
        QCOMPARE(l.value(),  QString("First"));

        s.setValue("Second");

        QCOMPARE(s.value(),   QString("Second"));
        QCOMPARE(l.value(),   QString("Second"));
    }
    //......................................
    {
        Name s("First");
        Name s2 = s;
        QCOMPARE(s2.isValid(), true);

        QCOMPARE(s.value(),  QString("First"));
        QCOMPARE(s2.value(), QString("First"));

        s.setValue("Second");

        QCOMPARE(s.value(),   QString("Second"));
        QCOMPARE(s2.value(),  QString("First"));
    }
    //......................................
    {
        Value v;
        QCOMPARE(v.toName().isValid(), false);

        Name &l = v.toName();
        QCOMPARE(l.isValid(), false);

        l.setValue("Test");
        QCOMPARE(l.value(),             QString(""));
        QCOMPARE(v.toName().value(),    QString(""));
    }
    //......................................
    {
        Value v;
        QCOMPARE(v.toName().isValid(), false);

        Name s = v.toName();
        QCOMPARE(s.isValid(), true);

        s.setValue("Test");
        QCOMPARE(s.value(),             QString("Test"));
        QCOMPARE(v.toName().value(),    QString(""));
    }
    //......................................

}
