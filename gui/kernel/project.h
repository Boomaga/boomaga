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



#include "printer.h"

#include <QObject>
#include <QList>
#include <QStringList>
#include <QImage>

class InputFile;
class TmpPdfFile;
class Sheet;
class Layout;
class QTextStream;

class Job
{
public:
    Job(const QString &fileName, const QString &title="", bool autoRemove=false);
    Job(const Job &other);
    explicit Job(const InputFile *inputFile);

    QString fileName() const { return mFileName; }
    QString title() const { return mTitle; }
    bool autoRemove() const { return mAutoRemove; }

private:
    QString mFileName;
    QString mTitle;
    bool    mAutoRemove;
};

class Jobs: public QList<Job>
{
public:
    Jobs() {}

    int indexOf(const QString &fileName);

};

class ProjectPage: public QObject
{
    Q_OBJECT
public:
    explicit ProjectPage(InputFile *inputFile, int pageNum);
    ~ProjectPage();

    InputFile *inputFile() const { return mInputFile; }
    int pageNum() const { return mPageNum; }

    int pdfObjectNum() const { return mPdfObjectNum; }
    void setPdfObjectNum(int id) { mPdfObjectNum = id; }

    QRectF rect() const { return mRect; }
    void setRect(const QRectF rect) { mRect = rect; }


    bool visible() const { return mVisible;}
    void setVisible(bool value);

private:
    InputFile *mInputFile;
    int mPageNum;
    int mPdfObjectNum;
    QRectF mRect;
    int num(int inc=0);
    bool mVisible;
};

class ProjectPageList: public QList<ProjectPage*>
{
public:
    ProjectPageList(): QList<ProjectPage*>() {}

    void removeFile(const InputFile *file);
    void moveFile(const InputFile *file, const InputFile *before);
    void addFile(const InputFile *file);
    int indexOfFirstPage(const InputFile *file);
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

    const QList<InputFile*> *inputFiles() const { return &mFiles; }

    int filesCount() const { return mFiles.count(); }
    InputFile *file(int index) const { return mFiles[index]; }

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

    Printer *printer() const { return mPrinter; }
    void setPrinter(Printer *value);

    bool error(const QString &message);

    QImage sheetImage(int sheetNum) const;

    void free();

public slots:
    void addFile(Job job);
    void addFiles(QList<Job> jobs);
    void removeFile(int index);
    void moveFile(int from, int to);
    void setLayout(const Layout *layout);

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
    ProjectPageList mPages;
    QList<InputFile*> mFiles;
    Jobs mJobs;
    QList<Sheet*> mSheets;
    QList<Sheet*> mPreviewSheets;
    TmpPdfFile *mTmpFile;
    TmpPdfFile *mLastTmpFile;

    Printer *mPrinter;
    Printer mNullPrinter;

    void updateSheets();
    TmpPdfFile *createTmpPdfFile(QList<Job> jobs);
    void stopMerging();
};


#define project Project::instance()

QDebug operator<<(QDebug dbg, const Job &job);

#endif // PROJECT_H
