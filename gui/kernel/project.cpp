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

#include "kernel/project.h"
#include "settings.h"
#include "job.h"

#include "inputfile.h"
#include "tmppdffile.h"
#include "sheet.h"
#include "layout.h"

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QMessageBox>

#include <math.h>




/************************************************

 ************************************************/
ProjectPage::ProjectPage():
    QObject(),
    mPageNum(-1),
    mPdfObjectNum(0),
    mVisible(true)
{

}


/************************************************
 *
 * ***********************************************/
ProjectPage::ProjectPage(const ProjectPage *other):
    QObject(),
    mInputFile(other->mInputFile),
    mPageNum(other->mPageNum),
    mPdfObjectNum(other->mPdfObjectNum),
    mRect(other->mRect),
    mVisible(other->mVisible)
{

}

/************************************************
 *
 ************************************************/
ProjectPage::ProjectPage(const InputFile &inputFile, int pageNum):
    QObject(),
    mInputFile(inputFile),
    mPageNum(pageNum),
    mPdfObjectNum(0),
    mVisible(true)
{
}


/************************************************

 ************************************************/
ProjectPage::~ProjectPage()
{
}


/************************************************
 *
 * ***********************************************/
QRectF ProjectPage::rect() const
{
    if (mRect.isValid())
        return mRect;
    else
        return project->printer()->paperRect();
}


/************************************************

 ************************************************/
void ProjectPage::setVisible(bool value)
{
    mVisible = value;
    emit visibleChanged();
}


/************************************************

 ************************************************/
Project::Project(QObject *parent) :
    QObject(parent),
    mLayout(0),
    mTmpFile(0),
    mLastTmpFile(0),
    mPrinter(&mNullPrinter),
    mDoubleSided(true)
{
}


/************************************************

 ************************************************/
Project::~Project()
{
    free();
}


/************************************************

 ************************************************/
void Project::free()
{
    delete mTmpFile;
    qDeleteAll(mJobs);
}


/************************************************

 ************************************************/
TmpPdfFile *Project::createTmpPdfFile(QList<InputFile> files)
{
    TmpPdfFile *res = new TmpPdfFile(files, this);

    connect(res, SIGNAL(progress(int,int)),
            this, SLOT(tmpFileProgress(int,int)));

    connect(res, SIGNAL(merged()),
            this, SLOT(tmpFileMerged()));

    return res;
}


/************************************************

 ************************************************/
void Project::addFile(InputFile file)
{
    addFiles(QList<InputFile>() << file);
}


/************************************************

 ************************************************/
void Project::addFiles(QList<InputFile> files)
{
    QList<InputFile> request;
    request << mInputFiles;
    request << files;
    stopMerging();

    mLastTmpFile = createTmpPdfFile(request);
    mLastTmpFile->merge();
}


/************************************************

 ************************************************/
void Project::removeJob(int index)
{
    stopMerging();
    Job * job = mJobs.takeAt(index);
    mInputFiles.removeAll(job->inputFile());
    delete job;

    update();

    mLastTmpFile = createTmpPdfFile(mInputFiles);
    mLastTmpFile->merge();
}


/************************************************

 ************************************************/
void Project::moveJob(int from, int to)
{
    mJobs.move(from, to);
    update();
}


/************************************************

 ************************************************/
void Project::tmpFileMerged()
{
    TmpPdfFile *tmpPdf = qobject_cast<TmpPdfFile*>(sender());

    if (!tmpPdf)
        return;

    if (tmpPdf != mLastTmpFile)
    {
        tmpPdf->deleteLater();
        return;
    }

    if (!tmpPdf->isValid())
    {
//TODO:        if (mTmpFile)
//TODO:            mInputFiles = mTmpFile->inputFiles();

        tmpPdf->deleteLater();
        mLastTmpFile = 0;
        return;
    }

    // Update jobs and remove old one ................
    QMutableListIterator<Job*> j(mJobs);

    while(j.hasNext())
    {
        Job *job = j.next();
        int n = tmpPdf->jobs().indexOfInputFile(job->inputFile());

        if (n<0)
        {
            j.remove();
            delete job;
        }
        else
        {
            for(int p=0; p<job->pageCount(); ++p)
            {
                ProjectPage *projPage = job->page(p);

                if (!projPage->inputFile().isNull())
                {
                    ProjectPage *tmpPage = tmpPdf->jobs().at(n)->page(projPage->pageNum());
                    projPage->setPdfObjectNum(tmpPage->pdfObjectNum());
                    projPage->setRect(tmpPage->rect());
                }
            }
        }
    }
    //................................................

    // Add new jobs and its pages ....................
    foreach(Job *tmpJob, tmpPdf->jobs())
    //for (int i=0; i<tmpPdf->jobsinputFiles().count(); ++i)
    {
        int n = mJobs.indexOfInputFile(tmpJob->inputFile());

        if (n<0)
        {
            Job *job = new Job(tmpJob);
            connect(job, SIGNAL(changed(ProjectPage*)),
                    this, SLOT(update()));

            mJobs << job;
            mInputFiles << job->inputFile();
        }
    }
    //................................................

    delete mTmpFile;
    mTmpFile = mLastTmpFile;
    connect(mTmpFile, SIGNAL(imageChanged(int)),
            this, SIGNAL(sheetImageChanged(int)));

    mLastTmpFile = 0;

    update();
}


