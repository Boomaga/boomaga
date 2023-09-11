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


#include "kernel/project.h"
#include "settings.h"
#include "job.h"

#include "tmppdffile.h"
#include "sheet.h"
#include "layout.h"
#include "iofiles/infile.h"
#include "iofiles/boofile.h"

#include <unistd.h>
#include <fstream>
#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QDateTime>
#include <QUuid>
#include <QUrl>

#define META_SIZE (4 * 1024)

using namespace  std;

class ProjectState
{
public:
    explicit ProjectState(const Project *p):
        mProject(p),
        mCurrentPage(p->currentPage()),
        mCurrentSheet(p->currentSheet())
    {
    }

    const ProjectPage *currentPage() const { return mCurrentPage; }
    const Sheet *currentSheet() const { return mCurrentSheet; }

    bool currentPageChanged() const { return mCurrentPage != mProject->currentPage(); }
    bool currentSheetChanged() const { return mCurrentSheet != mProject->currentSheet(); }

private:
    const Project *mProject;
    const ProjectPage *mCurrentPage;
    const Sheet *mCurrentSheet;
};


/************************************************

 ************************************************/
Project::Project(QObject *parent) :
    QObject(parent),
    mLayout(0),
    mCurrentPage(0),
    mCurrentSheet(0),
    mSheetCount(0),
    mTmpFile(0),
    mLastTmpFile(0),
    mNullPrinter("Fake"),
    mPrinter(&mNullPrinter),
    mDoubleSided(true),
    mRotation(NoRotate)
{
}


/************************************************

 ************************************************/
Project::~Project()
{
    free();
}


/************************************************

 ************************************************/
void Project::free()
{
    for (const Job &job: mJobs)
    {
        if (job.fileName().endsWith(AUTOREMOVE_EXT))
        {
            QFile(job.fileName()).remove();
        }
    }

    mJobs.clear();
    delete mTmpFile;
}


/************************************************

 ************************************************/
TmpPdfFile *Project::createTmpPdfFile()
{
    TmpPdfFile *res = new TmpPdfFile(this);

    connect(res, SIGNAL(progress(int,int)),
            this, SLOT(tmpFileProgress(int,int)));

    connect(res, SIGNAL(merged()),
            this, SLOT(tmpFileMerged()));

    return res;
}


/************************************************
 *
 * ***********************************************/
void Project::addJob(const Job &job)
{
    addJobs(JobList() << job);
}


/************************************************
 *
 * ***********************************************/
void Project::addJobs(const JobList &jobs)
{
    try
    {
        foreach (Job job, jobs)
        {
            mJobs << job;
        }

        stopMerging();
        update();

        mLastTmpFile = createTmpPdfFile();
        mLastTmpFile->merge(mJobs);
    }
    catch (BoomagaError &err)
    {
        qWarning() << Q_FUNC_INFO << err.what();
        error(err.what());
    }
}


/************************************************

 ************************************************/
void Project::removeJob(int index)
{
    try
    {

        stopMerging();

        QString fileName = mJobs.at(index).fileName();
        mJobs.removeAt(index);
        update();

        if (fileName.endsWith(AUTOREMOVE_EXT))
        {
            bool remove = true;
            for (const Job &job: mJobs)
                remove = remove && job.fileName() != fileName;

            if (remove)
                QFile(fileName).remove();
        }

        mLastTmpFile = createTmpPdfFile();
        mLastTmpFile->merge(mJobs);
    }
    catch (BoomagaError &err)
    {
        qWarning() << Q_FUNC_INFO << err.what();
        error(err.what());
    }
}


/************************************************

 ************************************************/
void Project::moveJob(int from, int to)
{
    mJobs.move(from, to);
    update();
}


/************************************************

 ************************************************/
