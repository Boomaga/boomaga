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
void TestBoomaga::testPdfString()
{
    {
        PDF::String s;
        QCOMPARE(s.isValid(), true);
        QCOMPARE(s.value(),   QString(""));
    }
    //......................................
    {
        PDF::String s("Test");
        QCOMPARE(s.isValid(), true);
        QCOMPARE(s.value(),   QString("Test"));
    }
    //......................................
    {
        PDF::String s("First");
        PDF::String &l = s;
        QCOMPARE(l.isValid(), true);

        QCOMPARE(s.value(),  QString("First"));
        QCOMPARE(l.value(),  QString("First"));

        s.setValue(QString("Second"));

        QCOMPARE(s.value(),   QString("Second"));
        QCOMPARE(l.value(),   QString("Second"));
    }
    //......................................
    {
        PDF::String s("First");
        PDF::String s2 = s;
        QCOMPARE(s2.isValid(), true);

        QCOMPARE(s.value(),  QString("First"));
        QCOMPARE(s2.value(), QString("First"));

        s.setValue(QString("Second"));

        QCOMPARE(s.value(),   QString("Second"));
        QCOMPARE(s2.value(),  QString("First"));
    }
    //......................................
    {
        Value v;
        QCOMPARE(v.asString().isValid(), false);

        PDF::String &l = v.asString();
        QCOMPARE(l.isValid(), false);

        l.setValue(QString("Test"));
        QCOMPARE(l.value(),              QString(""));
        QCOMPARE(v.asString().value(),   QString(""));
    }
    //......................................
    {
        Value v;
        QCOMPARE(v.asString().isValid(), false);

        PDF::String s = v.asString();
        QCOMPARE(s.isValid(), true);

        s.setValue(QString("Test"));
        QCOMPARE(s.value(),              QString("Test"));
        QCOMPARE(v.asString().value(),   QString(""));
    }
    //......................................
}


/************************************************
 *
 ************************************************/
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


