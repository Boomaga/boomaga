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


#ifndef JOBLISTVIEW_H
#define JOBLISTVIEW_H

#include <QListWidget>
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <kernel/job.h>
#include <QDropEvent>

#include "pagelistview.h"
class JobListView: public PagesListView
{
    Q_OBJECT
public:
    explicit JobListView(QWidget *parent = 0);

signals:
    void contextMenuRequested(Job job);

protected:
    QList<ItemInfo> getPages() const;
    void contextMenuEvent(QContextMenuEvent *e);
};


#endif // JOBLISTVIEW_H
