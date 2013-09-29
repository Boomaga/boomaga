#include "inputfile.h"
#include <QFile>
#include <QDebug>
#include "project.h"


/************************************************

 ************************************************/
InputFile::InputFile(const QString &fileName, int pageCount):
    mFileName(fileName),
    mAutoRemove(false),
    mPages(pageCount)
{
    for (int i=0; i<pageCount; ++i)
        mPages[i] = new ProjectPage(this, i);
}


/************************************************

 ************************************************/
InputFile::InputFile(const Job &job, int pageCount):
    mFileName(job.fileName()),
    mTitle(job.title()),
    mAutoRemove(job.autoRemove()),
    mPages(pageCount)
{
    for (int i=0; i<pageCount; ++i)
        mPages[i] = new ProjectPage(this, i);
}


/************************************************

 ************************************************/
InputFile::~InputFile()
{
    qDeleteAll(mPages);

    if (mAutoRemove)
        QFile::remove(mFileName);
}