void Project::tmpFileMerged()
{
    TmpPdfFile *tmpPdf = qobject_cast<TmpPdfFile*>(sender());
    if (!tmpPdf)
        return;

    if (tmpPdf != mLastTmpFile)
    {
        tmpPdf->deleteLater();
        return;
    }

//    foreach (const Job &job, mJobs)
//    {
//        for (int p=0; p<job.pageCount(); ++p)
//        {
//            ProjectPage *page = job.page(p);
//            page->setPdfInfo(tmpPdf->pageInfo(job, page->jobPageNum()));
//        }

//    }

    delete mTmpFile;
    mTmpFile = mLastTmpFile;
    mLastTmpFile = 0;

    if (mMetaData.title().isEmpty() && !mJobs.isEmpty())
    {
        mMetaData.setTitle(mJobs.first().title());
    }

    update();
}


/************************************************

 ************************************************/
void Project::update()
{
    ProjectState state(this);
    ProjectPage *curPage = 0;

    mPages.clear();
    foreach(const Job &job, mJobs)
    {
        for (int p=0; p<job.pageCount(); ++p)
        {
            ProjectPage *page = job.page(p);
            if (page->visible())
            {
                page->setPageNum(mPages.count());
                mPages << page;
                if (page == mCurrentPage)
                    curPage = page;
            }
        }
    }
    mRotation = calcRotation(mPages, mLayout);

    if (!mPages.isEmpty())
    {
        mCurrentPage = curPage ? curPage : mPages.first();
    }
    else
    {
        mCurrentPage = 0;
    }


    mSheetCount = 0;

    qDeleteAll(mPreviewSheets);
    mPreviewSheets.clear();
    bool emitTmpFileRenamed = false;

    if (!mPages.isEmpty())
    {
        mLayout->updatePages(mPages);
        mSheetCount = mLayout->calcSheetCount();

        Direction direction = settings->value(Settings::RightToLeft).toBool() ? RightToLeft : LeftToRight;
        mLayout->fillPreviewSheets(&mPreviewSheets, direction);

        if (mTmpFile)
        {
            mTmpFile->updateSheets(mPreviewSheets);
            emitTmpFileRenamed = true;
        }
    }

    foreach (Sheet *s, mPreviewSheets)
    {
        for (int i=0; i<s->count(); ++i)
        {
            if (s->page(i))
                s->page(i)->setSheet(s);

        }
    }

    if (mCurrentPage)
        mCurrentSheet = mCurrentPage->sheet();
    else
        mCurrentSheet = 0;




    if (emitTmpFileRenamed)
        emit tmpFileRenamed(mTmpFile->fileName());

    if (state.currentSheetChanged())
    {
        emit currentSheetChanged(currentSheet());
        emit currentSheetChanged(currentSheetNum());
    }

    if (state.currentPageChanged() || state.currentSheetChanged())
    {
        emit currentPageChanged(currentPage());
        emit currentPageChanged(currentPageNum());
    }

    emit changed();
}


/************************************************
 *
 ************************************************/
int Project::currentPageNum() const
{
    if (mCurrentPage)
        return mCurrentPage->pageNum();

    return -1;
}


/************************************************
 *
 ************************************************/
int Project::previewPageCount() const
{
    return mSheetCount * mLayout->pagePerSheet();
}


/************************************************
 *
 ************************************************/
int Project::previewPageNum(int pageNum) const
{
    if (pageNum < 0)
        return -1;

    const ProjectPage *page = mPages.at(pageNum);
    return mLayout->previewPageNum(page->sheet()->sheetNum()) + page->sheet()->indexOfPage(page);
}


/************************************************
 *
 ************************************************/
int Project::currentPreviewPage() const
{
    if (!mCurrentSheet)
        return 0;

    if (!mCurrentPage)
        return mLayout->previewPageNum(mCurrentSheet->sheetNum());


    return mLayout->previewPageNum(mCurrentSheet->sheetNum()) + mCurrentSheet->indexOfPage(mCurrentPage);
}


/************************************************
 *
 ************************************************/
void Project::deletePage(ProjectPage *page)
{
    // Now the current will be the next visible page.
    // If we delete the last page, the current to be previous.
    ProjectPage *nextCurPage = nextVisiblePage(page);
    if (!nextCurPage)
        nextCurPage = prevVisiblePage(page);


    if (page->isBlankPage())
    {
        int n = mJobs.indexOfProjectPage(page);
        if (n<0)
            return;

        mJobs.value(n).removePage(page);
    }
    else
    {
        page->hide();
    }


    mCurrentPage = nextCurPage;
    update();
}


