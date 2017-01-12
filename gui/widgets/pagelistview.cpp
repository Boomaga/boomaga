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


#include "pagelistview.h"
#include "kernel/project.h"
#include "../render.h"

#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>
#include <QPainter>
#include <QBuffer>
#include <limits>

#include "boomagatypes.h"

#define RESOLUTIN 30
#define PAGE_NUM_ROLE           Qt::UserRole + 1
#define TOOLTIP_TEMPLATE_ROLE   Qt::UserRole + 2

#define MIN_ICON_SIZE 32
#define MAX_ICON_SIZE 200

#define TOOLTIP_HTML "<html>" \
    "<table border=0><tr>" \
        "<td><img height=200 src='%IMG%'></td>" \
        "<td><div style='margin: 8px 16px 8px 16px;'>%1</div></td>" \
    "</tr></table>" \
    "</html>"

#define STYLE_SHEET \
    "QListView::item{" \
        "margin: 4px;" \
    "} " \
    "QToolTip {" \
    "opacity: 254; "\
    "}"


/************************************************
 *
 ************************************************/
PagesListView::PagesListView(QWidget *parent):
    QListWidget(parent),
    mRender(new Render(RESOLUTIN)),
    mIconSize(64)
{
    connect(project, SIGNAL(tmpFileRenamed(QString)),
            mRender, SLOT(setFileName(QString)));

    connect(mRender, SIGNAL(pageReady(QImage,int)),
            this, SLOT(previewRedy(QImage,int)));

    connect(project, SIGNAL(changed()),
            this, SLOT(updateItems()));

    setIconSize(64);

    setStyleSheet(STYLE_SHEET);
}


/************************************************
 *
 ************************************************/
PagesListView::~PagesListView()
{
}


/************************************************
 *
 ************************************************/
void PagesListView::setIconSize(int size)
{
    mIconSize = qBound(MIN_ICON_SIZE, size, MAX_ICON_SIZE);
    QListWidget::setIconSize(QSize(mIconSize, mIconSize));
}


/************************************************
 *
 ************************************************/
int PagesListView::itemPageCount(int row) const
{
    int end = row + 1 < count() ? item(row + 1)->data(PAGE_NUM_ROLE).toInt() - 1 : project->pageCount() -1;
    return end - item(row)->data(PAGE_NUM_ROLE).toInt();
}


/************************************************
 *
 ************************************************/
void PagesListView::updateItems()
{

    QList<ItemInfo> pages = getPages();
    int cur = currentRow();

    setUpdatesEnabled(false);
    while(this->count() > pages.count())
    {
        delete takeItem(this->count()-1);
    }

    for (int i=0; i<count(); ++i)
    {
        ItemInfo page = pages.at(i);
        QListWidgetItem *item = this->item(i);
        item->setText(page.title);
        item->setData(PAGE_NUM_ROLE, page.page);
        item->setData(TOOLTIP_TEMPLATE_ROLE, page.toolTip);

        if (page.page > -1)
            mRender->renderPage(page.page);
    }

    while(this->count() < pages.count())
    {
        ItemInfo page = pages.at(this->count());
        QListWidgetItem *item = new QListWidgetItem(this);
        item->setText(page.title);
        item->setData(PAGE_NUM_ROLE, page.page);
        item->setData(TOOLTIP_TEMPLATE_ROLE, page.toolTip);
        item->setIcon(createIcon(QImage()));
        addItem(item);

        if (page.page > -1)
            mRender->renderPage(page.page);
    }

    setCurrentRow(cur);
    setUpdatesEnabled(true);
}


/************************************************
 *
 ************************************************/
void PagesListView::setPageNum(int pageNum)
{
    if (count() < 1)
        return;

    if (pageNum < 0)
    {
        setCurrentRow(0);
        return;
    }

    for (int i=count()-1; i>-1; --i)
    {
        int p = item(i)->data(PAGE_NUM_ROLE).toInt();
        if (p <= pageNum)
        {
            setCurrentRow(i);
            return;
        }
    }

    setCurrentRow(0);
}

/************************************************
 *
 ************************************************/
QString imageAsText(const QImage &image)
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return QString("data:image/png;base64,") + QString(buffer.data().toBase64());
}


/************************************************
 *
 ************************************************/
void PagesListView::previewRedy(QImage image, int pageNum)
{
    for (int i=0; i<count(); ++i)
    {
        if (item(i)->data(PAGE_NUM_ROLE) == pageNum)
        {
            item(i)->setIcon(createIcon(image));

            QString toolTip = item(i)->data(TOOLTIP_TEMPLATE_ROLE).toString();
            if (toolTip.isEmpty())
            {
                item(i)->setToolTip("");
            }
            else
            {
                item(i)->setToolTip(QString(TOOLTIP_HTML)
                                .replace("%IMG%", imageAsText(image))
                                .arg(toolTip));
            }

            return;
        }
    }
}


/************************************************
 *
 ************************************************/
void PagesListView::mouseReleaseEvent(QMouseEvent *e)
{
    int n = indexAt(e->pos()).row();
    if (n < 0)
        return;

    bool ok;
    int page = item(n)->data(PAGE_NUM_ROLE).toInt(&ok);
    if (!ok || page < 0)
        return;

    emit pageSelected(page);

    int sheet = project->previewSheets().indexOfPage(page);
    if (sheet < 0)
        return;

    emit sheetSelected(sheet);
}


/************************************************
 *
 ************************************************/
void PagesListView::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::CTRL)
    {
        if (e->delta() > 0)
            setIconSize(iconSize() + 8);
        else
            setIconSize(iconSize() - 8);

        e->ignore();
    }
    else
    {
        QListWidget::wheelEvent(e);
    }
}


/************************************************
 *
 ************************************************/
void PagesListView::dropEvent(QDropEvent *e)
{
    int from = currentRow();
    if (from < 0)
    {
        e->ignore();
        return;
    }

    int to = indexAt(e->pos()).row();
    if (dropIndicatorPosition() == QAbstractItemView::BelowItem)
    {
        to++;
    }
    else if (dropIndicatorPosition() == QAbstractItemView::OnViewport)
    {
        if (e->pos().y() > visualItemRect(item(count()-1)).bottom())
            to = count();
    }

    if (to > from)
        to--;


    if (to < 0)
    {
        e->ignore();
        return;

    }

    QListWidget::dropEvent(e);
    emit itemMoved(from, to);
}


/************************************************
 *
 ************************************************/
int PagesListView::indexOfPage(int pageNum) const
{
    for (int i=0; i<this->count(); ++i)
    {
        if (this->item(i)->data(PAGE_NUM_ROLE) == pageNum)
            return i;

    }
    return -1;
}


/************************************************
 *
 ************************************************/
QIcon PagesListView::createIcon(const QImage &image) const
{
    QImage img;
    if (image.isNull())
    {
        QSizeF size = project->printer()->paperSize(UnitPoint);
        size.scale(MAX_ICON_SIZE, MAX_ICON_SIZE, Qt::KeepAspectRatio);

        img = QImage(size.toSize(), QImage::Format_ARGB32);
        img.fill(Qt::white);
    }
    else
    {
        img = image.scaled(MAX_ICON_SIZE, MAX_ICON_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if (project->grayscale())
            img = toGrayscale(img);
    }

    QPainter painter(&img);
    painter.setPen(Qt::gray);
    painter.drawRect(img.rect().adjusted(0, 0, -1, -1));

    return QIcon(QPixmap::fromImage(img));
}
