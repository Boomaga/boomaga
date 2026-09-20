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
#include <poppler-document.h>
#include <poppler-page-renderer.h>
#include <poppler-page.h>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

#include "kernel/project.h"
#include "kernel/layout.h"


/************************************************

 ************************************************/
QImage doRenderSheet(poppler::document *doc, int sheetNum, double resolution)
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
    mBusy(false),
    mPopplerDoc(0)
{
    if (QFileInfo(fileName).exists())
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
QImage RenderWorker::renderSheet(int sheetNum)
{
    if (!mPopplerDoc)
    {
        // The worker was marked busy when the job was handed out, so release it
        // here as well - otherwise it would never be offered another one.
        mBusy = false;
        return QImage();
    }

    QImage img = doRenderSheet(mPopplerDoc, sheetNum, mResolution);

    // Clear the flag before emitting: the signal is delivered to the Render on
    // another thread, which may hand this worker its next job straight away.
    mBusy = false;
    emit sheetReady(img, sheetNum);
    return img;
}


/************************************************
 *
 ************************************************/
QImage RenderWorker::renderPage(int sheetNum, const QRectF &pageRect, int pageNum)
{
    if (!mPopplerDoc)
    {
        // See renderSheet().
        mBusy = false;
        return QImage();
    }

    QImage img = doRenderSheet(mPopplerDoc, sheetNum, mResolution);

    QSizeF printerSize =  project->printer()->paperRect().size();

    if (isLandscape(project->rotation()))
        printerSize.transpose();

    double scale = qMin(img.width() * 1.0 / printerSize.width(),
                        img.height() * 1.0 / printerSize.height());

    QSize size = QSize(pageRect.width()  * scale,
                       pageRect.height() * scale);

    if (isLandscape(project->rotation()))
        size.transpose();

    QRect rect(QPoint(0, 0), size);
    if (isLandscape(project->rotation()))
    {
        rect.moveRight(img.width() - pageRect.top()  * scale);
        rect.moveTop(pageRect.left() * scale);
    }
    else
    {
        rect.moveLeft(pageRect.left() * scale);
        rect.moveTop(pageRect.top()  * scale);
    }

    img = img.copy(rect);

    // Clear the flag before emitting, see renderSheet().
    mBusy = false;
    emit pageReady(img, pageNum);
    return img;
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
    mFileName = fileName;

    // The queued jobs refer to the sheets of the previous document.
    mQueue.clear();

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

        connect(worker, SIGNAL(sheetReady(QImage,int)),
                this, SIGNAL(sheetReady(QImage,int)));

        connect(worker, SIGNAL(sheetReady(QImage,int)),
                this, SLOT(workerFinished()));

        connect(worker, SIGNAL(pageReady(QImage,int)),
                this, SIGNAL(pageReady(QImage,int)));

        connect(worker, SIGNAL(pageReady(QImage,int)),
                this, SLOT(workerFinished()));


        worker->moveToThread(worker->thread());
        worker->thread()->start();
    }
}


/************************************************
 *
 ************************************************/
void Render::renderSheet(int sheetNum)
{
    foreach (RenderWorker *worker, mWorkers)
    {
        if (!worker->isBusy())
        {
            startRenderSheet(worker, sheetNum);
            return;
        }
    }

    QPair<int,bool> job(sheetNum, false);
    if (!mQueue.contains(job))
        mQueue.prepend(job);
}


/************************************************
 *
 ************************************************/
void Render::renderPage(int pageNum)
{
    foreach (RenderWorker *worker, mWorkers)
    {
        if (!worker->isBusy())
        {
            startRenderPage(worker, pageNum);
            return;
        }
    }

    QPair<int,bool> job(pageNum, true);
    if (!mQueue.contains(job))
        mQueue.prepend(job);

}


/************************************************
 *
 ************************************************/
void Render::cancelSheet(int sheetNum)
{
    mQueue.removeAll(QPair<int,bool>(sheetNum, false));
}


/************************************************
 *
 ************************************************/
void Render::cancelPage(int pageNum)
{
    mQueue.removeAll(QPair<int,bool>(pageNum, true));
}


/************************************************
 *
 ************************************************/
void Render::workerFinished()
{
    RenderWorker *worker = qobject_cast<RenderWorker*>(sender());
    if (!worker)
        return;

    // A queued job may have become unrenderable while it waited, so keep
    // taking jobs until one is really handed out. Stopping at the first
    // unrenderable one would leave this worker without a job, and nothing
    // would ever trigger it again to drain the rest of the queue.
    while (!mQueue.isEmpty())
    {
        QPair<int,bool> job = mQueue.takeFirst();

        bool started = job.second ? startRenderPage(worker, job.first)
                                  : startRenderSheet(worker, job.first);
        if (started)
            return;
    }
}


/************************************************
 *
 ************************************************/
bool Render::startRenderSheet(RenderWorker *worker, int sheetNum)
{
    worker->setBusy(true);
    QMetaObject::invokeMethod(worker,
                              "renderSheet",
                              Qt::QueuedConnection,
                              Q_ARG(int, sheetNum));
    return true;
}


/************************************************
 *
 ************************************************/
bool Render::startRenderPage(RenderWorker *worker, int pageNum)
{
    int sheetNum = project->previewSheets().indexOfPage(pageNum);
    if (sheetNum < 0)
        return false;

    Sheet *sheet = project->previewSheets().at(sheetNum);
    ProjectPage *page = project->page(pageNum);

    int pageOnSheet = -1;
    for (int i = 0; i<sheet->count(); ++i)
    {
        if (sheet->page(i) == page)
            pageOnSheet = i;
    }

    if (pageOnSheet < 0)
        return false;

    TransformSpec spec = project->layout()->transformSpec(sheet, pageOnSheet, project->rotation());

    worker->setBusy(true);
    QMetaObject::invokeMethod(worker,
                              "renderPage",
                              Qt::QueuedConnection,
                              Q_ARG(int, sheetNum),
                              Q_ARG(QRectF, spec.rect),
                              Q_ARG(int, pageNum));
    return true;
}


/************************************************

 ************************************************/
QImage toGrayscale(const QImage &srcImage)
{
    // Convert to 32bit pixel format
    QImage dstImage = srcImage.convertToFormat(srcImage.hasAlphaChannel() ?
                                                   QImage::Format_ARGB32 : QImage::Format_RGB32);

    unsigned int *data = (unsigned int*)dstImage.bits();
    int pixelCount = dstImage.width() * dstImage.height();

    // Convert each pixel to grayscale
    for(int i = 0; i < pixelCount; ++i)
    {
        int val = qGray(*data);
        *data = qRgba(val, val, val, qAlpha(*data));
        ++data;
    }

    return dstImage;
}
