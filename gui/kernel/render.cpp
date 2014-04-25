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


#include <QCoreApplication>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <poppler/cpp/poppler-page.h>
#include "render.h"
#include "kernel/project.h"
#include "kernel/sheet.h"
#include <snappy.h>
#include <QDebug>
#include <QMutexLocker>

class CompressedImage
{
public:
    CompressedImage(const QImage &image);
    ~CompressedImage();

    QImage image();

    int width() const { return mWidth; }
    int height() const { return mHeight; }
    int bytesPerLine() const { return mBytesPerLine; }

private:
    char *mData;
    size_t mDataLen;
    int mWidth;
    int mHeight;
    int mBytesPerLine;
    QImage::Format mFormat;
};


/************************************************

 ************************************************/
CompressedImage::CompressedImage(const QImage &image):
    mWidth(image.width()),
    mHeight(image.height()),
    mBytesPerLine(image.bytesPerLine()),
    mFormat(image.format())
{
    mData = new char[snappy::MaxCompressedLength(mBytesPerLine * mHeight)];
    snappy::RawCompress((const char*)image.constBits(), mBytesPerLine * mHeight, mData, &mDataLen);
}


/************************************************

 ************************************************/
CompressedImage::~CompressedImage()
{
    delete[] mData;
}


/************************************************

 ************************************************/
QImage CompressedImage::image()
{
    size_t bufLen;
    snappy::GetUncompressedLength(mData, mDataLen, &bufLen);

    uchar *buf = new uchar[bufLen];
    snappy::RawUncompress(mData, mDataLen, (char *)buf);

    // I use convert for deep copy of the QImage data.
    QImage img = QImage(buf, mWidth, mHeight, mBytesPerLine, mFormat).copy();
    delete[] buf;
    return img;
}



/************************************************

 ************************************************/
RenderThread::RenderThread(const QString &pdfFileName):
    QThread(),
    mPdfFileName(pdfFileName),
    mNextSheet(0),
    mStopped(true)
{
}


/************************************************

 ************************************************/
RenderThread::~RenderThread()
{
}


/************************************************

 ************************************************/
void RenderThread::setSheetNumHint(int sheetNum)
{
    QMutexLocker locker(&mNextSheetMutex);
    mNextSheet = sheetNum;
}

void RenderThread::stop()
{
    mStopped = true;
    this->wait();
}


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

 ************************************************/
void RenderThread::run()
{
    mStopped = false;

    //qDebug() << "CREATE thread doc ************************";
    poppler::document *doc = poppler::document::load_from_file(mPdfFileName.toLocal8Bit().data());
    if (!doc)
        return;


    int cnt = doc->pages();
    QVector<bool> ready(cnt, false);

    int left=cnt;
    while (left > 0)
    {
        int n;
        {
            QMutexLocker locker(&mNextSheetMutex);

            if (mStopped)
                break;

            if (mNextSheet >= cnt)
                mNextSheet = 0;

            if (ready[mNextSheet])
            {
                mNextSheet++;
                continue;
            }


            n = mNextSheet;
            ready[mNextSheet] = true;
            mNextSheet++;
            left--;
        }

        QImage img = renderSheet(doc, n, 150);
        if (!img.isNull())
        {
            CompressedImage *res = new CompressedImage(img);
            emit sheetReady(res, n);
        }
        else
        {
            emit sheetReady(0, n);
        }

    }
    //qDebug() << "DELETE thread doc ************************";
    delete doc;
}


/************************************************

 ************************************************/
Render::Render(const QString &pdfFileName, QObject *parent) :
    QObject(parent),
    mPdfFileName(pdfFileName),
    mThread(pdfFileName)
{
    mLoResDoc = poppler::document::load_from_file(mPdfFileName.toLocal8Bit().data());

    mImages.resize(mLoResDoc->pages());

    connect(&mThread, SIGNAL(sheetReady(CompressedImage*,int)),
            this, SLOT(sheetRenderUpdated(CompressedImage*,int)));

    connect(this, SIGNAL(setSheetNumHint(int)),
            &mThread, SLOT(setSheetNumHint(int)));
}


/************************************************

 ************************************************/
Render::~Render()
{
    mThread.stop();
    mThread.wait();
    delete mLoResDoc;
    qDeleteAll(mImages);
}


/************************************************

 ************************************************/
void Render::start()
{
    mThread.start();
}


/************************************************

 ************************************************/
void Render::stop()
{
    mThread.stop();
    mThread.wait();
}


/************************************************

 ************************************************/
QImage Render::image(int sheetNum) const
{
    if (sheetNum < mImages.count())
    {
        CompressedImage *ci = mImages[sheetNum];
        if (ci)
            return mImages[sheetNum]->image();
    }

    emit setSheetNumHint(sheetNum);

    if (mLoResDoc)
        return renderSheet(mLoResDoc, sheetNum, 60);

    return QImage();
}


/************************************************

 ************************************************/
void Render::sheetRenderUpdated(CompressedImage *image, int sheetNum)
{
    mImages[sheetNum] = image;
    emit imageChanged(sheetNum);
}



