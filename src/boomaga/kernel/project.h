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
#include "projectpage.h"

#include <QObject>
#include <QList>
#include <QStringList>
#include <QImage>
#include <QPointer>

class Job;
class TmpPdfFile;
class Layout;
#include "sheet.h"

class MetaData
{
public:
    MetaData() {}

    QString author() const { return mAuthor; }
    void setAuthor(const QString &value) { mAuthor = value;}

    QString title() const { return mTitle; }
    void setTitle(const QString &value) { mTitle = value; }

    QString subject() const { return mSubject; }
    void setSubject(const QString &value) { mSubject = value; }

    QString keywords() const { return mKeywords; }
    void setKeywords(const QString &value) { mKeywords = value; }

    QByteArray asPDFDict() const;

#if 0
    //QByteArray asXMP() const;
#endif

private:
    QString mTitle;         // The document’s title.
    QString mAuthor;        // The name of the person who created the document.
    QString mSubject;       // The subject of the document.
    QString mKeywords;      // Keywords associated with the document.

#if 0
    void xmp(QByteArray &out, const QString &format) const;
    void xmp(QByteArray &out, const QString &format, const QString &value) const;
#endif

    void addDictItem(QByteArray &out, const QString &key, const QString &value) const;
    void addDictItem(QByteArray &out, const QString &key, const QDateTime &value) const;
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

    const JobList *jobs() const { return &mJobs; }

    int pageCount() const { return mPages.count(); }
    ProjectPage *page(int index) const { return mPages.at(index); }
    QList<ProjectPage*> pages() const { return mPages; }

    int sheetCount() const { return mSheetCount; }
    QList<Sheet*> selectSheets(PagesType pages = AllPages, PagesOrder order = ForwardOrder) const;

    SheetList previewSheets() const { return mPreviewSheets; }
    int previewSheetCount() const { return mPreviewSheets.count(); }
    Sheet *previewSheet(int index) const { return mPreviewSheets[index]; }

    bool writeDocument(const QList<Sheet*> &sheets, QIODevice *out);
    bool writeDocument(const QList<Sheet*> &sheets, const QString &fileName);

    const Layout *layout() const { return mLayout; }
    bool doubleSided() const;

    Printer *printer() const { return mPrinter; }
    void setPrinterProfile(Printer *printer, int profile, bool update = true);

    MetaData metaData() const { return mMetaData; }
    void setMetadata(const MetaData &value) { mMetaData = value; }

    void free();

    void save(const QString &fileName);
    JobList load(const QString &fileName, const QString &options = "");
    JobList load(const QStringList &fileNames, const QString &options = "");

    Rotation rotation() const { return mRotation; }

public:
    ProjectPage *currentPage() const { return mCurrentPage; }
    int currentPageNum() const;

    int previewPageNum(int pageNum) const;
    int previewPageCount() const;
    int currentPreviewPage() const;

    void deletePage(ProjectPage *page);
    void undoDeletePage(ProjectPage *page);

    void deletePagesEnd(ProjectPage *page);
    void insertBlankPageBefore(ProjectPage *page);
    void insertBlankPageAfter(ProjectPage *page);

    ProjectPage *prevVisiblePage(ProjectPage *current) const;
    ProjectPage *nextVisiblePage(ProjectPage *current) const;


public slots:
    void setCurrentPage(ProjectPage *page);
    void setCurrentPage(int pageNum);
    void prevPage();
    void nextPage();

public:
    Sheet *currentSheet() const;
    int currentSheetNum() const;

public slots:
    void setCurrentSheet(int sheetNum);
    void prevSheet();
    void nextSheet();


public slots:
    bool error(const QString &message) const;

    void addJob(const Job &job);
    void addJobs(const JobList &jobs);
    void removeJob(int index);
    void moveJob(int from, int to);
    void setLayout(const Layout *layout);
    void setDoubleSided(bool value);
    void update();


signals:
    void changed();
    void progress(int progr, int all) const;
    void tmpFileRenamed(const QString &mTmpFileName);
    void currentPageChanged(ProjectPage *page);
    void currentPageChanged(int page);
    void currentSheetChanged(Sheet *sheet);
    void currentSheetChanged(int sheet);

protected:
    Rotation calcRotation(const QList<ProjectPage *> &pages, const Layout *layout) const;

private slots:
    void tmpFileMerged();
    void tmpFileProgress(int progr, int all) const;

private:
    explicit Project(QObject *parent = 0);
    ~Project();

    const Layout *mLayout;
    QList<ProjectPage*> mPages;
    QPointer<ProjectPage> mCurrentPage;
    Sheet *mCurrentSheet;
    JobList mJobs;

    int mSheetCount;
    SheetList mPreviewSheets;
    TmpPdfFile *mTmpFile;
    TmpPdfFile *mLastTmpFile;

    Printer mNullPrinter;
    Printer *mPrinter;
    bool mDoubleSided;

    MetaData mMetaData;
    Rotation mRotation;

    TmpPdfFile *createTmpPdfFile();
    void stopMerging();
};


#define project Project::instance()

#endif // PROJECT_H
