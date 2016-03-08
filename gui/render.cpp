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

#include "render.h"
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <poppler/cpp/poppler-page.h>
#include <QDebug>
#include <QFile>


/************************************************

 ************************************************/
QImage renderSheet(poppler::document *doc, int sheetNum, double resolution)
{
    poppler::page *page = doc->create_page(sheetNum);
    if (page)
    {
        poppler::page_renderer prender;
        prender.set_render_hint(poppler::page_renderer::antialiasing, true);
        prender.set_render_hint(poppler::page_renderer::text_antialiasing, true);

        poppler::image img = prender.render_page(page, resolution, resolution);

        QImage::Format format = QImage::Format_Invalid;

        switch (img.format())
        {
        case poppler::image::format_invalid: format = QImage::Format_Invalid; break;
        case poppler::image::format_mono:    format = QImage::Format_Mono;    break;
        case poppler::image::format_rgb24:   format = QImage::Format_RGB32;   break;
        case poppler::image::format_argb32:  format = QImage::Format_ARGB32;  break;
        }


        QImage result;
        if (format != QImage::Format_Invalid)
        {
            result = QImage(reinterpret_cast<const uchar*>(img.const_data()),
                            img.width(), img.height(),
                            img.bytes_per_row(),
                            format).copy();
        }

        delete page;
        return result;
    }

    return QImage();
}


/************************************************
 *
 ************************************************/
RenderWorker::RenderWorker(const QString &fileName, int resolution):
    QObject(),
    mSheetNum(0),
    mResolution(resolution),
    mBussy(false)
{
    mPopplerDoc = poppler::document::load_from_file(fileName.toLocal8Bit().data());
}


/************************************************
 *
 ************************************************/
RenderWorker::~RenderWorker()
{
    delete mPopplerDoc;
}


/************************************************
 *
 ************************************************/
void RenderWorker::render(int sheetNum)
{
    mBussy = true;
    QImage img = renderSheet(mPopplerDoc, sheetNum, mResolution);
    emit imageReady(img, sheetNum);
    mBussy = false;
}


/************************************************
 *
 ************************************************/
Render::Render(double resolution, int threadCount, QObject *parent):
    QObject(parent),
    mResolution(resolution),
    mThreadCount(threadCount)
{
    mWorkers.reserve(threadCount);
}


/************************************************
 *
 ************************************************/
Render::~Render()
{
    foreach (RenderWorker *worker, mWorkers)
    {
        worker->thread()->quit();
        worker->thread()->wait();
        delete worker;
    }
}


/************************************************
 *
 ************************************************/
void Render::setFileName(const QString &fileName)
{
    foreach(RenderWorker *worker, mWorkers)
    {
        worker->thread()->quit();
        worker->thread()->wait();
        delete worker;
    }

    mWorkers.resize(mThreadCount);

    for (int i=0; i<mWorkers.count(); ++i)
    {
        RenderWorker *worker = new RenderWorker(fileName, mResolution);
        mWorkers[i] = worker;

        connect(worker, SIGNAL(imageReady(QImage,int)),
                this, SIGNAL(imageReady(QImage,int)));

        connect(worker, SIGNAL(imageReady(QImage,int)),
                this, SLOT(workerFinished()));

        worker->moveToThread(worker->thread());
        worker->thread()->start();
    }
}


/************************************************
 *
 ************************************************/
void Render::render(int sheetNum)
{
    foreach (RenderWorker *worker, mWorkers)
    {
        if (!worker->isBussy())
        {
            QMetaObject::invokeMethod(worker, "render", Qt::QueuedConnection, Q_ARG(int, sheetNum));
            return;
        }
    }

    if (!mQueue.contains(sheetNum))
        mQueue.prepend(sheetNum);
}


/************************************************
 *
 ************************************************/
void Render::cancel(int sheetNum)
{
    mQueue.removeAll(sheetNum);
}


/************************************************
 *
 ************************************************/
void Render::workerFinished()
{
    if (!mQueue.isEmpty())
    {
        RenderWorker *worker = qobject_cast<RenderWorker*>(sender());
        QMetaObject::invokeMethod(worker, "render", Qt::QueuedConnection, Q_ARG(int, mQueue.takeFirst()));
    }
}
