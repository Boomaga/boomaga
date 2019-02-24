/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2019 Boomaga team https://github.com/Boomaga
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
#include "tools.h"
#include <QTest>
#include <QDebug>
#include "../iofiles/infile.h"
#include "../iofiles/pdffile.h"
#include "../iofiles/boofile.h"
#include "../iofiles/cupsboofile.h"
#include "../iofiles/postscriptfile.h"
#include <QSettings>
#include <QDir>


/************************************************
 *
 ************************************************/
static QString typeToString(InFile::Type type)
{
    switch (type)
    {
    case InFile::Type::Pdf:         return "PDF";
    case InFile::Type::PostScript:  return "POSTSCRIPT";
    case InFile::Type::Boo:         return "BOO";
    case InFile::Type::CupsBoo:     return "CUPSBOO";
    default:                        return "UNKNOWN";
    }
}


/************************************************
 *
 ************************************************/
#include <QProcessEnvironment>
void TestBoomaga::testInFiles()
{
    QFETCH(QString, expectedFile);
    QString inFile = expectedFile.left(expectedFile.length() - 9);

    QSettings expect(expectedFile, QSettings::IniFormat);
    expect.setIniCodec("UTF-8");

    try
    {
        QDir().mkpath(dir());
        setenv("XDG_CACHE_HOME", dir().toLocal8Bit().data(), 1);
        auto type = InFile::getType(inFile);

        QCOMPARE(typeToString(type), expect.value("type").toString());

        QObject keeper;
        InFile *res = nullptr;
        switch (type) {
        case InFile::Type::Pdf:
            res = new PdfFile(&keeper);
            break;

        case InFile::Type::Boo:
            res = new BooFile(&keeper);
            break;

        case InFile::Type::CupsBoo:
            res = new CupsBooFile(&keeper);
            break;

        case InFile::Type::PostScript:
            res = new PostScriptFile(&keeper);
            break;

        default:
            QFAIL("Unknown file type");
            break;
        }

        res->load(inFile);
        QCOMPARE(typeToString(res->type()), expect.value("type").toString());


        // Check metadata .......................
        expect.beginGroup(QString("metadata"));

        if (expect.contains("title"))
            QCOMPARE(res->metaData().title(), expect.value("title").toString());

        if (expect.contains("author"))
            QCOMPARE(res->metaData().author(), expect.value("author").toString());

        if (expect.contains("subject"))
            QCOMPARE(res->metaData().subject(), expect.value("subject").toString());

        if (expect.contains("keywords"))
            QCOMPARE(res->metaData().keywords(), expect.value("keywords").toString());

        expect.endGroup();
        // Check metadata .......................

        // Check jobs ...........................
        QCOMPARE(res->jobs().count(), expect.value("jobs count").toInt());

        for (int j=0; j<res->jobs().count(); ++j)
        {
            const Job &job = res->jobs().at(j);
            expect.beginGroup(QString("job %1").arg(j));

            if (expect.contains("pageCount"))
                QCOMPARE(job.pageCount(), expect.value("pageCount").toInt());

            if (expect.contains("title"))
                QCOMPARE(job.title(false), expect.value("title").toString());

            for (int p=0; p<job.pageCount(); ++p)
            {
                const ProjectPage *page = job.page(p);
                QString key = QString("page.%1").arg(p);

                if (expect.contains(key + ".jobPageNum"))
                {
                    int expected = expect.value(key + ".jobPageNum").toInt();
                    if (expected > -1) // Real pages are numbered from 1.
                        --expected;    // For the blank pages used -1.

                    QCOMPARE(page->jobPageNum(), expected);
                }
                else
                {
                    QCOMPARE(page->jobPageNum(), p);
                }

                {
                    bool expected = !expect.value(key + ".hidden", false).toBool();
                    QCOMPARE(page->visible(), expected);
                }

                {
                    int expected = expect.value(key + ".rotation", 0).toInt();
                    QCOMPARE(int(page->manualRotation()), expected);
                }


                {
                    bool expected = expect.value(key + ".startSubBooklet", false).toBool();
                    QCOMPARE(page->isManualStartSubBooklet(), expected);
                }

            }


            expect.endGroup();
        }
    }
    catch (BoomagaError &err)
    {
        FAIL_EXCEPTION(err);
    }
}


/************************************************
 *
 ************************************************/
void TestBoomaga::testInFiles_data()
{
    QTest::addColumn<QString>("expectedFile");
    QString srcDir = mDataDir + "testInFiles";

    foreach(QString f, QDir(srcDir).entryList(QStringList("*_expected"), QDir::Files, QDir::Name))
    {

        QTest::newRow(f.left(f.length() - 9).toLocal8Bit())
                << srcDir + QDir::separator() + f;

    }
}
