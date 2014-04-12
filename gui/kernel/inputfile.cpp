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

#include "inputfile.h"
#include <QFile>
#include <QIODevice>
#include <QFileInfo>
#include <QDebug>
#include "project.h"



class InputFileData : public QSharedData {
public:
    InputFileData()
    {
        mAutoRemove = false;
        mIsNull = true;
        mStartPos = 0;
        mEndPos = 0;
    }

    QString mFileName;
    QString mTitle;
    bool mAutoRemove;
    bool mIsNull;
    qint64 mStartPos;
    qint64 mEndPos;
};


/************************************************

 ************************************************/
InputFile::InputFile():
    mData(new InputFileData)
{
}



/************************************************

 ************************************************/
InputFile::InputFile(const QString &fileName, const QString &title, bool autoRemove, qint64 startPos, qint64 endPos):
    mData(new InputFileData)
{
    mData->mFileName = fileName;
    mData->mTitle = title;
    mData->mAutoRemove = autoRemove;
    mData->mIsNull = false;
    mData->mStartPos = startPos;
    if (endPos)
        mData->mEndPos = endPos;
    else
        mData->mEndPos = QFileInfo(fileName).size();

}


/************************************************

 ************************************************/
InputFile::InputFile(const InputFile &other):
    mData(other.mData)
{

}


/************************************************

 ************************************************/
InputFile &InputFile::operator =(const InputFile &other)
{
    if (this != &other)
        mData = other.mData;

    return *this;
}


/************************************************

 ************************************************/
InputFile::~InputFile()
{
}


/************************************************

 ************************************************/
bool InputFile::operator ==(const InputFile &other) const
{
    return mData == other.mData;
}


/************************************************

 ************************************************/
QString InputFile::fileName() const
{
    return mData->mFileName;
}


/************************************************

 ************************************************/
QString InputFile::title() const
{
    return mData->mTitle;
}


/************************************************

 ************************************************/
bool InputFile::autoRemove() const
{
    return mData->mAutoRemove;
}


/************************************************

 ************************************************/
bool InputFile::isNull() const
{
    return mData->mIsNull;
}


/************************************************

 ************************************************/
qint64 InputFile::startPos() const
{
    return mData->mStartPos;
}


/************************************************

 ************************************************/
qint64 InputFile::endPos() const
{
    return mData->mEndPos;
}


/************************************************

 ************************************************/
qint64 InputFile::length() const
{
    return mData->mEndPos - mData->mStartPos;
}


/************************************************
 *
 * ***********************************************/
InputFileList::InputFileList(const QList<Job *> &other):
    QList<InputFile>()
{
    foreach (Job *job, other)
        append(job->inputFile());
}
