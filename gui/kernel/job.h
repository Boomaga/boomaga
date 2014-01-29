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

class ProjectPage;

class Job : public QObject
{
    Q_OBJECT
public:
    explicit Job(const InputFile &inputfile, QObject *parent=0);
    explicit Job(const Job *other, QObject *parent=0);
    virtual ~Job();

    int pageCount() const { return mPages.count(); }
    ProjectPage *page(int index) const { return mPages[index]; }
    int visiblePageCount() const;

    int indexOfPage(ProjectPage *page, int from = 0) const { return mPages.indexOf(page, from); }
    void insertPage(int before, ProjectPage *page);
    void addPage(ProjectPage *page);
    void removePage(ProjectPage *page);
    ProjectPage *takePage(ProjectPage *page);

    QString title(bool human = true) const;
    void setTitle(const QString &title);

    InputFile inputFile() const { return mInputFile; }

signals:
    void changed(ProjectPage *page);

private slots:
    void emitChanged();

private:
    QList<ProjectPage*> mPages;
    QString mTitle;
    InputFile mInputFile;
};


class JobList: public QList<Job*>
{
public:
    JobList();
    JobList(const QList<Job*> & other);

    QList<InputFile> inputFiles() const;
    int indexOfInputFile(const InputFile &inputFile, int from = 0) const;

    Job *findJob(ProjectPage *page) const;
};
#endif // JOB_H
