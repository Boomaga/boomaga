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

class PreviewWidget : public QFrame
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = 0);
    ~PreviewWidget();
    
    int currentSheet() const { return mSheetNum; }
    QRectF pageRect(int pageNum) const;
    int pageAt(const QPoint &point) const;

public slots:
    void setCurrentSheet(int sheetNum);
    void nextSheet();
    void prevSheet();
    void refresh();

signals:
    void changed(int sheetNum);
    void contextMenuRequested(int pageNum);

protected:
    void paintEvent(QPaintEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void sheetImageChanged(int sheetNum);

private:
    QImage mImage;
    int mSheetNum;
    Sheet::Hints mHints;
};

#endif // PREVIEWWIDGET_H
