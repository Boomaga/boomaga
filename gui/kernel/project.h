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

class ProjectPage
{
public:
    explicit ProjectPage();
    explicit ProjectPage(const ProjectPage *other);
    explicit ProjectPage(const InputFile &inputFile, int pageNum);
    ~ProjectPage();

    InputFile inputFile() const { return mInputFile; }
    int pageNum() const { return mPageNum; }

    int pdfObjectNum() const { return mPdfObjectNum; }
    void setPdfObjectNum(int id) { mPdfObjectNum = id; }

    virtual QRectF rect() const;
    void setRect(const QRectF rect) { mRect = rect; }


    bool visible() const { return mVisible;}
    void setVisible(bool value);
    void hide() { setVisible(false); }
    void show() { setVisible(true); }

    bool isBlankPage();
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

    const JobList *jobs() const { return &mJobs; }

    int pageCount() const { return mPages.count(); }
    ProjectPage *page(int index) const { return mPages.at(index); }


    QList<Sheet*> sheets() const { return mSheets; }
    int sheetCount() const { return mSheets.count(); }
    Sheet *sheet(int index) const { return mSheets[index]; }
    QList<Sheet*> selectSheets(PagesType pages = AllPages, PagesOrder order = ForwardOrder) const;


    int previewSheetCount() const { return mPreviewSheets.count(); }
    Sheet *previewSheet(int index) const { return mPreviewSheets[index]; }

    bool writeDocument(const QList<Sheet *> &sheets, QIODevice *out);
    bool writeDocument(const QList<Sheet*> &sheets, const QString &fileName);

    const Layout *layout() const { return mLayout; }
    bool doubleSided() const;

    Printer *printer() const { return mPrinter; }
    void setPrinter(Printer *value);

    QImage sheetImage(int sheetNum) const;

    MetaData metaData() const { return mMetaData; }
    void setMetadata(const MetaData &value) { mMetaData = value; }

    void free();

    void save(const QString &fileName);
    void load(const QString &fileName, const QString title = "", bool autoRemove = false);
    void load(const QStringList &fileNames, bool autoRemove = false);
    void load(const QStringList &fileNames, const QStringList &titles, bool autoRemove = false);

public slots:
    bool error(const QString &message);

    void addJob(Job job);
    void addJobs(JobList jobs);
    void removeJob(int index);
    void moveJob(int from, int to);
    void setLayout(const Layout *layout);
    void setDoubleSided(bool value);
    void update();

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
    JobList mJobs;

    QList<Sheet*> mSheets;
    QList<Sheet*> mPreviewSheets;
    TmpPdfFile *mTmpFile;
    TmpPdfFile *mLastTmpFile;

    Printer *mPrinter;
    Printer mNullPrinter;
    bool mDoubleSided;

    MetaData mMetaData;

    TmpPdfFile *createTmpPdfFile();
    void stopMerging();
};


#define project Project::instance()

#endif // PROJECT_H
