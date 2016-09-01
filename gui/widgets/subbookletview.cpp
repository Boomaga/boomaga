/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2016 Boomaga team https://github.com/Boomaga
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


#include "subbookletview.h"
#include "../kernel/project.h"

/************************************************
 *
 ************************************************/
SubBookletView::SubBookletView(QWidget *parent) :
    PagesListView(parent)
{

}


/************************************************
 *
 ************************************************/
QList<PagesListView::ItemInfo> SubBookletView::getPages() const
{
    QList<ItemInfo> res;
    if (!project->previewSheetCount())
        return res;

    QList<int> pages;
    pages << 0;
    for (int i=1; i<project->pageCount(); ++i)
    {
        if (project->page(i)->isStartSubBooklet())
            pages << i;
    }

    for (int i=0; i<pages.count(); ++i)
    {
        int endPage = i+1<pages.count() ? pages.at(i+1) : project->pageCount();
        ItemInfo page;
        page.page = pages.at(i);
        page.title = tr("Sub-booklet %1").arg(i+1) + "\n      " + tr("%1 pages").arg(endPage - page.page);
        page.toolTip = QString("<b>%1</b><p><font size=-1><i>%2</i></font>")
                .arg(tr("Sub-booklet %1").arg(i+1))
                .arg(tr("%1 pages").arg(endPage - page.page));
        res << page;
    }

    return res;
}
