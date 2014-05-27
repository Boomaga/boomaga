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


#ifndef JOB_H
#define JOB_H

#include <QList>
#include <QObject>
#include <QString>
#include "inputfile.h"

#include <QExplicitlySharedDataPointer>

class ProjectPage;
class QIODevice;
class PDFDoc;
class JobData;

class Job
{
public:
    enum State
    {
        JobEmpty,
        JobNotReady,
        JobReady,
        JobError
    };

    explicit Job();
    explicit Job(const QString &fileName, qint64 startPos=0, qint64 endPos=0);
    Job(const Job &other);
    virtual ~Job();

    Job& operator=(const Job& other);
    bool operator==(const Job& other) const;

    int pageCount() const;
    ProjectPage *page(int index) const;
    int visiblePageCount() const;
    ProjectPage *firstVisiblePage() const;

    int indexOfPage(const ProjectPage *page, int from = 0) const;
    void insertPage(int before, ProjectPage *page);
    void addPage(ProjectPage *page);
    void removePage(ProjectPage *page);
    void removePages(const QList<ProjectPage*> pages);

    ProjectPage *takePage(ProjectPage *page);

    QString title(bool human = true) const;
    void setTitle(const QString &title);

    InputFile inputFile() const;

    State state() const;
    QString errorString() const;

    bool autoRemove() const;
    void setAutoRemove(bool value);
    void insertBlankPage(int before);

private:
    QExplicitlySharedDataPointer<JobData> mData;
};


class JobList: public QList<Job>
{
public:
    JobList();
    JobList(const QList<Job> & other);

    int indexOfProjectPage(const ProjectPage *page, int from = 0) const;
};

#endif // JOB_H
