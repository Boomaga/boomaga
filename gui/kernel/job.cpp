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

#include <QDebug>

/************************************************

 ************************************************/
Job::Job(const InputFile &inputfile, QObject *parent):
    QObject(parent),
    mInputFile(inputfile)
{
}


/************************************************
 *
 ************************************************/
Job::Job(const Job *other, QObject *parent):
    QObject(parent)
{
    mInputFile = other->mInputFile;
    mTitle = other->mTitle;
    for (int i=0; i< other->pageCount(); ++i)
    {
        addPage(new ProjectPage(other->page(i)));
    }
}


/************************************************

 ************************************************/
Job::~Job()
{
}


/************************************************

 ************************************************/
void Job::addPage(ProjectPage *page)
{
    mPages.append(page);
    connect(page, SIGNAL(visibleChanged()),
            this, SLOT(emitPageVisibleChanged()));
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
void Job::emitPageVisibleChanged()
{
    ProjectPage *page = qobject_cast<ProjectPage*>(sender());
    if (!page)
        return;

    emit pageVisibleChanged(page);
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
QList<InputFile> JobList::inputFiles() const
{
    QList<InputFile> result;

    InputFile last;
    for(int j=0; j<this->count(); ++j)
    {
        Job * job = this->at(j);
        for(int i=0; i<job->pageCount(); ++i)
        {
            ProjectPage *page = job->page(i);
            if (page->inputFile() != last &&
                result.indexOf(page->inputFile()) < 0)
            {
                result << page->inputFile();
                last = page->inputFile();
            }
        }
    }

    return result;
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
