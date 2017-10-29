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


#ifndef TESTBOOMAGA_H
#define TESTBOOMAGA_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QVariant>



#include "../boomagatypes.h"
class LayoutNUp;
class Sheet;
class ProjectPage;

class TestBoomaga : public QObject
{
    Q_OBJECT
public:
    explicit TestBoomaga(QObject *parent = 0);

private slots:
    void initTestCase();
    void test_RotationType();

    void test_ProjectRotation();
    void test_ProjectRotation_data();

    void test_PageRotation();
    void test_PageRotation_data();

    void test_PagePosition();
    void test_PagePosition_data();

    void test_BackendOptions();
    void test_BackendOptions_data();

    void test_ProjectFilePageSpec();
    void test_ProjectFilePageSpec_data();

    void test_BooklesSplit();
    void test_BooklesSplit_data();

    void testReadName();
    void testReadName_data();

    void testReadLink();
    void testReadLink_data();

    void testReadNum();
    void testReadNum_data();

    void testSkipDict();
    void testSkipDict_data();

    void testPdfArray();

    void testPdfBool();

    void testPdfDict();

    void testPdfHexString();

    void testPdfLiteralString();

    void testPdfName();

    void testPdfNumber();



private:
    LayoutNUp *createLayout(const QString &name);
    QList<ProjectPage*> createPages(const QString &definition);
    Sheet *createSheet(const QString &definition);
    Sheet *createSheet(int pagePerSheet, int pageRotation, QRectF  mediaBox = QRectF(), QRectF cropBox = QRectF());
    Rotation StrToRotation(const QString &str);
};



#endif // TESTBOOMAGA_H
