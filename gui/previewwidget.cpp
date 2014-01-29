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


#include "previewwidget.h"
#include "kernel/project.h"
#include "kernel/layout.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QWheelEvent>
#include <QDebug>
#include <QRectF>

#define MARGIN_H 20
#define MARGIN_V 20

/************************************************

 ************************************************/
PreviewWidget::PreviewWidget(QWidget *parent) :
    QFrame(parent),
    mSheetNum(-1)
{
    QPalette pal(palette());
    pal.setColor(QPalette::Background, QColor(105, 101, 98));
    setPalette(pal);
    setAutoFillBackground(true);

    connect(project, SIGNAL(changed()),
            this, SLOT(refresh()));

    connect(project, SIGNAL(sheetImageChanged(int)),
            this, SLOT(sheetImageChanged(int)));
}


/************************************************

 ************************************************/
PreviewWidget::~PreviewWidget()
{
}


/************************************************
 *
 * ***********************************************/
QRectF PreviewWidget::pageRect(int pageNum) const
{
    if (mSheetNum < 0)
        return QRectF();

    Sheet * sheet = project->previewSheet(mSheetNum);
    TransformSpec spec = project->layout()->transformSpec(sheet, pageNum);

    if (mHints.testFlag(Sheet::HintLandscapePreview))
    {
        double factor = qMin((this->geometry().height() - MARGIN_H) * 1.0 / project->printer()->paperRect().width(),
                             (this->geometry().width()  - MARGIN_V) * 1.0 / project->printer()->paperRect().height());

        QRectF pageRect;
        pageRect.setHeight(factor * project->printer()->paperRect().width());
        pageRect.setWidth(factor  * project->printer()->paperRect().height());
        pageRect.moveCenter(geometry().center());

        return QRectF(pageRect.left()   + spec.rect.top()   * factor,
                      pageRect.bottom() - spec.rect.right() * factor,
                      factor * spec.rect.height(),
                      factor * spec.rect.width());
    }
    else
    {
        double factor = qMin((this->geometry().width()  - MARGIN_H) * 1.0 / project->printer()->paperRect().width(),
                             (this->geometry().height() - MARGIN_V) * 1.0 / project->printer()->paperRect().height());

        QRectF pageRect;
        pageRect.setHeight(factor * project->printer()->paperRect().height());
        pageRect.setWidth(factor  * project->printer()->paperRect().width());
        pageRect.moveCenter(geometry().center());

        return QRectF(pageRect.left() + spec.rect.left() * factor,
                      pageRect.top()  + spec.rect.top()  * factor,
                      factor * spec.rect.width(),
                      factor * spec.rect.height());
    }
}


/************************************************
 *
 * ***********************************************/
int PreviewWidget::pageAt(const QPoint &point) const
{
    if (mSheetNum < 0)
        return -1;

    Sheet * sheet = project->previewSheet(mSheetNum);
    for (int i=0; i<sheet->count(); ++i)
    {
        if (pageRect(i).contains(point))
            return i;
    }

    return -1;
}


/************************************************

 ************************************************/
void PreviewWidget::sheetImageChanged(int sheetNum)
{
    if (sheetNum == mSheetNum)
        setCurrentSheet(mSheetNum);
}


/************************************************

 ************************************************/
void PreviewWidget::paintEvent(QPaintEvent *event)
{
    if (mImage.isNull())
        return;

    double factor;
    if (mHints.testFlag(Sheet::HintLandscapePreview))
    {
        factor = qMin((this->geometry().height() - MARGIN_H) * 1.0 / project->printer()->paperRect().width(),
                      (this->geometry().width()  - MARGIN_V) * 1.0 / project->printer()->paperRect().height());
    }
    else
    {
        factor = qMin((this->geometry().width()  - MARGIN_H) * 1.0 / project->printer()->paperRect().width(),
                      (this->geometry().height() - MARGIN_V) * 1.0 / project->printer()->paperRect().height());
    }

    QRect rect;
    rect.setWidth(project->printer()->paperRect().width()   * factor);
    rect.setHeight(project->printer()->paperRect().height() * factor);

    QImage img = mImage.scaled(rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);


    rect.moveCenter(QPoint(0, 0));

    QRectF clipRect = rect;

    if (mHints.testFlag(Sheet::HintOnlyLeft))
        clipRect.setTop(0);

    if (mHints.testFlag(Sheet::HintOnlyRight))
        clipRect.setBottom(0);


    // Draw .....................................
    QPainter painter(this);
    painter.save();
    painter.translate(geometry().center());

    if (mHints.testFlag(Sheet::HintLandscapePreview))
        painter.rotate(90);

    painter.save();
    painter.setClipRect(clipRect);
    painter.drawImage(rect, img);
    painter.restore();

    if (mHints.testFlag(Sheet::HintDrawFold))
    {
        QPen pen = painter.pen();
        pen.setStyle(Qt::SolidLine);
        pen.setColor(Qt::lightGray);
        painter.setPen(pen);
        painter.drawLine(rect.left(), 0, rect.right(), 0);
    }
    painter.restore();

#if 0
    painter.save();
    painter.fillRect(project->printer()->paperRect(), Qt::white);
    painter.drawRect(project->printer()->paperRect());

    Sheet *sheet = project->previewSheet(mSheetNum);
    for (int i=0; i< sheet->count(); ++i)
    {
        if (sheet->page(i))
        {
            QRectF r = project->layout()->transformSpec(sheet, i).rect;
            painter.drawRect(r);
            painter.drawText(r, QString("%1").arg(i + 1), QTextOption(Qt::AlignCenter));
        }
    }

    painter.restore();
#endif

}


/************************************************

 ************************************************/
void PreviewWidget::setCurrentSheet(int sheetNum)
{
    if (project->previewSheetCount())
    {
        mSheetNum = qBound(0, sheetNum, project->previewSheetCount()-1);
        mHints = project->previewSheet(mSheetNum)->hints();
        mImage = project->sheetImage(mSheetNum);
    }
    else
    {
        mSheetNum = -1;
        mImage = QImage();
    }

    emit changed(mSheetNum);
    update();
}


/************************************************

 ************************************************/
void PreviewWidget::nextSheet()
{
    setCurrentSheet(mSheetNum + 1);
}


/************************************************

 ************************************************/
void PreviewWidget::prevSheet()
{
    setCurrentSheet(mSheetNum - 1);
}


/************************************************

 ************************************************/
void PreviewWidget::refresh()
{
    setCurrentSheet(mSheetNum);
}


/************************************************

 ************************************************/
void PreviewWidget::wheelEvent(QWheelEvent *event)
{
    setCurrentSheet(mSheetNum +
                    (event->delta() < 0 ? 1 : -1));
}


/************************************************

 ************************************************/
void PreviewWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        switch (event->key())
        {

        case Qt::Key_Home:
            setCurrentSheet(0);
            break;

        case Qt::Key_End:
            setCurrentSheet(project->previewSheetCount()-1);
            break;

        case Qt::Key_PageUp:
            setCurrentSheet(mSheetNum - 10);
            break;

        case Qt::Key_PageDown:
            setCurrentSheet(mSheetNum + 10);
            break;

        case Qt::Key_Up:
            setCurrentSheet(mSheetNum - 1);
            break;

        case Qt::Key_Down:
            setCurrentSheet(mSheetNum + 1);
            break;

        }
    }
}


/************************************************
 *
 * ***********************************************/
void PreviewWidget::contextMenuEvent(QContextMenuEvent *event)
{
    emit contextMenuRequested(pageAt(event->pos()));
}
