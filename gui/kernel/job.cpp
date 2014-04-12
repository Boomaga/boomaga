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
#include "project.h"
#include "boomagapoppler.h"

#include <QDebug>
#include <QIODevice>
#include "kernel/inputfile.h"
#include "kernel/boomagapoppler.h"

/************************************************

 ************************************************/
Job::Job(const InputFile &inputfile, QObject *parent):
    QObject(parent),
    mInputFile(inputfile),
    mState(JobEmpty)
{
    BoomagaPDFDoc *doc = new BoomagaPDFDoc(mInputFile.fileName(), mInputFile.startPos(), mInputFile.endPos());

    if (doc->isValid())
    {
        mTitle = doc->getMetaInfo("Title");
        int pageCount = doc->getNumPages();

        for (int i=0; i< pageCount; ++i)
            addPage(new ProjectPage(mInputFile, i));

        mState = JobNotReady;
    }
    else
    {
        mState = JobError;
        mErrorString = doc->errorString();
    }

    delete doc;
}


/************************************************
 *
 ************************************************/
Job::Job(const Job *other, QObject *parent):
    QObject(parent),
    mTitle(other->mTitle),
    mInputFile(other->mInputFile),
    mState(other->mState)
{
    for (int i=0; i< other->pageCount(); ++i)
        addPage(new ProjectPage(other->page(i)));
}


/************************************************

 ************************************************/
Job::~Job()
{
    qDeleteAll(mPages);
}


/************************************************
 *
 * ***********************************************/
int Job::visiblePageCount() const
{
    int res =0;
    foreach (ProjectPage *p, mPages)
    {
        if (p->visible())
            res++;
    }

    return res;
}


/************************************************
 *
 * ***********************************************/
void Job::insertPage(int before, ProjectPage *page)
{
    mPages.insert(before, page);
    connect(page, SIGNAL(visibleChanged()),
            this, SLOT(emitChanged()));

    emit changed(page);
}


/************************************************

 ************************************************/
void Job::addPage(ProjectPage *page)
{
    mPages.append(page);
    connect(page, SIGNAL(visibleChanged()),
            this, SLOT(emitChanged()));

    emit changed(page);
}


/************************************************
 *
 * ***********************************************/
void Job::removePage(ProjectPage *page)
{
    ProjectPage *p = takePage(page);
    p->deleteLater();
}


/************************************************

 ************************************************/
ProjectPage *Job::takePage(ProjectPage *page)
{
    int n = mPages.indexOf(page);
    if (n>-1)
    {
        ProjectPage *page = mPages.takeAt(n);
        disconnect(page, 0, this, 0);

        emit changed(0);
        return page;
    }
    else
        return 0;
}


/************************************************

 ************************************************/
QString Job::title(bool human) const
{
    if (mTitle.isEmpty() && human)
        return tr("Untitled");

    return mTitle;
}


/************************************************

 ************************************************/
void Job::setTitle(const QString &title)
{
    mTitle = title;
}


/************************************************
 *
 * ***********************************************/
void Job::emitChanged()
{
    ProjectPage *page = qobject_cast<ProjectPage*>(sender());
    if (!page)
        return;

    emit changed(page);
}


/************************************************
 *
 * ***********************************************/
void Job::insertBlankPage(int before)
{
    insertPage(before, new ProjectPage());
}



/************************************************
 *
 * ***********************************************/
JobList::JobList():
    QList<Job*>()
{

}


/************************************************
 *
 * ***********************************************/
JobList::JobList(const QList<Job *> &other):
    QList<Job*>(other)
{
}


/************************************************
 *
 * ***********************************************/
int JobList::indexOfInputFile(const InputFile &inputFile, int from) const
{
    for(int i=from; i<this->count(); ++i)
    {
        if (at(i)->inputFile() == inputFile)
            return i;
    }

    return -1;
}


/************************************************
 *
 * ***********************************************/
Job *JobList::findJob(ProjectPage *page) const
{
    for(int j=0; j<this->count(); ++j)
    {
        Job *job = this->value(j);
        if (job->indexOfPage(page) > -1)
            return job;
    }

    return 0;
}
