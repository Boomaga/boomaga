/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2013 Boomaga team https://github.com/Boomaga
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

#include "psfile.h"
#include "kernel/psproject.h"
#include "kernel/psengine.h"
#include "settings.h"

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QDebug>

#include <math.h>

/************************************************

 ************************************************/
PsProject::PsProject(QObject *parent) :
    QObject(parent),
    mEngine(0),
    mPrinter(&mNullPrinter)
{
    mPsFile = new GsMergeFile(QDir::tempPath() + QString("/boomaga_%1.ps").arg(QCoreApplication::applicationPid()), this);

    setLayout(settings->layout());
}


/************************************************

 ************************************************/
PsProject::~PsProject()
{
    settings->setLayout(mLayout);
    settings->sync();
    qDeleteAll(mPages);
    qDeleteAll(mFiles);
}


/************************************************

 ************************************************/
void PsProject::addFile(QString fileName)
{
    emit fileAboutToBeAdded(fileName);

    PsFile *file =  new PsFile(fileName, this);
    mFiles << file;

    file->parse();
    for (int i=0; i<file->pageCount(); ++i)
    {
        PsProjectPage *p = new PsProjectPage(file, i);
        mPages << p;
    }

    mPsFile->merge(mFiles);
    updatePages();
    updateSheets();

    emit fileAdded(file);
}


/************************************************

 ************************************************/
void PsProject::removeFile(int index)
{
    emit fileAboutToBeRemoved(mFiles.at(index));

    PsFile *oldFile = mFiles.takeAt(index);

    for (int i = mPages.count()-1; i>-1; --i)
    {
        if (mPages.at(i)->file() == oldFile)
        {
            delete mPages.takeAt(i);
        }
    }
    delete oldFile;

    mPsFile->merge(mFiles);
    updatePages();
    updateSheets();

    emit fileRemoved();
}


/************************************************

 ************************************************/
void PsProject::updatePages()
{
    int n=0;
    foreach(const PsFile *f, mFiles)
    {
        foreach(PsProjectPage *page, mPages)
        {
            if (page->file() == f)
            {
                const PsFilePage &fPage = mPsFile->page(n + page->pageNum());
                page->setBegin(fPage.filePos().begin);
                page->setEnd(fPage.filePos().end);
                page->setRect(fPage.rect());
            }
        }

        n+=f->pageCount();
    }
}


/************************************************

 ************************************************/
int PsProject::pageCount() const
{
    return mPages.count();
}


/************************************************

 ************************************************/
PsProjectPage *PsProject::page(int index)
{
    return mPages[index];
}


/************************************************

 ************************************************/
void PsProject::updateSheets()
{
    qDeleteAll(mSheets);
    mSheets.clear();
    if (!mPages.isEmpty())
        mEngine->fillSheets(&mSheets);

    qDeleteAll(mPreviewSheets);
    mPreviewSheets.clear();
    if (!mPages.isEmpty())
        mEngine->fillPreviewSheets(&mPreviewSheets);

    emit changed();
}


/************************************************

 ************************************************/
void PsProject::writeDocument(const QList<const PsSheet *> &sheets, QTextStream *out)
{
    mEngine->writeDocument(sheets, out);
}


/************************************************

 ************************************************/
void PsProject::writeDocument(PsProject::PagesType pages, QTextStream *out)
{
    writeDocument(pages, ForwardOrder, out);
}


/************************************************

 ************************************************/
void PsProject::writeDocument(PsProject::PagesType pages, PsProject::PagesOrder order, QTextStream *out)
{
    int start;
    int inc;
    int end = sheetCount();

    switch (pages)
    {
    case PsProject::OddPages:
        start = 0;
        inc = 2;
        break;

    case PsProject::EvenPages:
        start = 1;
        inc = 2;
        break;

    case PsProject::AllPages:
        start = 0;
        inc = 1;
        break;
    }

    QList<const PsSheet *> sheets;

    if (order == PsProject::ForwardOrder)
    {
        for (int i=start; i < end; i += inc)
            sheets << mSheets.at(i);
    }
    else
    {
        for (int i=end-1; i >=start; i -= inc)
            sheets << mSheets.at(i);
    }

    writeDocument(sheets, out);
}


/************************************************

 ************************************************/
void PsProject::setLayout(PsProject::PsLayout layout)
{
    if (!mEngine || mLayout != layout)
    {
        mLayout = layout;
        PsEngine *oldEngine = mEngine;
        switch (mLayout)
        {
        case Layout1Up:
            mEngine = new EngineNUp(this, 1, 1);
            break;

        case Layout2Up:
            mEngine = new EngineNUp(this, 2, 1);
            break;

        case Layout4Up:
            mEngine = new EngineNUp(this, 2, 2);
            break;

        case Layout8Up:
            mEngine = new EngineNUp(this, 4, 2);
            break;

        case LayoutBooklet:
            mEngine = new EngineBooklet(this);
            break;
        }

        delete oldEngine;

        updateSheets();
    }
}


/************************************************

 ************************************************/
QString PsProject::layoutToStr(PsProject::PsLayout value)
{
    switch (value)
    {
    case Layout1Up:     return "1up";
    case Layout2Up:     return "2up";
    case Layout4Up:     return "4up";
    case Layout8Up:     return "8up";
    case LayoutBooklet: return "Booklet";
    }
}


/************************************************

 ************************************************/
PsProject::PsLayout PsProject::strToLayout(const QString &value)
{
    QString s = value.toUpper();
    if (s == "1UP")     return Layout1Up;
    if (s == "2UP")     return Layout2Up;
    if (s == "4UP")     return Layout4Up;
    if (s == "8UP")     return Layout4Up;
    if (s == "BOOKLET") return LayoutBooklet;

    return LayoutBooklet;
}


/************************************************

 ************************************************/
void PsProject::setPrinter(Printer *value)
{
    if (value)
        mPrinter = value;
    else
        mPrinter = &mNullPrinter;

    updateSheets();
    emit changed();
}


/************************************************

 ************************************************/
PsProjectPage::PsProjectPage(PsFile *file, int pageNum):
    mFile(file),
    mPageNum(pageNum)
{

}