/************************************************
 *
 ************************************************/
void Project::undoDeletePage(ProjectPage *page)
{
    if (page->visible())
        return;

    page->show();
    mCurrentPage = page;
    update();
}


/************************************************
 *
 ************************************************/
void Project::deletePagesEnd(ProjectPage *page)
{
    int n = jobs()->indexOfProjectPage(page);
    if (n<0)
        return;

    Job job = jobs()->value(n);

    // Now the current will be the next visible page.
    // If we delete the last page, the current to be previous.
    ProjectPage *nextCurPage = nextVisiblePage(job.page(job.pageCount()-1));
    if (!nextCurPage)
        nextCurPage = prevVisiblePage(page);


    QList<ProjectPage*> deleted;
    for (int i = job.indexOfPage(page); i < job.pageCount(); ++i)
    {
        ProjectPage *p = job.page(i);
        if (p->isBlankPage())
            deleted << p;
        else
            p->hide();
    }

    job.removePages(deleted);
    mCurrentPage = nextCurPage;
    update();
}


/************************************************
 *
 ************************************************/
void Project::insertBlankPageBefore(ProjectPage *page)
{
    int j = jobs()->indexOfProjectPage(page);
    if (j<0)
        return;

    Job job = jobs()->value(j);
    mCurrentPage = job.insertBlankPage(job.indexOfPage(page));
    this->update();
}


/************************************************
 *
 ************************************************/
void Project::insertBlankPageAfter(ProjectPage *page)
{
    int j = jobs()->indexOfProjectPage(page);
    if (j<0)
        return;

    Job job = jobs()->value(j);
    mCurrentPage = job.insertBlankPage(job.indexOfPage(page) + 1);
    this->update();
}


/************************************************
 *
 ************************************************/
ProjectPage *Project::prevVisiblePage(ProjectPage *current) const
{
    int n = mPages.indexOf(current) - 1;

    for (; n > -1; n--)
    {
        if (mPages.at(n)->visible())
            return mPages.at(n);
    }

    return NULL;
}


/************************************************
 *
 ************************************************/
ProjectPage *Project::nextVisiblePage(ProjectPage *current) const
{
    int n = mPages.indexOf(current) + 1;
    if (n == 0)
        return NULL;

    for (; n < mPages.size(); n++)
    {
        if (mPages.at(n)->visible())
            return mPages.at(n);
    }

    return NULL;

}


/************************************************
 *
 ************************************************/
void Project::setCurrentPage(ProjectPage *page)
{  
    if (page == mCurrentPage)
        return;

    if (mPreviewSheets.empty())
        return;

    ProjectState state(this);

    if (page)
    {
        mCurrentPage = page;
        mCurrentSheet = page->sheet();
    }
    else
    {
        mCurrentPage = 0;
        mCurrentSheet = 0;
    }

    if (state.currentSheetChanged())
    {
        emit currentSheetChanged(currentSheet());
        emit currentSheetChanged(currentSheetNum());
    }

    if (state.currentPageChanged() || state.currentSheetChanged())
    {
        emit currentPageChanged(currentPage());
        emit currentPageChanged(currentPageNum());
    }
}


/************************************************
 *
 ************************************************/
void Project::setCurrentPage(int pageNum)
{
    if (mPages.empty())
        return;

    pageNum = qBound(0, pageNum, pageCount()-1);
    setCurrentPage(mPages.at(pageNum));
}


/************************************************
 *
 ************************************************/
void Project::prevPage()
{
    setCurrentPage(mCurrentPage - 1);
}


/************************************************
 *
 ************************************************/
void Project::nextPage()
{
    setCurrentPage(mCurrentPage + 1);
}


/************************************************
 *
 ************************************************/
Sheet *Project::currentSheet() const
{
    return mCurrentSheet;
}


/************************************************
 *
 ************************************************/
int Project::currentSheetNum() const
{
    if (mCurrentSheet)
        return mCurrentSheet->sheetNum();

    return -1;
}


/************************************************
 *
 ************************************************/
