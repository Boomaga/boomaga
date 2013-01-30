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


#include "psrender.h"
#include "kernel/pssheet.h"

#include <ghostscript/gdevdsp.h>
#include <ghostscript/iapi.h>
#include <ghostscript/ierrors.h>

#include "kernel/psproject.h"
#include <QCoreApplication>
#include <QDebug>


class PsRenderPrivate
{
public:
    PsRenderPrivate(PsRender *parent);
    ~PsRenderPrivate();
    void newImage(const uchar *data, int width, int height, int bytesPerLine);
    void clear();
    void refresh();

    void run();
    QList<QByteArray> gsArgs();
    bool gsRunString(const QByteArray &data);

    static int emptyCallback(void *handle, void *device);
    static int emptyCallback(void *handle, void *device, int width, int height, int raster, unsigned int format);

    static int openCallback(void *handle, void *device);
    static int precloseCallback(void *handle, void *device);
    static int closeCallback(void *handle, void *device);
    static int presizeCallback(void *handle, void *device, int width, int height, int raster, unsigned int format);
    static int sizeCallback(void *handle, void *device, int width, int height, int raster, unsigned int format, unsigned char *pimage);
    static int pageCallback(void *handle, void *device, int copies, int flush);
    static int syncCallback(void *handle, void *device);


    QList<const PsSheet*> mSheets;
    QList<QImage> mImages;
    PsProject *mProject;
    void  *mGsInstance;
    QByteArray mInputData;

    unsigned char *mGsImage; /*! Image buffer we received from Ghostscript library */
    int mGsImageHeight;
    int mGsImageWidth;
    int mGsImageRowLength;
    display_callback mGsDisplayCallback;

private:
    PsRender* const q_ptr;
    Q_DECLARE_PUBLIC(PsRender)
};


/************************************************

 ************************************************/
int PsRenderPrivate::emptyCallback(void *handle, void *device)
{
    return 0;
}

/************************************************

 ************************************************/
int PsRenderPrivate::emptyCallback(void *handle, void *device, int width, int height, int raster, unsigned int format)
{
    return 0;
}


/************************************************
 Device has been resized.
 New pointer to raster returned in pimage
 ************************************************/
int PsRenderPrivate::sizeCallback(void *handle, void *device, int width, int height, int raster, unsigned int format, unsigned char *pimage)
{
    if (!handle)
        return 0;

    PsRenderPrivate *render = static_cast<PsRenderPrivate*>(handle);
    render->mGsImage = pimage;
    render->mGsImageWidth = width;
    render->mGsImageHeight = height;
    render->mGsImageRowLength = raster;
    return 0;
}


/************************************************
 showpage
 If you want to pause on showpage, then don't return immediately
 ************************************************/
int PsRenderPrivate::pageCallback (void *handle, void * device, int copies, int flush)
{
    if (!handle)
        return 0;

    PsRenderPrivate *render = static_cast<PsRenderPrivate*>(handle);
    render->newImage(render->mGsImage, render->mGsImageWidth, render->mGsImageHeight, render->mGsImageRowLength);
    return 0;
}


/************************************************

 ************************************************/
void PsRenderPrivate::newImage(const uchar *data, int width, int height, int bytesPerLine)
{
    Q_Q(PsRender);
    QImage img(data, width, height, bytesPerLine, QImage::Format_RGB32);
    // I use convert for deep copy of the QImage data.
    mImages.append(img.convertToFormat(QImage::Format_ARGB32));
    emit q->changed(mImages.count() - 1);
    QCoreApplication::processEvents();
}


/************************************************

 ************************************************/
PsRenderPrivate::PsRenderPrivate(PsRender *parent):
    q_ptr(parent),
    mGsInstance(0)
{

    // Callback structure for "display" device ..
    mGsDisplayCallback.size = sizeof (display_callback);
    mGsDisplayCallback.version_major    = DISPLAY_VERSION_MAJOR;
    mGsDisplayCallback.version_minor    = DISPLAY_VERSION_MINOR;
    mGsDisplayCallback.display_open     = PsRenderPrivate::emptyCallback;
    mGsDisplayCallback.display_preclose = PsRenderPrivate::emptyCallback;
    mGsDisplayCallback.display_close    = PsRenderPrivate::emptyCallback;
    mGsDisplayCallback.display_presize  = PsRenderPrivate::emptyCallback;
    mGsDisplayCallback.display_size     = PsRenderPrivate::sizeCallback;
    mGsDisplayCallback.display_sync     = PsRenderPrivate::emptyCallback;
    mGsDisplayCallback.display_page     = PsRenderPrivate::pageCallback;
    mGsDisplayCallback.display_update   = NULL;
    mGsDisplayCallback.display_memalloc = NULL;
    mGsDisplayCallback.display_memfree  = NULL;
    mGsDisplayCallback.display_separation = NULL;
}


/************************************************mGsDisplayCallback

 ************************************************/
PsRenderPrivate::~PsRenderPrivate()
{
}


/************************************************

 ************************************************/
void PsRenderPrivate::clear()
{
    mSheets.clear();
    mImages.clear();
}


/************************************************

 ************************************************/
PsRender::PsRender(PsProject *project, QObject *parent) :
    QObject(parent),
    d_ptr(new PsRenderPrivate(this))
{
    d_ptr->mProject = project;
}


/************************************************

 ************************************************/
PsRender::~PsRender()
{
    delete d_ptr;
}


/************************************************

 ************************************************/
int PsRender::sheetCount() const
{
    Q_D(const PsRender);
    return d->mImages.count();
}


