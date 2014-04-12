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


#ifndef INPUTFILE_H
#define INPUTFILE_H

#include <QString>
#include <QVector>
#include <QSharedDataPointer>

class InputFileData;
class QIODevice;
class Job;

class InputFile
{
public:
    InputFile();
    InputFile(const QString &fileName, const QString &title = "", bool autoRemove=false, qint64 startPos=0, qint64 endPos=0);
    InputFile(const InputFile &other);
    InputFile &operator=(const InputFile &other);
    ~InputFile();

    bool operator==(const InputFile &other) const;
    inline bool operator!=(const InputFile &other) const { return !operator==(other); }

    QString fileName() const;
    QString title() const;
    bool autoRemove() const;
    bool isNull() const;
    qint64 startPos() const;
    qint64 endPos() const;
    qint64 length() const;

private:
    QSharedDataPointer<InputFileData> mData;
};

class InputFileList: public QList<InputFile>
{
public:
    InputFileList() : QList<InputFile>() {}
    InputFileList(const QList<InputFile> &other) : QList<InputFile>(other) {}
    InputFileList(const QList<Job*> & other);
};

#endif // INPUTFILE_H
