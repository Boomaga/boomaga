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


#ifndef RENDER_H
#define RENDER_H

#include <QObject>
#include <QImage>
#include <QThread>


namespace poppler
{
    class document;
}

class RenderWorker: public QObject
{
    Q_OBJECT
public:
    explicit RenderWorker(const QString &fileName);
    virtual ~RenderWorker();

public slots:
    void render(int sheetNum, int resolution);

signals:
    void imageReady(QImage image, int sheetNum) const;

private slots:
    void doRender(int sheetNum, int resolution) const;

private:
    int mSheetNum;
    poppler::document *mPopplerDoc;
};


class Render : public QObject
{
    Q_OBJECT
public:
    explicit Render(QObject *parent = 0);
    virtual ~Render();


public slots:
    void render(int sheetNum, int resolution) const;
    void setFileName(const QString &fileName);

signals:
    void imageReady(QImage image, int sheetNum) const;

private:
    QThread mThread;
    RenderWorker *mWorker;

};

#endif // RENDER_H