/************************************************

 ************************************************/
const PsSheet *PsRender::sheet(int index) const
{
    Q_D(const PsRender);
    return d->mSheets[index];
}


/************************************************

 ************************************************/
QImage PsRender::image(int index)
{
    Q_D(PsRender);
    return d->mImages[index];
}


/************************************************

 ************************************************/
void PsRender::refresh()
{
    Q_D(PsRender);
    d->refresh();
}

/************************************************

 ************************************************/
void PsRenderPrivate::refresh()
{
    Q_Q(PsRender);
    emit q->started();

    clear();
    for (int i=0; i< mProject->previewSheetCount(); ++i)
        mSheets.append(mProject->previewSheet(i));

    mInputData.clear();
    QTextStream *stream = new QTextStream(&mInputData, QIODevice::ReadWrite);

    mProject->writeDocument(mSheets, stream);
    delete stream;

    run();

    emit q->finished();
}


/************************************************
 See:
 http://ghostscript.com/doc/current/Use.htm
 http://ghostscript.com/doc/current/Devices.htm
 ************************************************/
QList<QByteArray> PsRenderPrivate::gsArgs()
{
    QList<QByteArray> args;
    args << "render";
    args << "-dMaxBitmap=10000000";
    args << "-dDELAYSAFER";
    args << "-dNOPAUSE";
    args << "-dNOPAGEPROMPT";
    args << "-dQUIET";
    args << "-dBATCH";

    args << "-dNOTRANSPARENCY";
    args << "-dTextAlphaBits=4";
    args << "-dGraphicsAlphaBits=4";

    args << QString("-dDisplayFormat=%1").arg(
                DISPLAY_COLORS_RGB |
                DISPLAY_DEPTH_8 |
                DISPLAY_ROW_ALIGN_DEFAULT |
                DISPLAY_ALPHA_NONE |
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
                DISPLAY_UNUSED_FIRST |
                DISPLAY_BIGENDIAN |
#else
                DISPLAY_UNUSED_LAST |
                DISPLAY_LITTLEENDIAN |
#endif
                DISPLAY_TOPFIRST).toAscii();

    args << QString("-sDisplayHandle=16#%1").arg((long long unsigned int)this, 0, 16).toAscii();
    args << "-sDEVICE=display";

    // Quality vs speed
    //args << QString("-r%1x%2").arg(200).toLatin1();
    args << QString("-r%1x%2").arg(150).toLatin1();

    //"-g%dx%d", sWidth, sHeight); // Size of page
    //"-r%fx%f", sWidth * mXDpi    , scale * mYDpi  );
    //"-r%fx%f", scale * mXDpi, scale * mYDpi);

    //if (mUsePlatformFonts == FALSE)
    //    sprintf(args[arg++], "-dNOPLATFONTS");

    //"-dDEVICEWIDTHPOINTS=%d",  width); Размеры страницы, перекрывают размеры прописанные в PS.
    //"-dDEVICEHEIGHTPOINTS=%d", height);
    //"-dDEVICEWIDTH=%d",  width);
    //"-dDEVICEHEIGHT=%d", height);
    //args << "-sPAPERSIZE=a4";
    //args << "-dFIXEDMEDIA";

    //qDebug() << args;
    return args;
}


/************************************************

 ************************************************/
void PsRenderPrivate::run()
{
    Q_Q(PsRender);

    int gsRes;
    // Create ...................................
    gsRes = gsapi_new_instance(&mGsInstance, 0);
    if (gsRes < 0)
    {
        qWarning() << "Error gsapi_new_instance" << gsRes;
        return;
    }


    // Set display callback .....................
    gsRes = gsapi_set_display_callback(mGsInstance, &mGsDisplayCallback);
    if (gsRes < 0)
    {
        qWarning() << "Error gsapi_set_display_callback" << gsRes;
        gsapi_delete_instance(mGsInstance);
        return;
    }


    // Args .....................................
    QList<QByteArray> args = gsArgs();
    int argc = args.count();
    char *argv[100];
    for (int i = 0; i < argc; ++i)
    {
        argv[i] = args[i].data();
    }


    // Init .....................................
    gsRes = gsapi_init_with_args(mGsInstance, argc, argv);
    if (gsRes < -100)
    {
        qWarning() << "Error gsapi_init_with_args" << gsRes;
        gsapi_delete_instance(mGsInstance);
        return;
    }


    // Run ......................................
    int error;
    gsRes = gsapi_run_string_begin(mGsInstance, 0, &error);
    if (gsRes < -100)
    {
        qWarning() << "Error gsapi_run_string_begin" << gsRes;
        gsapi_exit(mGsInstance);
        gsapi_delete_instance(mGsInstance);
        return;
    }

    const char *buff = mInputData.constData();
    const char *end = mInputData.constData() + mInputData.size();
    while (buff < end)
    {
        int len = qMin(65535, (int)(end - buff));
        int gsRes = gsapi_run_string_continue(mGsInstance, buff, len, 0, &error);
        if (gsRes != 0 && gsRes != e_NeedInput)
            break;
        buff += len;
    }

    gsapi_run_string_end(mGsInstance, 0, &error);
    gsRes = gsapi_exit(mGsInstance);
    gsapi_delete_instance(mGsInstance);
}


/************************************************

 ************************************************/
bool PsRenderPrivate::gsRunString(const QByteArray &data)
{
    int error;
    int gsRes = gsapi_run_string_continue(mGsInstance, data.data(), data.length(), 0, &error);
    if (gsRes == 0 || gsRes == e_NeedInput)
        return true;

    return false;
}


