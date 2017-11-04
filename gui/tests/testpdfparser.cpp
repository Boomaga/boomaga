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
#include <QDebug>

using namespace PDF;


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
        QCOMPARE(v.asArray().isValid(), false);
        QCOMPARE(v.asArray().count(),   0);

        Array &la1 = a1;
        QCOMPARE(la1.isValid(), true);
        QCOMPARE(la1.count(),   0);

        Array &la2 = v.asArray();
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
        QCOMPARE(a1.at(0).asNumber().value(),     42.0);
        QCOMPARE(a1.at(1).asNumber().value(),     55.0);
        QCOMPARE(a1.at(2).asNumber().value(),     33.0);

        QCOMPARE(a1[0].asNumber().value(),        42.0);
        QCOMPARE(a1[1].asNumber().value(),        55.0);
        QCOMPARE(a1[2].asNumber().value(),        33.0);

        a1.remove(1);
        QCOMPARE(a1.count(),   2);
        QCOMPARE(a1.at(0).asNumber().value(),     42.0);
        QCOMPARE(a1.at(1).asNumber().value(),     33.0);

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
        QCOMPARE(v.asBool().isValid(), false);

        Bool &lb = v.asBool();
        QCOMPARE(lb.isValid(), false);

        lb.setValue(true);
        QCOMPARE(lb.value(),  false);
        QCOMPARE(v.asBool().value(),  false);
    }
    //......................................
    {
        Value v;
        QCOMPARE(v.asBool().isValid(), false);


        Bool b = v.asBool();
        QCOMPARE(b.isValid(), true);

        b.setValue(true);
        QCOMPARE(b.value(),           true);
        QCOMPARE(v.asBool().value(),  false);

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

        Number &ln = v.asNumber();
        QCOMPARE(ln.value(), 42.0);

        ln.setValue(10);
        QCOMPARE(ln.value(), 10.0);
        QCOMPARE(n.value(),  42.0);
    }

    //......................................
    {
        Number n(42);
        Value &v = n;

        Number &ln = v.asNumber();
        QCOMPARE(ln.value(), 42.0);

        ln.setValue(10);
        QCOMPARE(ln.value(), 10.0);
        QCOMPARE(n.value(),  10.0);
    }

    //......................................
    {
        Number n(42);
        Value v = n;

        Number &n2 = v.asNumber();
        QCOMPARE(n2.value(), 42.0);

        n2.setValue(10);
        QCOMPARE(n.value(),  42.0);
        QCOMPARE(n2.value(), 10.0);
    }

    //......................................

    //......................................
    {
        Value v;
        Number &ln1 = v.asNumber();
        ln1.setValue(42);
        QCOMPARE(ln1.value(),  0.0);

        Number &ln2 = v.asNumber();
        QCOMPARE(ln2.value(),  0.0);

        ln2.setValue(33);
        QCOMPARE(ln1.value(),  0.0);
        QCOMPARE(ln2.value(),  0.0);

    }
    //......................................

    //......................................
    {
        Value v;
        Number &ln = v.asNumber();
        QCOMPARE(ln.isValid(),  false);

        ln.setValue(11);
        QCOMPARE(ln.value(), 0.0);

        Number n = v.asNumber();
        QCOMPARE(n.isValid(),  true);

        n.setValue(11);
        QCOMPARE(n.value(), 11.0);

    }
    //......................................
}


