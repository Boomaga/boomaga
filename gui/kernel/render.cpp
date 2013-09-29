#include <QCoreApplication>

#include "render.h"
#include "kernel/project.h"
#include <poppler/qt4/poppler-qt4.h>
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
    delete [] mData;
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
    delete buf;
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
QImage renderSheet(Poppler::Document *doc, int sheetNum, double res)
{
    Poppler::Page *page = doc->page(sheetNum);
    if (page)
    {
        Poppler::Page::Rotation rotate;
        //if (project->previewSheet(sheetNum)->hints().testFlag(Sheet::HintLandscapePreview))
        //    rotate = Poppler::Page::Rotate90;
        //else
            rotate = Poppler::Page::Rotate0;

        QImage img = page->renderToImage(res, res, -1, -1, -1, -1, rotate);

        delete page;
        return img;
    }

    return QImage();
}


/************************************************

 ************************************************/
void RenderThread::run()
{
    mStopped = false;

    //qDebug() << "CREATE thread doc ************************";
    Poppler::Document *doc = Poppler::Document::load(mPdfFileName);
    if (!doc)
        return;

    doc->setRenderHint(Poppler::Document::Antialiasing, true);
    doc->setRenderHint(Poppler::Document::TextAntialiasing, true);

    int cnt = doc->numPages();
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
    mLoResDoc = Poppler::Document::load(mPdfFileName);
    mLoResDoc->setRenderHint(Poppler::Document::Antialiasing, true);
    mLoResDoc->setRenderHint(Poppler::Document::TextAntialiasing, true);


    mImages.resize(mLoResDoc->numPages());

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
    CompressedImage *ci = mImages[sheetNum];
    if (ci)
    {
        return mImages[sheetNum]->image();
    }

    emit setSheetNumHint(sheetNum);

    if (mLoResDoc)
    {
        return renderSheet(mLoResDoc, sheetNum, 60);
    }

    return QImage();
}


/************************************************

 ************************************************/
void Render::sheetRenderUpdated(CompressedImage *image, int sheetNum)
{
    mImages[sheetNum] = image;
    emit imageChanged(sheetNum);
}



