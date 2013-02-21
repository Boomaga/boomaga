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


#include "psview.h"
#include "psrender.h"
#include "kernel/pssheet.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QWheelEvent>

/************************************************

 ************************************************/
PsView::PsView(QWidget *parent) :
    QFrame(parent)
{

}


/************************************************

 ************************************************/
PsView::~PsView()
{

}


/************************************************

 ************************************************/
void PsView::setCurrentSheet(int sheetNum)
{
    mSheetNum = sheetNum;
    if (sheetNum < mRender->sheetCount())
    {
        mSheet = mRender->sheet(sheetNum);

        if (mSheet->hints().testFlag(PsSheet::HintLandscapePreview))
        {
            QMatrix matrix;
            matrix.rotate(90);
            mImage = mRender->image(sheetNum).transformed(matrix);
        }
        else
        {
            mImage = mRender->image(sheetNum);
        }
    }
    else
    {
        mSheet = 0;
        mImage = QImage();
    }

    update();
}


/************************************************

 ************************************************/
void PsView::setRender(PsRender *value)
{
    mRender = value;
    connect(mRender, SIGNAL(changed(int)), this, SLOT(renderChanged(int)));
}


/************************************************

 ************************************************/
void PsView::paintEvent(QPaintEvent *event)
{
    if (!mSheet)
        return;

    QImage img = mImage.scaled(this->geometry().width() - 20, this->geometry().height() - 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QRect dRect = img.rect();
    QRect sRect = img.rect();

    dRect.moveCenter(this->geometry().center());

    QPoint foldStart, foldEnd;
    if (mSheet->hints().testFlag(PsSheet::HintDrawFold))
    {
        foldStart = QPoint(dRect.center().x(), dRect.top());
        foldEnd = QPoint(dRect.center().x(), dRect.bottom());
    }


    if (mSheet->hints().testFlag(PsSheet::HintOnlyLeft))
    {
        sRect.setRight(sRect.center().x());
        dRect.setRight(dRect.center().x());
    }

    if (mSheet->hints().testFlag(PsSheet::HintOnlyRight))
    {
        sRect.setLeft(sRect.center().x());
        dRect.setLeft(dRect.center().x());
    }


    QPainter painter(this);


    painter.drawImage(dRect, img, sRect);

    if (mSheet->hints().testFlag(PsSheet::HintDrawFold))
    {
        QPen pen = painter.pen();
        pen.setStyle(Qt::SolidLine);
        pen.setColor(Qt::lightGray);
        painter.setPen(pen);
        painter.drawLine(foldStart, foldEnd);
    }
}


/************************************************

 ************************************************/
void PsView::wheelEvent(QWheelEvent *event)
{
    emit whellScrolled(event->delta());
}


/************************************************

 ************************************************/
void PsView::renderChanged(int sheetNum)
{
    if (sheetNum == mSheetNum)
        setCurrentSheet(mSheetNum);
}