void Project::setCurrentSheet(int sheetNum)
{
    if (sheetNum == currentSheetNum())
        return;

    if (mPreviewSheets.empty())
        return;

    ProjectState state(this);

    sheetNum = qBound(0, sheetNum, mPreviewSheets.count()-1);
    mCurrentSheet = mPreviewSheets.at(sheetNum);
    mCurrentPage = mCurrentSheet->firstVisiblePage();

    if (state.currentSheetChanged())
    {
        emit currentSheetChanged(currentSheet());
        emit currentSheetChanged(currentSheetNum());
    }

    if (state.currentPageChanged() || state.currentSheetChanged())
    {
        emit currentPageChanged(currentPage());
        emit currentPageChanged(currentPageNum());
    }
}


/************************************************
 *
 ************************************************/
void Project::prevSheet()
{
    setCurrentSheet(currentSheetNum() -1 );
}


/************************************************
 *
 ************************************************/
void Project::nextSheet()
{
    setCurrentSheet(currentSheetNum() + 1 );
}


/************************************************
 *
 * ***********************************************/
Rotation Project::calcRotation(const QList<ProjectPage *> &pages, const Layout *layout) const
{
    foreach (const ProjectPage *page, pages)
    {
        if (page)
        {
            if ((isLandscape(page->pdfRotation()) ^ isLandscape(page->rect())) ^ isLandscape(layout->rotation()))
                return Rotate90;
            else
                return NoRotate;
        }
    }
    return layout->rotation();
}


/************************************************

 ************************************************/
void Project::stopMerging()
{
    if (mLastTmpFile)
    {
        mLastTmpFile->deleteLater();
        mLastTmpFile = 0;
    }
}


/************************************************

 ************************************************/
void Project::tmpFileProgress(int progr, int all) const
{
    if (sender() == mLastTmpFile)
        emit progress(progr, all);
}


/************************************************

 ************************************************/
bool Project::error(const QString &message) const
{
    QMessageBox::critical(0, tr("Boomaga", "Error message title"), message);
    qWarning() << message;
    return false;
}


/************************************************

 ************************************************/
QList<Sheet*> Project::selectSheets(Project::PagesType pages, Project::PagesOrder order) const
{
    int start = 0;
    int inc = 0;
    int end = sheetCount();

    switch (pages)
    {
    case Project::OddPages:
        start = 0;
        inc = 2;
        break;

    case Project::EvenPages:
        start = 1;
        inc = 2;
        break;

    case Project::AllPages:
        start = 0;
        inc = 1;
        break;
    }

    QList<Sheet *> sheets;
    mLayout->fillSheets(&sheets);
    QList<Sheet *> res;

    for (int i=start; i < end; i += inc)
    {
        Sheet *sheet = sheets.at(i);
        sheets[i] = 0;

        if (order == Project::ForwardOrder)
            res.append(sheet);
        else
            res.insert(0, sheet);
    }

    qDeleteAll(sheets);

    return res;
}


/************************************************

 ************************************************/
bool Project::writeDocument(const QList<Sheet*> &sheets, QIODevice *out)
{
    return mTmpFile->writeDocument(sheets, out);
}


/************************************************

 ************************************************/
bool Project::writeDocument(const QList<Sheet*> &sheets, const QString &fileName)
{
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly))
        return project->error(tr("I can't write to file '%1'").arg(fileName) + "\n" + f.errorString());

    bool res = writeDocument(sheets, &f);
    f.close();

    return res;
}


/************************************************
 *
 ************************************************/
bool Project::doubleSided() const
{
    if (mLayout->id() == "Booklet")
        return true;
    else
        return mDoubleSided;
}


/************************************************

 ************************************************/
void Project::setLayout(const Layout *layout)
{
    mLayout = layout;
    update();
}


/************************************************
 *
 ************************************************/
void Project::setDoubleSided(bool value)
{
    mDoubleSided = value;
    emit changed();
}


/************************************************

 ************************************************/
void Project::setPrinterProfile(Printer *printer, int profile, bool update)
{
    if (printer)
    {
        mPrinter = printer;
        mPrinter->setCurrentProfile(profile);
    }
    else
    {
        mPrinter = &mNullPrinter;
    }

    if (update)
    {
        this->update();
    }
}


