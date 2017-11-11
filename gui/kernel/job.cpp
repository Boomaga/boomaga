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


#include "job.h"
#include "projectpage.h"

#include <QDebug>
#include <QIODevice>
#include <QFileInfo>
#include "kernel/inputfile.h"
#include "pdfparser/pdfreader.h"
#include <QSharedDataPointer>

#define LOCK_CHECK          0
#define LOCK_CREATE         100
#define LOCK_DESTROY       -1
#define LOCK_TO_REMOVE     -99
#define LOCK_TO_NOTREMOVE   99

class JobData: public QSharedData
{
public:
    JobData(const QString &fileName, qint64 startPos, qint64 endPos, const QList<int> *pages);
    ~JobData();

    QString mFileName;
    qint64 mStartPos;
    qint64 mEndPos;
    QList<ProjectPage*> mPages;
    QString mTitle;
    Job::State mState;
    QString mErrorString;
    bool mAutoRemove;

    static int lockUnlockFile(const QString &file, int lock);
    ProjectPage *takePage(ProjectPage *page);
};


/************************************************

 ************************************************/
JobData::JobData(const QString &fileName, qint64 startPos, qint64 endPos, const QList<int> *pages):
    mFileName(fileName),
    mStartPos(startPos),
    mEndPos(endPos),
    mState(Job::JobEmpty),
    mAutoRemove(false)
{
    if (!fileName.isEmpty())
    {
        QFileInfo fi(fileName);
        if (!mEndPos)
            mEndPos = fi.size();

        try
        {
            PDF::Reader reader;
            reader.open(fi.absoluteFilePath(), mStartPos, mEndPos);

            mTitle = reader.find("/Trailer/Info/Title").asString().value();
            int pageCount = reader.pageCount();

            InputFile inputFile(mFileName, mStartPos, mEndPos);

            if (pages && !pages->isEmpty())
            {
                foreach (int p, *pages)
                {
                    if (p < 0 || p >= pageCount)
                    {
                        qWarning() << QString("Page %1 out of range 1..%3")
                                      .arg(p+1)
                                      .arg(pageCount);
                        continue;
                    }

                    mPages << new ProjectPage(inputFile, p);
                }

            }
            else
            {
                for (int i=0; i< pageCount; ++i)
                    mPages << new ProjectPage(inputFile, i);
            }

            mState = Job::JobNotReady;
        }
        catch (const PDF::Error &err)
        {
            mState = Job::JobError;
            mErrorString = err.what();
        }
    }
    lockUnlockFile(mFileName, LOCK_CREATE);
}


/************************************************

 ************************************************/
JobData::~JobData()
{
    if (!mFileName.isEmpty())
    {
        lockUnlockFile(mFileName, LOCK_DESTROY);

        if (!lockUnlockFile(mFileName, LOCK_CHECK))
            QFile::remove(mFileName);
    }
    qDeleteAll(mPages);
}


/************************************************

 ************************************************/
int JobData::lockUnlockFile(const QString &file, int lock)
{
    if (file.isEmpty())
        return 0;

    int res = 0;
    static QHash<QString, int> items;
    if (items.contains(file))
        res = items[file];

    res += lock;
    items[file] = res;

    return res;
}


/************************************************

 ************************************************/
ProjectPage *JobData::takePage(ProjectPage *page)
{
    int n = mPages.indexOf(page);
    if (n>-1)
    {
        ProjectPage *page = mPages.takeAt(n);
        return page;
    }
    else
        return 0;
}


/************************************************

 ************************************************/
Job::Job():
    mData(new JobData("", 0, 0, 0))
{
}


/************************************************

 ************************************************/
Job::Job(const QString &fileName, qint64 startPos, qint64 endPos):
    mData(new JobData(fileName, startPos, endPos, 0))
{
}


/************************************************
 *
 * ***********************************************/
Job::Job(const QString &fileName, const QList<int> &pages, qint64 startPos, qint64 endPos):
    mData(new JobData(fileName, startPos, endPos, &pages))
{

}


/************************************************
 *
 ************************************************/
Job::Job(const Job &other):
    mData(other.mData)
{
}


/************************************************

 ************************************************/
Job::~Job()
{

}


/************************************************

 ************************************************/
Job &Job::operator =(const Job &other)
{
    mData = other.mData;
    return *this;
}