/************************************************
 *
 ************************************************/
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
        double n = d1.value("Test").asNumber(&ok).value();
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
        double n = ld1.value("value_D1").asNumber(&ok).value();
        QCOMPARE(ok,  true);
        QCOMPARE(n,   42.0);

        Dict d2 = d1;
        d2.insert("value_D1", Number(55));
        d2.insert("value_D2", Number(17));

        QCOMPARE(d1.value("value_D1").asNumber(&ok).value(),   42.0);

        QCOMPARE(d2.value("value_D1").asNumber(&ok).value(),   55.0);
        QCOMPARE(d2.value("value_D2").asNumber(&ok).value(),   17.0);
    }
    //......................................

    //......................................
    {
        bool ok;

        Dict d1;
        d1.insert("value_D1", Number(42));

        Value v = d1;

        Dict &lv= v.asDict(&ok);
        QCOMPARE(ok,  true);

        Dict d2 = v.asDict(&ok);
        QCOMPARE(ok,  true);

        d2.insert("value_D2", Number(55));

        QCOMPARE(d1.value("value_D1").asNumber().value(),   42.0);
        QCOMPARE(lv.value("value_D1").asNumber().value(),   42.0);

        QCOMPARE(d2.value("value_D1").asNumber().value(),   42.0);
        QCOMPARE(d2.value("value_D2").asNumber().value(),   55.0);

        lv.insert("value_D1", Number(10));
        QCOMPARE(lv.value("value_D1").asNumber().value(),   10.0);
        QCOMPARE(v.asDict().value("value_D1").asNumber().value(),   10.0);
        QCOMPARE(d1.value("value_D1").asNumber().value(),   42.0);


        QCOMPARE(d2.value("value_D1").asNumber().value(),   42.0);
    }
    //......................................

    //......................................
    {
        bool ok;
        Value v;
        QCOMPARE(v.asDict(&ok).count(), 0);
        QCOMPARE(ok,  false);

        v.asDict().insert("KEY", 11);
        QCOMPARE(v.asDict().count(), 0);

    }
    //......................................

    //......................................
    {
        Value v;
        QCOMPARE(v.isValid(),   false);
        QCOMPARE(v.type(),      Value::Type::Undefined);

        Dict &ld = v.asDict();
        QCOMPARE(ld.isValid(),  false);
        QCOMPARE(ld.type(),     Value::Type::Dict);

        Dict d1;
        QCOMPARE(d1.isValid(),  true);
        QCOMPARE(d1.type(),     Value::Type::Dict);

        Dict d2(v.asDict());
        QCOMPARE(d2.isValid(),  true);
        QCOMPARE(d2.type(),     Value::Type::Dict);

        Dict d3 = v.asDict();
        QCOMPARE(d3.isValid(),  true);
        QCOMPARE(d3.type(),     Value::Type::Dict);

    }
    //......................................

    //......................................
    {
        Value v;
        Dict &ld = v.asDict();
        QCOMPARE(ld.isValid(),  false);

        ld.asDict().insert("KEY", 11);
        QCOMPARE(ld.count(), 0);

        Dict d = v.asDict();
        QCOMPARE(d.isValid(),  true);

        d.asDict().insert("KEY", 11);
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
        QCOMPARE(v.asHexString().isValid(), false);

        HexString &l = v.asHexString();
        QCOMPARE(l.isValid(), false);

        l.setValue(QString("Test").toLocal8Bit().toHex());
        QCOMPARE(l.value(),                 QByteArray(""));
        QCOMPARE(v.asHexString().value(),   QByteArray(""));
    }
    //......................................
    {
        Value v;
        QCOMPARE(v.asHexString().isValid(), false);

        HexString s = v.asHexString();
        QCOMPARE(s.isValid(), true);

        s.setValue(QString("Test").toLocal8Bit().toHex());
        QCOMPARE(s.value(),                 QString("Test").toLocal8Bit().toHex());
        QCOMPARE(v.asHexString().value(),   QByteArray(""));
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
        QCOMPARE(v.asLiteralString().isValid(), false);

        LiteralString &l = v.asLiteralString();
        QCOMPARE(l.isValid(), false);

        l.setValue(QString("Test").toLocal8Bit());
        QCOMPARE(l.value(),                 QByteArray(""));
        QCOMPARE(v.asLiteralString().value(),   QByteArray(""));
    }
    //......................................
    {
        Value v;
        QCOMPARE(v.asLiteralString().isValid(), false);

        LiteralString s = v.asLiteralString();
        QCOMPARE(s.isValid(), true);

        s.setValue(QString("Test").toLocal8Bit());
        QCOMPARE(s.value(),                 QString("Test").toLocal8Bit());
        QCOMPARE(v.asLiteralString().value(),   QByteArray(""));
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
        QCOMPARE(v.asName().isValid(), false);

        Name &l = v.asName();
        QCOMPARE(l.isValid(), false);

        l.setValue("Test");
        QCOMPARE(l.value(),             QString(""));
        QCOMPARE(v.asName().value(),    QString(""));
    }
    //......................................
    {
        Value v;
        QCOMPARE(v.asName().isValid(), false);

        Name s = v.asName();
        QCOMPARE(s.isValid(), true);

        s.setValue("Test");
        QCOMPARE(s.value(),             QString("Test"));
        QCOMPARE(v.asName().value(),    QString(""));
    }
    //......................................

}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfString_toString()
{
    QFETCH(QString, data);
    QFETCH(QString, expected);

    if (QString(QTest::currentDataTag()).toUpper().startsWith("HEX"))
    {
        PDF::HexString s;
        s.setValue(data.toLatin1());
        QCOMPARE(s.toString(), expected);
    }
    else
    {
        PDF::LiteralString s;
        s.setValue(data.toLatin1());
        QCOMPARE(s.toString(), expected);
    }
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testPdfString_toString_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<QString>("expected");

    QTest::newRow("Hex 01") << "" << "";
    QTest::newRow("Hex 02") << "54455354" << "TEST";
    QTest::newRow("Hex 03") << "54 45 53 54 " << "TEST";
    QTest::newRow("Hex 04") << " 5 4 4 5 5 3 5 4 " << "TEST";
    QTest::newRow("Hex 05") << "D0A0D183D181D181D0BAD0B8D0B9" << "Русский";
    QTest::newRow("Hex 06") << "d0a0d183d181d181d0bad0b8d0b9" << "Русский";
    QTest::newRow("Hex 07") << "D0 A0 D1 83 \n D1 81 D1 81 \n D0 BA D0 B8 \n D0 B9" << "Русский";
    QTest::newRow("Hex 08") << "5420450A53095420" << "T E\nS\tT ";
    QTest::newRow("Hex 09") << "544553542" << "TEST ";


    QTest::newRow("Literal 01") << "" << "";
    QTest::newRow("Literal 02") << "Test" << "Test";
    QTest::newRow("Literal 03") << "Strings may contain balanced parentheses ( ) and special characters ( * ! & } ^ % and so on"
                                << "Strings may contain balanced parentheses ( ) and special characters ( * ! & } ^ % and so on";
    QTest::newRow("Literal 04") << "\\n\\r\\t\\b\\f\\(\\)\\\\" << "\n\r\t\b\f()\\";
    QTest::newRow("Literal 05") << "These \ntwo strings \nare the same." << "These \ntwo strings \nare the same.";

    QTest::newRow("Literal 06") << "These \\ \ntwo strings \\ \nare the same." << "These  \ntwo strings  \nare the same.";
    QTest::newRow("Literal 07") << "These \\\ntwo strings \\\nare the same." << "These two strings are the same.";
    QTest::newRow("Literal 08") << "These \\\rtwo strings \\\rare the same." << "These two strings are the same.";
    QTest::newRow("Literal 09") << "These \\\r\ntwo strings \\\r\nare the same." << "These two strings are the same.";
    QTest::newRow("Literal 10") << "These \\\n\rtwo strings \\\n\rare the same." << "These two strings are the same.";

    QTest::newRow("Literal 11") << "This string has an end-of-line at the end of it.\n" << "This string has an end-of-line at the end of it.\n";
    QTest::newRow("Literal 12") << "\\053"      << "+";
    QTest::newRow("Literal 13") << "\\53"       << "+";
    QTest::newRow("Literal 14") << "\\0533"     << "+3";
    QTest::newRow("Literal 15") << "\\0073"     << "\a3";
    QTest::newRow("Literal 16") << "\\7"        << "\a";
    QTest::newRow("Literal 17") << "\\07"       << "\a";
    QTest::newRow("Literal 18") << "\\007"      << "\a";
    QTest::newRow("Literal 19") << "A\\7B"      << "A\aB";
    QTest::newRow("Literal 20") << "A\\07B"     << "A\aB";
    QTest::newRow("Literal 21") << "A\\007B"    << "A\aB";
    QTest::newRow("Literal 22") << "\\245"      << "\245";
    QTest::newRow("Literal 23") << "\\8"        << "8";
    QTest::newRow("Literal 24") << "\\9"        << "9";
    QTest::newRow("Literal 25") << "a\\aa"      << "aaa";
    QTest::newRow("Literal 26") << "a\\za"      << "aza";
}

