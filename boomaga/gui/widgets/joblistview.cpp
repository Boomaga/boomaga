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


#include "joblistview.h"
#include "kernel/project.h"
#include "icon.h"
#include "../render.h"
#include <QMouseEvent>
#include <QDebug>
#include <QPainter>
#include <QBuffer>
#include <QAbstractItemModel>

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QTextDocument>
#endif

/************************************************
 *
 ************************************************/
JobListView::JobListView(QWidget *parent):
    PagesListView(parent)
{
    connect(this, SIGNAL(itemMoved(int,int)),
            project, SLOT(moveJob(int,int)));
}


/************************************************
 *
 ************************************************/
QList<PagesListView::ItemInfo> JobListView::getPages() const
{
    QList<ItemInfo> res;
    int pageNum = 0;
    for (int i=0; i<project->jobs()->count(); ++i)
    {
        Job job = project->jobs()->at(i);
        ItemInfo page;
        page.title = job.title() + "\n      " + tr("%1 pages").arg(job.visiblePageCount());
        page.page = job.visiblePageCount() ? pageNum : -1;
        page.toolTip = QString("<b>%1</b><p><font size=-1><i>%2</i></font>")
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
                .arg(Qt::escape(job.title()))
#else
                .arg(job.title().toHtmlEscaped())
#endif
                .arg(tr("%1 pages").arg(job.visiblePageCount()));

        res << page;

        pageNum += job.visiblePageCount();
    }
    return res;
}


/************************************************
 *
 ************************************************/
void JobListView::contextMenuEvent(QContextMenuEvent *e)
{
    int n = indexAt(e->pos()).row();
    if (n >-1 && n < project->jobs()->count())
        emit contextMenuRequested(project->jobs()->at(n));
}
