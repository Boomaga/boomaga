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


#ifndef GRAYSCALECONVERTER_H
#define GRAYSCALECONVERTER_H

#include <QObject>
#include <QStringList>

class GrayScaleConverterRequest;

class GrayScaleConverter : public QObject
{
    Q_OBJECT
public:
    explicit GrayScaleConverter(QObject *parent = 0);
    ~GrayScaleConverter();

    void addRequest(const QString &fromFileName, int startPos, int endPos);

    QStringList run();
    //const QStringList outFiles() const { return mOutFiles; }

signals:
    void Ready();

private slots:
    //void procReady;

private:
    QList<GrayScaleConverterRequest> *mRequests;
    //QStringList mOutFiles;
};

#endif // GRAYSCALECONVERTER_H
