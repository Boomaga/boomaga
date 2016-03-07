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


#include <sys/time.h>
#include <time.h>


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
RenderWorker::RenderWorker(const QString &fileName):
    QObject(),
    mSheetNum(0)
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
void RenderWorker::render(int sheetNum, int resolution)
{
    mSheetNum = sheetNum;
    QMetaObject::invokeMethod(this, "doRender", Qt::QueuedConnection, Q_ARG(int, sheetNum), Q_ARG(int, resolution));
}


/************************************************
 *
 ************************************************/
void RenderWorker::doRender(int sheetNum, int resolution) const
{
    if (sheetNum == mSheetNum)
    {
        QImage img = renderSheet(mPopplerDoc, sheetNum, resolution);
        emit imageReady(img, sheetNum);
    }
}



/************************************************
 *
 ************************************************/
Render::Render(QObject *parent):
    QObject(parent),
    mWorker(0)
{

}


/************************************************
 *
 ************************************************/
Render::~Render()
{
    mThread.quit();
    mThread.wait();
    delete mWorker;
}


/************************************************
 *
 ************************************************/
void Render::setFileName(const QString &fileName)
{
    mThread.quit();
    mThread.wait();
    delete mWorker;

    mWorker = new RenderWorker(fileName);
    connect(mWorker, SIGNAL(imageReady(QImage,int)),
            this, SIGNAL(imageReady(QImage,int)));

    mWorker->moveToThread(&mThread);
    mThread.start();
}


/************************************************
 *
 ************************************************/
void Render::render(int sheetNum, int resolution) const
{
    QMetaObject::invokeMethod(mWorker, "render", Qt::QueuedConnection, Q_ARG(int, sheetNum), Q_ARG(int, resolution));
}
