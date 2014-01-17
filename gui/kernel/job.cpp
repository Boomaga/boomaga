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
    mTitle = other->title();
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
}


/************************************************

 ************************************************/
ProjectPage *Job::takePage(ProjectPage *page)
{
    int n = mPages.indexOf(page);
    if (n>-1)
        return mPages.takeAt(n);
    else
        return 0;
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
