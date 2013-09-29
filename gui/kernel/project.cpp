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
Job::Job(const QString &fileName, const QString &title, bool autoRemove):
    mFileName(fileName),
    mTitle(title),
    mAutoRemove(autoRemove)
{
}


/************************************************

 ************************************************/
Job::Job(const Job &other)
{
    mFileName = other.mFileName;
    mTitle = other.mTitle;
    mAutoRemove = other.mAutoRemove;
}


/************************************************

 ************************************************/
Job::Job(const InputFile *inputFile)
{
    mFileName = inputFile->fileName();
    mTitle = inputFile->title();
    mAutoRemove = inputFile->autoRemove();
}


/************************************************

 ************************************************/
int Jobs::indexOf(const QString &fileName)
{
    for (int i=0; i<count(); ++i)
    {
        if (this->at(i).fileName() == fileName)
            return i;
    }
    return -1;
}


/************************************************

 ************************************************/
void ProjectPageList::removeFile(const InputFile *file)
{
    for(int i=count()-1; i>-1; --i)
    {
        if (at(i)->inputFile() == file)
            takeAt(i);
    }
}


/************************************************

 ************************************************/
void ProjectPageList::moveFile(const InputFile *file, const InputFile *before)
{
    if (file == before)
        return;

    removeFile(file);

    int n = indexOfFirstPage(before);
    for(int i=file->pageCount()-1; i>-1; --i)
    {
        insert(n, file->pages().at(i));
    }

}


/************************************************

 ************************************************/
void ProjectPageList::addFile(const InputFile *file)
{
    foreach (ProjectPage *page, file->pages())
        *this << page;
}

/************************************************

 ************************************************/
int ProjectPageList::indexOfFirstPage(const InputFile *file)
{
    for(int i=0; i<count(); ++i)
    {
        if (at(i)->inputFile() == file)
            return i;
    }

    return -1;
}




/************************************************

 ************************************************/
ProjectPage::ProjectPage(InputFile *inputFile, int pageNum):
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

 ************************************************/
void ProjectPage::setVisible(bool value)
{
    mVisible = value;
}

int ProjectPage::num(int inc)
{
    static int n=0;
    n +=inc;
    return n;
}


/************************************************

 ************************************************/
Project::Project(QObject *parent) :
    QObject(parent),
    mLayout(0),
    mTmpFile(0),
    mLastTmpFile(0),
    mPrinter(&mNullPrinter)
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
}


/************************************************

 ************************************************/
//bool searchInJobs(const QList<Job> jobs, const QString &fileName)
//{
//    foreach (Job job, jobs)
//    {
//        if (job.fileName() == fileName)
//            return true;
//    }

//    return false;
//}


/************************************************

 ************************************************/
InputFile *searchInInputFiles(QList<InputFile*> files, const QString &fileName)
{
    foreach(InputFile *file, files)
    {
        if (file->fileName() == fileName)
            return file;
    }

    return 0;
}


/************************************************

 ************************************************/
TmpPdfFile *Project::createTmpPdfFile(QList<Job> jobs)
{
    TmpPdfFile *res = new TmpPdfFile(jobs, this);

    connect(res, SIGNAL(progress(int,int)),
            this, SLOT(tmpFileProgress(int,int)));

    connect(res, SIGNAL(merged()),
            this, SLOT(tmpFileMerged()));

    return res;
}


/************************************************

 ************************************************/
void Project::addFile(Job job)
{
    addFiles(QList<Job>() << job);
}


/************************************************

 ************************************************/
void Project::addFiles(QList<Job> jobs)
{

    mJobs << jobs;
    stopMerging();

    mLastTmpFile = createTmpPdfFile(mJobs);
    mLastTmpFile->merge();
}


/************************************************

 ************************************************/
void Project::removeFile(int index)
{
    stopMerging();
    int n = mJobs.indexOf(mFiles.at(index)->fileName());
    mJobs.removeAt(n);

    InputFile *file = mFiles.takeAt(index);
    mPages.removeFile(file);
    delete file;

    updateSheets();

    mLastTmpFile = createTmpPdfFile(mJobs);
    mLastTmpFile->merge();
}

/************************************************

 ************************************************/
void Project::moveFile(int from, int to)
{
    mPages.moveFile(mFiles.at(from), mFiles.at(to));

    int f = mJobs.indexOf(mFiles.at(from)->fileName());
    int t = mJobs.indexOf(mFiles.at(to)->fileName());
    mJobs.move(f, t);

    mFiles.move(from, to);

    updateSheets();
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
        qDebug() << Q_FUNC_INFO << "before tmpPdf->deleteLater";
        tmpPdf->deleteLater();
        qDebug() << Q_FUNC_INFO << "after tmpPdf->deleteLater";
        mLastTmpFile = 0;
        return;
    }


    // Remove old InputFiles and its pages ...........
    for (int i=mFiles.count()-1; i>-1; --i)
    {
        if (mJobs.indexOf(mFiles.at(i)->fileName()) < 0)
        {
            InputFile *file = mFiles.takeAt(i);
            mPages.removeFile(file);
            delete file;
        }
    }
    // ...............................................

    // Add new InputFiles and its pages ..............
    int p = 0;
    foreach(Job job, tmpPdf->jobs())
    {
        InputFile *file = searchInInputFiles(mFiles, job.fileName());
        if (!file)
        {
            file = new InputFile(job, tmpPdf->inputFilePageCount(job.fileName()));
            mFiles << file;
            mPages.addFile(file);
        }

        int cnt = file->pageCount();
        for (int i=0; i<cnt; ++i)
        {
            ProjectPage *page = file->pages()[i];
            TmpPdfFilePage pdfPage = tmpPdf->page(p);

            page->setPdfObjectNum(pdfPage.pdfObjNum);
            page->setRect(pdfPage.rect);
            p++;
        }
    }
    // ...............................................
    delete mTmpFile;
    mTmpFile = mLastTmpFile;
    connect(mTmpFile, SIGNAL(imageChanged(int)),
            this, SIGNAL(sheetImageChanged(int)));

    mLastTmpFile = 0;

    updateSheets();
}


/************************************************

 ************************************************/
void Project::updateSheets()
{
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
    int start;
    int inc;
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
            res << mSheets.at(i);
    }
    else
    {
        for (int i=end-1; i >=start; i -= inc)
            res << mSheets.at(i);
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

 ************************************************/
void Project::setLayout(const Layout *layout)
{
    mLayout = layout;
    updateSheets();
}


/************************************************

 ************************************************/
void Project::setPrinter(Printer *value)
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










