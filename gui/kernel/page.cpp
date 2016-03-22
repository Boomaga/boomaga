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


#include "page.h"
#include "project.h"

/************************************************

 ************************************************/
Page::Page(QObject *parent):
    QObject(parent),
    mPageNum(-1),
    mVisible(true),
    mManualRotation(NoRotate),
    mStartSubBooklet(false)
{

}


/************************************************
 *
 * ***********************************************/
Page::Page(const Page *other, QObject *parent):
    QObject(parent),
    mInputFile(other->mInputFile),
    mPageNum(other->mPageNum),
    mVisible(other->mVisible),
    mPdfInfo(other->mPdfInfo),
    mManualRotation(other->mManualRotation),
    mStartSubBooklet(other->mStartSubBooklet)
{

}


/************************************************
 *
 ************************************************/
Page::Page(const InputFile &inputFile, int pageNum, QObject *parent):
    QObject(parent),
    mInputFile(inputFile),
    mPageNum(pageNum),
    mVisible(true),
    mManualRotation(NoRotate),
    mStartSubBooklet(false)
{
}


/************************************************

 ************************************************/
Page::~Page()
{
}


/************************************************
 *
 * ***********************************************/
QRectF Page::rect() const
{
    if (mPdfInfo.cropBox.isValid())
        return mPdfInfo.cropBox;
    else
        return project->printer()->paperRect();
}


/************************************************

 ************************************************/
Rotation Page::pdfRotation() const
{
    int r = mPdfInfo.rotate % 360;

    if (r == 90)    return Rotate90;
    if (r == 180)   return Rotate180;
    if (r == 270)   return Rotate270;
    else            return NoRotate;
}


/************************************************

 ************************************************/
void Page::setVisible(bool value)
{
    mVisible = value;
}


/************************************************

 ************************************************/
bool Page::isBlankPage() const
{
    return mPageNum < 0;
}


/************************************************
 *
 ************************************************/
void Page::setStartSubBooklet(bool value)
{
    mStartSubBooklet = value;
}
