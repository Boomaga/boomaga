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

#include <atomic>
#include <QObject>
#include <QImage>
#include <QThread>
#include <QList>
#include <QPair>

namespace poppler
{
    class document;
}

class RenderWorker: public QObject
{
    Q_OBJECT
public:
    explicit RenderWorker(const QString &fileName, int resolution);
    virtual ~RenderWorker();

    // Read from the thread owning the Render, written from both that thread
    // (when a job is handed out) and the worker thread (when it finishes).
    bool isBusy() const { return mBusy; }
    void setBusy(bool busy) { mBusy = busy; }
    QThread *thread() { return &mThread; }

public slots:
    QImage renderSheet(int sheetNum);
    QImage renderPage(int sheetNum, const QRectF &pageRect, int pageNum);

signals:
    void sheetReady(QImage, int sheetNum);
    void pageReady(QImage, int pageNum);

private:
    int mSheetNum;
    int mResolution;
    std::atomic<bool> mBusy;
    QThread mThread;
    poppler::document *mPopplerDoc;
};


class Render : public QObject
{
    Q_OBJECT
public:
    // Upper bound rather than a fixed count: the worker count follows the
    // number of cores, but Poppler stops scaling well before a large machine
    // runs out of them. Measured over a 342 page document, 150 dpi, 12 cores / 24 threads:
    // 4 workers 3.0x, 8 workers 4.1x, 16 workers 3.7x, 24 workers 3.4x, while
    // peak memory kept growing (32 MB at 8 workers, 124 MB at 24).
    static constexpr int DefaultMaxThreadCount = 8;

    explicit Render(double resolution, QObject *parent = nullptr,
                    int maxThreadCount = DefaultMaxThreadCount);
    virtual ~Render();

    QString fileName() const { return mFileName; }

public slots:
    void setFileName(const QString &fileName);

    void renderSheet(int sheetNum);
    void cancelSheet(int sheetNum);

    void renderPage(int pageNum);
    void cancelPage(int pageNum);

signals:
    void sheetReady(QImage, int sheetNum);
    void pageReady(QImage, int pageNum);

private slots:
    void workerFinished();

private:
    QString mFileName;
    QVector<RenderWorker*> mWorkers;
    int mResolution;
    int mThreadCount;
    QList<QPair<int, bool> > mQueue;

    RenderWorker *idleWorker() const;

    // Return whether the job was really handed to the worker.
    bool startRenderSheet(RenderWorker *worker, int sheetNum);
    bool startRenderPage(RenderWorker *worker, int pageNum);

};

QImage toGrayscale(const QImage &srcImage);

#endif // RENDER_H
