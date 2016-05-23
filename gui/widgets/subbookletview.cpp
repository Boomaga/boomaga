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
    for (int i=0; i<project->pageCount(); ++i)
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
