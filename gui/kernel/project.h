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


#ifndef PROJECT_H
#define PROJECT_H


#include "job.h"
#include "printer.h"
#include "inputfile.h"

#include <QObject>
#include <QList>
#include <QStringList>
#include <QImage>

class Job;
class TmpPdfFile;
class Sheet;
class Layout;
class QTextStream;


class ProjectPage: public QObject
{
    Q_OBJECT
public:
    explicit ProjectPage(const ProjectPage *other);
    explicit ProjectPage(const InputFile &inputFile, int pageNum);
    ~ProjectPage();

    InputFile inputFile() const { return mInputFile; }
    int pageNum() const { return mPageNum; }

    int pdfObjectNum() const { return mPdfObjectNum; }
    void setPdfObjectNum(int id) { mPdfObjectNum = id; }

    QRectF rect() const { return mRect; }
    void setRect(const QRectF rect) { mRect = rect; }


    bool visible() const { return mVisible;}
    void setVisible(bool value);

private:
    InputFile mInputFile;
    int mPageNum;
    int mPdfObjectNum;
    QRectF mRect;
    bool mVisible;
};



class Project : public QObject
{
    Q_OBJECT
public:
    enum PagesType
    {
        OddPages,
        EvenPages,
        AllPages
    };

    enum PagesOrder
    {
        ForwardOrder,
        BackOrder
    };

    static Project* instance();

    const QList<Job*> *jobs() const { return &mJobs; }

    int pageCount() const { return mPages.count(); }
    ProjectPage *page(int index) const { return mPages.at(index); }


    QList<Sheet*> sheets() const { return mSheets; }
    int sheetCount() const { return mSheets.count(); }
    Sheet *sheet(int index) const { return mSheets[index]; }
    QList<Sheet*> selectSheets(PagesType pages = AllPages, PagesOrder order = ForwardOrder) const;


    int previewSheetCount() const { return mPreviewSheets.count(); }
    Sheet *previewSheet(int index) const { return mPreviewSheets[index]; }

    void writeDocument(const QList<Sheet *> &sheets, QIODevice *out);
    void writeDocument(const QList<Sheet*> &sheets, const QString &fileName);

    const Layout *layout() const { return mLayout; }
    bool doubleSided() const;

    Printer *printer() const { return mPrinter; }
    void setPrinter(Printer *value);

    bool error(const QString &message);

    QImage sheetImage(int sheetNum) const;

    void free();

public slots:
    void addFile(InputFile file);
    void addFiles(QList<InputFile> files);
    void removeJob(int index);
    void moveJob(int from, int to);
    void setLayout(const Layout *layout);
    void setDoubleSided(bool value);

signals:
    void changed();
    void progress(int progr, int all) const;
    void sheetImageChanged(int sheetNum);

private slots:
    void tmpFileMerged();
    void tmpFileProgress(int progr, int all) const;

private:
    explicit Project(QObject *parent = 0);
    ~Project();

    const Layout *mLayout;
    QList<ProjectPage*> mPages;
    QList<InputFile> mInputFiles;
    JobList mJobs;

    QList<Sheet*> mSheets;
    QList<Sheet*> mPreviewSheets;
    TmpPdfFile *mTmpFile;
    TmpPdfFile *mLastTmpFile;

    Printer *mPrinter;
    Printer mNullPrinter;
    bool mDoubleSided;

    void updateSheets();
    TmpPdfFile *createTmpPdfFile(QList<InputFile> files);
    void stopMerging();
};


#define project Project::instance()


#endif // PROJECT_H