/************************************************

 ************************************************/
Project *Project::instance()
{
    static Project *inst = 0;
    if (!inst)
        inst = new Project();

    return inst;
}


#if 0
/************************************************

 ************************************************/
QByteArray MetaData::asXMP() const
{
    QString date = QDateTime::currentDateTime().toString(Qt::ISODate);
    QByteArray res;
    res.reserve(META_SIZE);

    res.append("<?xpacket begin=\"").append("\xEF\xBB\xBF", 3).append("\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n");
    xmp(res, "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Boomaga\">");
    xmp(res, "  <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">");

    xmp(res, "    <rdf:Description rdf:about=\"\" xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\">");
    xmp(res, "      <xmp:ModifyDate>%1</xmp:ModifyDate>", date);
    xmp(res, "      <xmp:CreateDate>%1</xmp:CreateDate>", date);
    xmp(res, "      <xmp:MetadataDate>%1</xmp:MetadataDate>", date);
    xmp(res, "      <xmp:CreatorTool>Boomaga Version %1</xmp:CreatorTool>", FULL_VERSION);
    xmp(res, "    </rdf:Description>");

    xmp(res, "    <rdf:Description rdf:about=\"\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\">");
    xmp(res, "      <dc:format>application/pdf</dc:format>");

    if (!mTitle.isEmpty())
    {
        xmp(res, "      <dc:title>");
        xmp(res, "        <rdf:Alt>");
        xmp(res, "          <rdf:li xml:lang=\"x-default\">%1</rdf:li>", mTitle);
        xmp(res, "        </rdf:Alt>");
        xmp(res, "      </dc:title>");
    }

    if (!mCreator.isEmpty())
    {
        xmp(res, "      <dc:creator>");
        xmp(res, "        <rdf:Seq>");
        xmp(res, "          <rdf:li>%1</rdf:li>", mCreator);
        xmp(res, "        </rdf:Seq>");
        xmp(res, "      </dc:creator>");
    }

    if (!mSubject.isEmpty())
    {
        xmp(res, "      <dc:subject>");
        xmp(res, "        <rdf:Bag>");
        xmp(res, "          <rdf:li>%1</rdf:li>", mSubject);
        xmp(res, "        </rdf:Bag>");
        xmp(res, "      </dc:subject>");
    }

    if (!mDescription.isEmpty())
    {
        xmp(res, "      <dc:description>");
        xmp(res, "        <rdf:Alt>");
        xmp(res, "          <rdf:li xml:lang=\"x-default\">%1</rdf:li>", mDescription);
        xmp(res, "        </rdf:Alt>");
        xmp(res, "      </dc:description>");
    }

    xmp(res, "    </rdf:Description>");

    xmp(res, "  </rdf:RDF>");
    xmp(res, "</x:xmpmeta>");

    int n = res.length();
    res = res.leftJustified(META_SIZE - 21, ' ');
    for (int i=n+100; i<res.length(); i+=100)
        res[i]='\n';

    xmp(res, "\n<?xpacket end=\"w\"?>");
    return res;
}


/************************************************

 ************************************************/
void MetaData::xmp(QByteArray &out, const QString &format) const
{
    out.append(format.toUtf8().data());
    out.append('\n');
}


/************************************************

 ************************************************/
void MetaData::xmp(QByteArray &out, const QString &format, const QString &value) const
{
    QString v = value;
    v.replace("'",  "&apos;");
    v.replace("\"", "&quot;");
    v.replace("&",  "&amp;");
    v.replace("<",  "&lt;");
    v.replace(">",  "&gt;");

    out.append(QString(format).arg(v).toUtf8());
    out.append('\n');
}

#endif

/************************************************

 ************************************************/
QByteArray MetaData::asPDFDict() const
{
    QByteArray res;
    res.reserve(META_SIZE);
    QDateTime now = QDateTime::currentDateTime();

    if (!mTitle.isEmpty())
        addDictItem(res, "Title",    mTitle);

    if (!mAuthor.isEmpty())
        addDictItem(res, "Author",   mAuthor);

    if (!mSubject.isEmpty())
        addDictItem(res, "Subject",  mSubject);

    if (!mKeywords.isEmpty())
        addDictItem(res, "Keywords", mKeywords);

    addDictItem(res, "CreationDate", now);  // The date and time the document was created
    addDictItem(res, "ModDate",      now);  // The date and time the document was most recently modified

    return res;
}


/************************************************

 ************************************************/
void MetaData::addDictItem(QByteArray &out, const QString &key, const QString &value) const
{
    out.append(QString("/").toLatin1() + key.toLatin1() + QString(" <FEFF").toLatin1());
    const ushort* utf16 = value.utf16();
    for (int i=0; utf16[i]>0; ++i)
    {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        qint8 b2 = (utf16[i] & 0xFF00) >> 8;
        qint8 b1 = (utf16[i] & 0x00FF);
#else
        qint8 b1 = (utf16[i] & 0xFF00) >> 8;
        qint8 b2 = (utf16[i] & 0x00FF);
#endif
        auto string = QString("%1%2").arg(b1, 2, 16, QChar('0')).arg(b2, 2, 16, QChar('0'));
        out.append(string.toLatin1());
    }
    out.append(">\n");
}


/************************************************
 *
 * ***********************************************/
void MetaData::addDictItem(QByteArray &out, const QString &key, const QDateTime &value) const
{
    QDateTime utc = value.toUTC();
    utc.setTimeSpec(Qt::LocalTime);
    int offset = utc.secsTo(value) / 60;

    out.append(QString("/").toLatin1() + key.toLatin1() + QString(" (").toLatin1());
    out.append(value.toString("yyyyMMddhhmmss").toLatin1());

    if (offset > 0)
    {
        auto string = QString("+%1'%2'")
                          .arg(offset / 60, 2, 10, QChar('0'))
                          .arg(offset % 60, 2, 10, QChar('0'));
        out.append(string.toLatin1());
    }
    else if (offset < 0)
    {
        auto string = QString("-%1'%2'")
                          .arg(-offset / 60, 2, 10, QChar('0'))
                          .arg(-offset % 60, 2, 10, QChar('0'));
        out.append(string.toLatin1());
    }
    out.append(")\n");
}


/************************************************

 ************************************************/
JobList Project::load(const QString &fileName)
{
    return load(QStringList() << fileName);
}


/************************************************

 ************************************************/
JobList Project::load(const QStringList &fileNames)
{
    stopMerging();
    QStringList errors;

    QStringList delFiles;
    JobList jobs;
    foreach(QString fileName, fileNames)
    {
        try
        {
            InFile::Type fileType = InFile::getType(fileName);
            if (fileType == InFile::Type::CupsBoo &&
                fileName.endsWith(AUTOREMOVE_EXT))
            {
                QString old = fileName;
                fileName = genTmpFileName(".cboo");
                QFile::copy(old, fileName);
                delFiles << old;
            }

            QObject keeper;
            InFile *parser = InFile::fromFile(fileName, &keeper);
            connect(parser, &InFile::startLongOperation,
                    [this, &parser](const QString &msg){
                auto task = new ProjectLongTask(msg, parser);
                emit longTaskStarted(task);
            });

            parser->load(fileName);
            jobs << parser->jobs();

            if (fileType == InFile::Type::Boo)
            {
                setMetadata(parser->metaData());
            }
        }
        catch (const BoomagaError &err)
        {
            qWarning() << err.what();
            errors << err.what();
        }
    }


    foreach(QString fileName, delFiles)
    {
        QFile(fileName).remove();
    }

    if (!jobs.isEmpty())
        addJobs(jobs);

    if (!errors.isEmpty())
        error(errors.join("\n\n"));

    return jobs;
}


/************************************************

 ************************************************/
void Project::save(const QString &fileName)
{
    BooFile file;
    file.setMetadata(mMetaData);
    file.setJobs(mJobs);
    file.save(fileName);
}


/************************************************
 *
 ************************************************/
ProjectLongTask::ProjectLongTask(const QString &title, QObject *parent):
    QObject(parent),
    mTitle(title)
{

}


/************************************************
 *
 ************************************************/
ProjectLongTask::~ProjectLongTask()
{
    emit finished();
}
