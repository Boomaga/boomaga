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
#include "render.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QWheelEvent>
#include <QDebug>
#include <QRectF>

#define MARGIN_H        20
#define MARGIN_V        20
#define MARGIN_BOOKLET  4
#define RESOLUTIN       150


/************************************************

 ************************************************/
PreviewWidget::PreviewWidget(QWidget *parent) :
    QFrame(parent),
    mSheetNum(-1),
    mDisplayedSheetNum(-1),
    mScaleFactor(0)
{
    QPalette pal(palette());
    pal.setColor(QPalette::Background, QColor(105, 101, 98));
    setPalette(pal);
    setAutoFillBackground(true);

    connect(project, SIGNAL(changed()),
            this, SLOT(refresh()));

    mRender = new Render(RESOLUTIN, 8, this);

    connect(project, SIGNAL(tmpFileRenamed(QString)),
            mRender, SLOT(setFileName(QString)));

    connect(mRender, SIGNAL(imageReady(QImage,int)),
            this, SLOT(sheetImageReady(QImage,int)));
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
    TransformSpec spec = project->layout()->transformSpec(sheet, pageNum, project->rotation());

    QSize size = QSize(spec.rect.width()  * mScaleFactor,
                       spec.rect.height() * mScaleFactor);

    if (isLandscape(project->rotation()))
        size.transpose();

    QRect rect(QPoint(0, 0), size);
    if (isLandscape(project->rotation()))
    {
        rect.moveRight(mDrawRect.right() - spec.rect.top()  * mScaleFactor);
        rect.moveTop( mDrawRect.top()  + spec.rect.left() * mScaleFactor);
    }
    else
    {
        rect.moveLeft(mDrawRect.left() + spec.rect.left() * mScaleFactor);
        rect.moveTop( mDrawRect.top()  + spec.rect.top()  * mScaleFactor);
    }

    if (mHints.testFlag(Sheet::HintSubBooklet))
    {
        if (rect.center().x() < mDrawRect.center().x())
            rect.adjust(-MARGIN_BOOKLET, 0, -MARGIN_BOOKLET, 0);
        else
            rect.adjust( MARGIN_BOOKLET, 0,  MARGIN_BOOKLET, 0);

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
 *
 ************************************************/
void PreviewWidget::drawShadow(QPainter &painter, QRectF rect)
{
    painter.save();
    painter.setClipRect(rect.adjusted(0, 0, 3, 3));

    QPen pen= painter.pen();
    QColor color = this->palette().color(QPalette::Background);

    rect.adjust(1, 1, 0, 0);
    pen.setColor(color.darker(130));
    painter.setPen(pen);
    painter.drawLine(rect.topRight(), rect.bottomRight());
    painter.drawLine(rect.bottomLeft(), rect.bottomRight());

    rect.adjust(1, 1, 1, 1);
    pen.setColor(color.darker(120));
    painter.setPen(pen);
    painter.drawLine(rect.topRight(), rect.bottomRight());
    painter.drawLine(rect.bottomLeft(), rect.bottomRight());

    rect.adjust(1, 1, 1, 1);
    pen.setColor(color.darker(110));
    painter.setPen(pen);
    painter.drawLine(rect.topRight(), rect.bottomRight());
    painter.drawLine(rect.bottomLeft(), rect.bottomRight());

    painter.restore();
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

    QSizeF printerSize =  project->printer()->paperRect().size();
    Rotation rotation = project->rotation();

    if (isLandscape(rotation))
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
        switch (rotation)
        {
        case NoRotate:  clipRect.setBottom(0); break;
        case Rotate90:  clipRect.setRight(0);  break;
        case Rotate180: clipRect.setBottom(0); break;
        case Rotate270: clipRect.setRight(0);  break;
        }
    }


    if (mHints.testFlag(Sheet::HintOnlyRight))
    {
        switch (rotation)
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


    if (mHints.testFlag(Sheet::HintSubBooklet))
    {
        painter.save();
        QPoint center = mDrawRect.center();
        QRectF imgRect;

        clipRect = mDrawRect;
        clipRect.setRight(center.x());
        clipRect.adjust(-MARGIN_BOOKLET, 0, -MARGIN_BOOKLET, 0);
        painter.setClipRect(clipRect);

        imgRect = img.rect();
        imgRect.setRight(img.rect().center().x());
        painter.drawImage(clipRect, img, imgRect);
        drawShadow(painter, clipRect);

        clipRect = mDrawRect;
        clipRect.setLeft(center.x());
        clipRect.adjust(MARGIN_BOOKLET, 0, MARGIN_BOOKLET, 0);
        painter.setClipRect(clipRect);

        imgRect = img.rect();
        imgRect.setLeft(img.rect().center().x());
        painter.drawImage(clipRect, img, imgRect);
        drawShadow(painter, clipRect);

        painter.restore();
    }
    else
    {
        painter.save();
        painter.setClipRect(clipRect);
        painter.drawImage(mDrawRect, img);
        drawShadow(painter, clipRect);
        painter.restore();
    }



    if (mHints.testFlag(Sheet::HintDrawFold) && !mHints.testFlag(Sheet::HintSubBooklet))
    {
        QPen pen = painter.pen();
        pen.setStyle(Qt::SolidLine);
        pen.setColor(Qt::lightGray);
        painter.setPen(pen);
        if (isLandscape(rotation))
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
            painter.drawText(this->pageRect(i).translated(10, 10), QString("%1").arg(i));
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
        mRender->cancel(mSheetNum);
        mSheetNum = qBound(0, sheetNum, project->previewSheetCount()-1);
        mRender->render(mSheetNum);
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
 *
 ************************************************/
void PreviewWidget::sheetImageReady(QImage image, int sheetNum)
{
    if (sheetNum >= qMin(mDisplayedSheetNum, mSheetNum) &&
        sheetNum <= qMax(mDisplayedSheetNum, mSheetNum))
    {
        mImage = image;
        mDisplayedSheetNum = sheetNum;
        mHints = project->previewSheet(mSheetNum)->hints();
        emit changed(mSheetNum);
        update();
    }
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
