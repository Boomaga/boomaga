#ifndef RENDER_H
#define RENDER_H

#include <QObject>
#include <QImage>
#include <QThread>
#include <QMutex>

namespace Poppler
{
    class Document;
}

class CompressedImage;

class RenderThread: public QThread
{
    Q_OBJECT
public:
    RenderThread(const QString &pdfFileName);
    ~RenderThread();

public slots:
    void setSheetNumHint(int sheetNum);

    void stop();

protected:
    void run();

signals:
    void sheetReady(CompressedImage *image, int sheetNum);

private:
    QString mPdfFileName;
    volatile int mNextSheet;
    QMutex mNextSheetMutex;
    volatile bool mStopped;
};



class Render : public QObject
{
    Q_OBJECT
public:
    explicit Render(const QString &pdfFileName, QObject *parent = 0);
    virtual ~Render();

    QImage image(int sheetNum) const;

    void start();
    void stop();
signals:
    void imageChanged(int sheetNum);
    void setSheetNumHint(int sheetNum) const;

private slots:
    void sheetRenderUpdated(CompressedImage *image, int sheetNum);

private:
    QString mPdfFileName;
    RenderThread mThread;
    QVector<CompressedImage*> mImages;
    Poppler::Document *mLoResDoc;
};



#endif // RENDER_H
