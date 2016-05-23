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
    mJobPageNum(-1),
    mPageNum(-1),
    mSheet(0),
    mVisible(true),
    mManualRotation(NoRotate),
    mManualStartSubBooklet(false),
    mAutoStartSubBooklet(false)
{

}


/************************************************
 *
 ************************************************/
ProjectPage::ProjectPage(const InputFile &inputFile, int jobPageNum, QObject *parent):
    QObject(parent),
    mInputFile(inputFile),
    mJobPageNum(jobPageNum),
    mPageNum(-1),
    mSheet(0),
    mVisible(true),
    mManualRotation(NoRotate),
    mManualStartSubBooklet(false),
    mAutoStartSubBooklet(false)
{
}


/************************************************

 ************************************************/
ProjectPage::~ProjectPage()
{
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
    return mJobPageNum < 0;
}


/************************************************
 *
 ************************************************/
void ProjectPage::setManualStartSubBooklet(bool value)
{
    mManualStartSubBooklet = value;
}


/************************************************
 *
 ************************************************/
void ProjectPage::setAutoStartSubBooklet(bool value)
{
    mAutoStartSubBooklet = value;
}


/************************************************
 *
 ************************************************/
ProjectPage *ProjectPage::clone(QObject *parent)
{
    ProjectPage *res = new ProjectPage(parent);
    res->mInputFile = mInputFile;
    res->mJobPageNum = mJobPageNum;
    res->mPdfInfo = mPdfInfo;
    res->setVisible(mVisible);
    res->setManualRotation(mManualRotation);
    res->setManualStartSubBooklet(mManualStartSubBooklet);
    return res;
}