/************************************************

 ************************************************/
void Project::update()
{
    mPages.clear();
    foreach(Job *job, mJobs)
    {
        for (int p=0; p<job->pageCount(); ++p)
        {
            if (job->page(p)->visible())
                mPages << job->page(p);
        }
    }

    qDeleteAll(mSheets);
    mSheets.clear();

    qDeleteAll(mPreviewSheets);
    mPreviewSheets.clear();

    if (!mPages.isEmpty())
    {
        mLayout->fillSheets(&mSheets);
        mLayout->fillPreviewSheets(&mPreviewSheets);

        mTmpFile->updateSheets(mPreviewSheets);
    }

    emit changed();
}


/************************************************

 ************************************************/
void Project::stopMerging()
{
    if (mLastTmpFile)
    {
        mLastTmpFile->stop();
        mLastTmpFile->deleteLater();
        mLastTmpFile = 0;
    }
}


/************************************************

 ************************************************/
void Project::tmpFileProgress(int progr, int all) const
{
    if (sender() == mLastTmpFile)
        emit progress(progr, all);
}


/************************************************

 ************************************************/
bool Project::error(const QString &message)
{
    QMessageBox::critical(0, tr("Boomaga", "Error message title"), message);
    qWarning() << message;
    return false;
}


/************************************************

 ************************************************/
QList<Sheet*> Project::selectSheets(Project::PagesType pages, Project::PagesOrder order) const
{
    int start = 0;
    int inc = 0;
    int end = sheetCount();

    switch (pages)
    {
    case Project::OddPages:
        start = 0;
        inc = 2;
        break;

    case Project::EvenPages:
        start = 1;
        inc = 2;
        break;

    case Project::AllPages:
        start = 0;
        inc = 1;
        break;
    }

    QList<Sheet *> res;

    if (order == Project::ForwardOrder)
    {
        for (int i=start; i < end; i += inc)
            res.append(mSheets.at(i));
    }
    else
    {
        for (int i=start; i < end; i += inc)
            res.insert(0, mSheets.at(i));
    }

    return res;
}


/************************************************

 ************************************************/
void Project::writeDocument(const QList<Sheet *> &sheets, QIODevice *out)
{
    mTmpFile->writeDocument(sheets, out);
}


/************************************************

 ************************************************/
void Project::writeDocument(const QList<Sheet *> &sheets, const QString &fileName)
{
    QFile f(fileName);
    f.open(QIODevice::WriteOnly);
    writeDocument(sheets, &f);
    f.close();
}


/************************************************
 *
 ************************************************/
bool Project::doubleSided() const
{
    if (mLayout->id() == "Booklet")
        return true;
    else
        return mDoubleSided;
}


/************************************************

 ************************************************/
void Project::setLayout(const Layout *layout)
{
    mLayout = layout;
    update();
}


/************************************************
 *
 ************************************************/
void Project::setDoubleSided(bool value)
{
    mDoubleSided = value;
    emit changed();
}


/************************************************

 ************************************************/
void Project::setPrinter(Printer *value)
{
    if (value)
        mPrinter = value;
    else
        mPrinter = &mNullPrinter;

    update();
    emit changed();
}


/************************************************

 ************************************************/
Project *Project::instance()
{
    static Project *inst = 0;
    if (!inst)
        inst = new Project();

    return inst;
}


/************************************************

 ************************************************/
QImage Project::sheetImage(int sheetNum) const
{
    if (mTmpFile)
        return mTmpFile->image(sheetNum);
    else
        return QImage();
}

