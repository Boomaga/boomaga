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


#include "projectpage.h"
#include "project.h"

/************************************************

 ************************************************/
ProjectPage::ProjectPage(QObject *parent):
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
ProjectPage::ProjectPage(const ProjectPage *other, QObject *parent):
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
ProjectPage::ProjectPage(const InputFile &inputFile, int pageNum, QObject *parent):
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
ProjectPage::~ProjectPage()
{
    qDebug() << Q_FUNC_INFO << this;
}


/************************************************
 *
 * ***********************************************/
QRectF ProjectPage::rect() const
{
    if (mPdfInfo.cropBox.isValid())
        return mPdfInfo.cropBox;
    else
        return project->printer()->paperRect();
}


/************************************************

 ************************************************/
Rotation ProjectPage::pdfRotation() const
{
    int r = mPdfInfo.rotate % 360;

    if (r == 90)    return Rotate90;
    if (r == 180)   return Rotate180;
    if (r == 270)   return Rotate270;
    else            return NoRotate;
}


/************************************************

 ************************************************/
void ProjectPage::setVisible(bool value)
{
    mVisible = value;
}


/************************************************

 ************************************************/
bool ProjectPage::isBlankPage() const
{
    return mPageNum < 0;
}


/************************************************
 *
 ************************************************/
void ProjectPage::setStartSubBooklet(bool value)
{
    mStartSubBooklet = value;
}

