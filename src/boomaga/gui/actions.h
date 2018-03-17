/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2014 Boomaga team https://github.com/Boomaga
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


#ifndef ACTIONS_H
#define ACTIONS_H

#include <QAction>
#include "kernel/job.h"
class ProjectPage;

class PageAction: public QAction
{
    Q_OBJECT
public:
    explicit PageAction(ProjectPage *page, QObject* parent):
        QAction(parent),
        mPage(page)
    {}

    PageAction(const QString &text, ProjectPage *page, QObject* parent):
        QAction(text, parent),
        mPage(page)
    {}

    PageAction(const QIcon &icon, const QString &text, ProjectPage *page, QObject* parent):
        QAction(icon, text, parent),
        mPage(page)
    {}

    ProjectPage *page() const { return mPage; }

private:
    ProjectPage *mPage;
};


class JobAction: public QAction
{
    Q_OBJECT
public:
    explicit JobAction(const Job &job, QObject* parent):
        QAction(parent),
        mJob(job)
    {}

    JobAction(const QString &text, const Job &job, QObject* parent):
        QAction(text, parent),
        mJob(job)
    {}

    JobAction(const QIcon &icon, const QString &text, const Job &job, QObject* parent):
        QAction(icon, text, parent),
        mJob(job)
    {}

    Job job() const { return mJob; }

private:
    Job mJob;
};


#endif // ACTIONS_H
