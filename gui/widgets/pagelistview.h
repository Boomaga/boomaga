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


#ifndef PAGELISTVIEW_H
#define PAGELISTVIEW_H

#include <QListWidget>
#include <kernel/job.h>

class Render;

class PagesListView: public QListWidget
{
     Q_OBJECT
public:
    explicit PagesListView(QWidget *parent = 0);
    virtual ~PagesListView();
    void setIconSize(int size);
    int iconSize() const { return mIconSize;  }
    int itemPageCount(int row) const;

public slots:
    void updateItems();
    void setPageNum(int pageNum);

signals:
    void pageSelected(int pageNum);
    void sheetSelected(int sheetNum);
    void itemMoved(int from, int to);

protected:
    struct ItemInfo {
        QString title;
        int page;
        QString toolTip;
    };
    virtual QList<ItemInfo> getPages() const = 0;
    void mouseReleaseEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void dropEvent(QDropEvent *e);

    int indexOfPage(int pageNum) const;

private slots:
    void previewRedy(QImage image, int pageNum);

private:
    Render *mRender;
    QImage mEmptyImage;

    QIcon createIcon(const QImage &image) const;
    int mIconSize;
};


#endif // PAGELISTVIEW_H
