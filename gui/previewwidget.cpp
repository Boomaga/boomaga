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
    mSheetNum(-1),
    mScaleFactor(0)
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
    if (mSheetNum < 0 || !mScaleFactor)
        return QRectF();

    Sheet * sheet = project->previewSheet(mSheetNum);
    TransformSpec spec = project->layout()->transformSpec(sheet, pageNum);

    QSize size = QSize(spec.rect.width()  * mScaleFactor,
                       spec.rect.height() * mScaleFactor);

    if (isLandscape(sheet->rotation()))
        size.transpose();

    QRect rect(QPoint(0, 0), size);
    if (isLandscape(sheet->rotation()))
    {
        rect.moveLeft(mDrawRect.left() + spec.rect.top()  * mScaleFactor);
        rect.moveTop( mDrawRect.top()  + spec.rect.left() * mScaleFactor);
    }
    else
    {
        rect.moveLeft(mDrawRect.left() + spec.rect.left() * mScaleFactor);
        rect.moveTop( mDrawRect.top()  + spec.rect.top()  * mScaleFactor);
    }

    return rect;
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

//#define DEBUG_LAYOUT
#ifdef DEBUG_LAYOUT
    {
        if (mImage.isNull())
            return;

        if (mSheetNum < 0)
            return;

        Sheet *sheet = project->previewSheet(mSheetNum);

        if (!sheet)
            return;

        QPainter painter(this);
        QRectF rect = project->printer()->paperRect();
        painter.fillRect(rect, Qt::white);


        for (int i=0; i< sheet->count(); ++i)
        {
            QPen pen = painter.pen();
            pen.setStyle(Qt::DotLine);
            pen.setColor(Qt::darkGray);
            painter.setPen(pen);
            TransformSpec spec = project->layout()->transformSpec(sheet, i);

            painter.drawRect(spec.rect);
            QFont font = painter.font();
            font.setPixelSize(spec.rect.height() / 2 );
            painter.setFont(font);
            painter.drawText(spec.rect, Qt::AlignCenter, QString("%1").arg(i+1));


            pen.setStyle(Qt::SolidLine);
            pen.setColor(Qt::red);
            painter.setPen(pen);

            switch (spec.rotation)
            {
            case NoRotate:
                painter.drawLine(spec.rect.topLeft(), spec.rect.topRight());
                break;

            case Rotate90:
                painter.drawLine(spec.rect.topRight(), spec.rect.bottomRight());
                break;

            case Rotate180:
                painter.drawLine(spec.rect.bottomLeft(), spec.rect.bottomRight());
                break;

            case Rotate270:
                painter.drawLine(spec.rect.topLeft(), spec.rect.bottomLeft());
                break;
            }
        }
        return;
    }

#endif

    if (mImage.isNull())
        return;

    Sheet *sheet = project->previewSheet(mSheetNum);

    QSizeF printerSize =  project->printer()->paperRect().size();
    if (isLandscape(sheet->rotation()))
        printerSize.transpose();

    mScaleFactor = qMin((this->geometry().width()  - 2.0 * MARGIN_H) * 1.0 / printerSize.width(),
                        (this->geometry().height() - 2.0 * MARGIN_V) * 1.0 / printerSize.height());

    if (mScaleFactor == 0)
    {
        mDrawRect = QRect();
        return;
    }

    QSize size = QSize(printerSize.width()  * mScaleFactor,
                       printerSize.height() * mScaleFactor);

    mDrawRect = QRect(QPoint(0, 0), size);
    mDrawRect.moveCenter(QPoint(0, 0));

    QRectF clipRect = mDrawRect;

    if (mHints.testFlag(Sheet::HintOnlyLeft))
    {
        switch (sheet->rotation())
        {
        case NoRotate:  clipRect.setBottom(0); break;
        case Rotate90:  clipRect.setRight(0);  break;
        case Rotate180: clipRect.setBottom(0); break;
        case Rotate270: clipRect.setRight(0);  break;
        }
    }


    if (mHints.testFlag(Sheet::HintOnlyRight))
    {
        switch (sheet->rotation())
        {
        case NoRotate:  clipRect.setTop(0);    break;
        case Rotate90:  clipRect.setLeft(0);   break;
        case Rotate180: clipRect.setTop(0);    break;
        case Rotate270: clipRect.setLeft(0);   break;
        }
    }


    QImage img = mImage.scaled(mDrawRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    // Draw .....................................
    QPainter painter(this);
    painter.save();
    painter.translate(geometry().center());

    painter.save();
    painter.setClipRect(clipRect);
    painter.drawImage(mDrawRect, img);
    painter.restore();

    if (mHints.testFlag(Sheet::HintDrawFold))
    {
        QPen pen = painter.pen();
        pen.setStyle(Qt::SolidLine);
        pen.setColor(Qt::lightGray);
        painter.setPen(pen);
        if (isLandscape(sheet->rotation()))
            painter.drawLine(0, mDrawRect.top(), 0, mDrawRect.bottom());
        else
            painter.drawLine(mDrawRect.left(), 0, mDrawRect.right(), 0);
    }
    painter.restore();

    mDrawRect.moveCenter(geometry().center());

//#define DEBUG_CLICK_RECT
#ifdef DEBUG_CLICK_RECT
    {
        painter.save();
        Sheet *sheet = project->previewSheet(mSheetNum);
        for (int i=0; i< sheet->count(); ++i)
        {
            QPen pen = painter.pen();
            pen.setStyle(Qt::DotLine);
            pen.setColor(Qt::red);
            painter.setPen(pen);
            painter.drawRect(this->pageRect(i));
        }
        painter.restore();
    }
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
