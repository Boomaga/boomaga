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


#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H


#include <QFrame>
#include "kernel/sheet.h"
#include <QHash>

class Render;

class RenderCache: public QObject
{
    Q_OBJECT
public:
    RenderCache(double resolution, int threadCount = 8, QObject *parent = 0);
    ~RenderCache();
    QString fileName() const;

public slots:
    void setFileName(const QString &fileName);
    void renderSheet(int sheetNum);
    void cancelSheet(int sheetNum);

signals:
    void sheetReady(QImage img, int sheetNum);

private slots:
    void onSheetReady(QImage img, int sheetNum);

private:
    QHash<int, QImage> mItems;
    Render *mRender;
};

class PreviewWidget : public QFrame
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = 0);
    ~PreviewWidget();
    
    QRectF pageRect(int pageNum) const;
    int pageAt(const QPoint &point) const;

public slots:
    void refresh();

signals:
    void changed();
    void contextMenuRequested(Sheet *sheet, ProjectPage *page);

protected:
    void paintEvent(QPaintEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void sheetImageReady(QImage image, int sheetNum);

private:
    QImage mImage;
    QRect mDrawRect;
    int mDisplayedSheetNum;
    double mScaleFactor;
    Sheet::Hints mHints;
    QHash<int, Sheet::Hints> mRequests;
    RenderCache *mRender;
    int mWheelDelta;

    void drawShadow(QPainter &painter, QRectF rect);
};

#endif // PREVIEWWIDGET_H
