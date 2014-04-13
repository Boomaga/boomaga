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


/************************************************

 ************************************************/
InputFile::InputFile():
    mStartPos(0),
    mEndPos(0)
{
}


/************************************************

 ************************************************/
InputFile::InputFile(const QString &fileName, qint64 startPos, qint64 endPos):
    mFileName(fileName),
    mStartPos(startPos),
    mEndPos(endPos)
{
    if (endPos)
        mEndPos = endPos;
    else
        mEndPos = QFileInfo(fileName).size();

}


/************************************************

 ************************************************/
InputFile::InputFile(const InputFile &other):
    mFileName(other.mFileName),
    mStartPos(other.mStartPos),
    mEndPos(other.mEndPos)
{
}


/************************************************

 ************************************************/
InputFile::~InputFile()
{
}


/************************************************

 ************************************************/
InputFile &InputFile::operator =(const InputFile &other)
{
    if (this != &other)
    {
        mFileName = other.mFileName;
        mStartPos = other.mStartPos;
        mEndPos = other.mEndPos;
    }

    return *this;
}


/************************************************

 ************************************************/
bool InputFile::operator ==(const InputFile &other) const
{
    return mStartPos == other.startPos() &&
           mEndPos   == other.mEndPos &&
           mFileName == other.mFileName;
}