/************************************************
 *
 * ***********************************************/
bool Job::operator==(const Job &other) const
{
    return mData == other.mData;
}


/************************************************

 ************************************************/
int Job::pageCount() const
{
    return mData->mPages.count();
}


/************************************************

 ************************************************/
ProjectPage *Job::page(int index) const
{
    return mData->mPages[index];
}


/************************************************
 *
 * ***********************************************/
int Job::visiblePageCount() const
{
    int res =0;
    foreach (ProjectPage *p, mData->mPages)
    {
        if (p->visible())
            res++;
    }

    return res;
}


/************************************************

 ************************************************/
ProjectPage *Job::firstVisiblePage() const
{
    foreach (ProjectPage *page, mData->mPages)
    {
        if (page->visible())
            return page;
    }

    return 0;
}


/************************************************

 ************************************************/
int Job::indexOfPage(const ProjectPage *page, int from) const
{
    return mData->mPages.indexOf(const_cast<ProjectPage*>(page), from);
}


/************************************************
 *
 * ***********************************************/
void Job::insertPage(int before, ProjectPage *page)
{
    mData->mPages.insert(before, page);
}


/************************************************

 ************************************************/
void Job::addPage(ProjectPage *page)
{
    mData->mPages.append(page);
}


/************************************************
 *
 * ***********************************************/
void Job::removePage(ProjectPage *page)
{
    ProjectPage *p = mData->takePage(page);
    delete p;
}


/************************************************

 ************************************************/
void Job::removePages(const QList<ProjectPage*> pages)
{
    foreach (ProjectPage *page, pages)
    {
        ProjectPage *p = mData->takePage(page);
        delete p;
    }
}


/************************************************

 ************************************************/
ProjectPage *Job::takePage(ProjectPage *page)
{
    int n = mData->mPages.indexOf(page);
    if (n>-1)
    {
        ProjectPage *page = mData->mPages.takeAt(n);
        return page;
    }
    else
        return 0;
}


/************************************************

 ************************************************/
QString Job::title(bool human) const
{
    if (mData->mTitle.isEmpty() && human)
        return QObject::tr("Untitled");

    return mData->mTitle;
}


/************************************************

 ************************************************/
void Job::setTitle(const QString &title)
{
    mData->mTitle = title;
}


/************************************************

 ************************************************/
InputFile Job::inputFile() const
{
    return InputFile(mData->mFileName, mData->mStartPos, mData->mEndPos);
}


/************************************************

 ************************************************/
Job::State Job::state() const
{
    return mData->mState;
}


/************************************************

 ************************************************/
QString Job::errorString() const
{
    return mData->mErrorString;
}


/************************************************

 ************************************************/
bool Job::autoRemove() const
{
    return mData->mAutoRemove;
}


/************************************************

 ************************************************/
void Job::setAutoRemove(bool value)
{
    if (!mData->mFileName.isEmpty())
    {
        if (!mData->mAutoRemove && value)
            mData->lockUnlockFile(mData->mFileName, LOCK_TO_REMOVE);

        else if (mData->mAutoRemove && !value)
            mData->lockUnlockFile(mData->mFileName, LOCK_TO_NOTREMOVE);
    }
    mData->mAutoRemove = value;

}


/************************************************
 *
 * ***********************************************/
ProjectPage *Job::insertBlankPage(int before)
{
    ProjectPage *page = new ProjectPage();
    insertPage(before, page);
    return page;
}


/************************************************
 *
 ************************************************/
Job Job::clone()
{
    Job res = *this;
    res.mData.detach();
    for (int i=0; i< mData->mPages.count(); ++i)
        res.mData->mPages[i] = mData->mPages.at(i)->clone();

    return res;
}


/************************************************
 *
 * ***********************************************/
JobList::JobList():
    QList<Job>()
{

}


/************************************************
 *
 * ***********************************************/
JobList::JobList(const QList<Job> &other):
    QList<Job>(other)
{
}


/************************************************

 ************************************************/
int JobList::indexOfProjectPage(const ProjectPage * page, int from) const
{
    if (!page)
        return -1;

    for(int i=from; i<this->count(); ++i)
    {
        if (at(i).indexOfPage(page) > -1)
            return i;
    }

    return -1;
}


