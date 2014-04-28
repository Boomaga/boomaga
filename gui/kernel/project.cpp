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

#include "inputfile.h"
#include "tmppdffile.h"
#include "sheet.h"
#include "layout.h"
#include "projectfile.h"

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QDateTime>


#define META_SIZE 4 * 1024

/************************************************

 ************************************************/
ProjectPage::ProjectPage():
    mPageNum(-1),
    mVisible(true),
    mManualRotate(NoRotate)
{

}


/************************************************
 *
 * ***********************************************/
ProjectPage::ProjectPage(const ProjectPage *other):
    mInputFile(other->mInputFile),
    mPageNum(other->mPageNum),
    mVisible(other->mVisible),
    mPdfInfo(other->mPdfInfo),
    mManualRotate(NoRotate)
{

}

/************************************************
 *
 ************************************************/
ProjectPage::ProjectPage(const InputFile &inputFile, int pageNum):
    mInputFile(inputFile),
    mPageNum(pageNum),
    mVisible(true),
    mManualRotate(NoRotate)
{
}


/************************************************

 ************************************************/
ProjectPage::~ProjectPage()
{
}


/************************************************
 *
 * ***********************************************/
QRectF ProjectPage::rect() const
{
    if (mPdfInfo.cropBox.isValid())
        return mPdfInfo.cropBox;
    else
        return project->printer()->paperRect();
}


/************************************************

 ************************************************/
Rotation ProjectPage::rotate() const
{
    uint r = (mPdfInfo.rotate + mManualRotate) % 360;

    if (r == 90)    return Rotate90;
    if (r == 180)   return Rotate180;
    if (r == 270)   return Rotate270;
    else            return NoRotate;
}


/************************************************

 ************************************************/
void ProjectPage::setVisible(bool value)
{
    mVisible = value;
}


/************************************************

 ************************************************/
bool ProjectPage::isBlankPage()
{
    return mPageNum < 0;
}


/************************************************

 ************************************************/
Project::Project(QObject *parent) :
    QObject(parent),
    mLayout(0),
    mTmpFile(0),
    mLastTmpFile(0),
    mPrinter(&mNullPrinter),
    mDoubleSided(true)
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
    mJobs.clear();
    delete mTmpFile;
}


/************************************************

 ************************************************/
TmpPdfFile *Project::createTmpPdfFile()
{
    TmpPdfFile *res = new TmpPdfFile(mJobs, this);

    connect(res, SIGNAL(progress(int,int)),
            this, SLOT(tmpFileProgress(int,int)));

    connect(res, SIGNAL(merged()),
            this, SLOT(tmpFileMerged()));

    return res;
}


/************************************************
 *
 * ***********************************************/
void Project::addJob(Job job)
{
    addJobs(JobList() << job);
}


/************************************************
 *
 * ***********************************************/
void Project::addJobs(JobList jobs)
{
    foreach (Job job, jobs)
    {
        mJobs << job;
    }

    stopMerging();
    update();

    mLastTmpFile = createTmpPdfFile();
    mLastTmpFile->merge();
}


/************************************************

 ************************************************/
void Project::removeJob(int index)
{
    stopMerging();
    mJobs.removeAt(index);
    update();

    mLastTmpFile = createTmpPdfFile();
    mLastTmpFile->merge();
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

    foreach (Job job, mJobs)
    {
        for (int p=0; p<job.pageCount(); ++p)
        {
            ProjectPage *page = job.page(p);
            page->setPdfInfo(tmpPdf->pageInfo(job.inputFile(), page->pageNum()));
        }

    }

    delete mTmpFile;
    mTmpFile = mLastTmpFile;
    connect(mTmpFile, SIGNAL(imageChanged(int)),
            this, SIGNAL(sheetImageChanged(int)));

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
    mPages.clear();
    foreach(const Job &job, mJobs)
    {
        for (int p=0; p<job.pageCount(); ++p)
        {
            if (job.page(p)->visible())
                mPages << job.page(p);
        }
    }

    qDeleteAll(mSheets);
    mSheets.clear();

    qDeleteAll(mPreviewSheets);
    mPreviewSheets.clear();

    if (!mPages.isEmpty())
    {
        mLayout->fillSheets(&mSheets);
        mLayout->fillPreviewSheets(&mPreviewSheets);

        if (mTmpFile)
            mTmpFile->updateSheets(mPreviewSheets);
    }

    emit changed();
}


/************************************************

 ************************************************/
void Project::stopMerging()
{
    if (mLastTmpFile)
    {
        mLastTmpFile->stop();
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
bool Project::error(const QString &message)
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

    QList<Sheet *> res;

    if (order == Project::ForwardOrder)
    {
        for (int i=start; i < end; i += inc)
            res.append(mSheets.at(i));
    }
    else
    {
        for (int i=start; i < end; i += inc)
            res.insert(0, mSheets.at(i));
    }

    return res;
}


/************************************************

 ************************************************/
bool Project::writeDocument(const QList<Sheet *> &sheets, QIODevice *out)
{
    return mTmpFile->writeDocument(sheets, out);
}


/************************************************

 ************************************************/
bool Project::writeDocument(const QList<Sheet *> &sheets, const QString &fileName)
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
void Project::setPrinter(Printer *value)
{
    if (value)
        mPrinter = value;
    else
        mPrinter = &mNullPrinter;

    update();
    emit changed();
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


/************************************************

 ************************************************/
QImage Project::sheetImage(int sheetNum) const
{
    if (mTmpFile)
        return mTmpFile->image(sheetNum);
    else
    {
        QImage img(1, 1, QImage::Format_ARGB32);
        img.fill(Qt::white);
        return img;
    }
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
    out.append("/" + key + " <FEFF");
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
        out.append(QString("%1%2").arg(b1, 2, 16, QChar('0')).arg(b2, 2, 16, QChar('0')));
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

    out.append("/" + key + " (");
    out.append(value.toString("yyyyMMddhhmmss"));

    if (offset > 0)
        out.append(QString("+%1'%2'")
                   .arg(offset / 60, 2, 10, QChar('0'))
                   .arg(offset % 60, 2, 10, QChar('0')));
    else if (offset < 0)
        out.append(QString("-%1'%2'")
                    .arg(-offset / 60, 2, 10, QChar('0'))
                    .arg(-offset % 60, 2, 10, QChar('0')));

    out.append(")\n");
}


/************************************************

 ************************************************/
void Project::load(const QString &fileName, const QString title, bool autoRemove)
{
    load(QStringList() << fileName, QStringList() << title, autoRemove);
}


/************************************************

 ************************************************/
void Project::load(const QStringList &fileNames, bool autoRemove)
{
    load(fileNames, QStringList(), autoRemove);
}


/************************************************

 ************************************************/
void Project::load(const QStringList &fileNames, const QStringList &titles, bool autoRemove)
{
    stopMerging();
    QStringList errors;

    JobList jobs;
    foreach(QString fileName, fileNames)
    {
        ProjectFile file;
        try
        {
            file.load(fileName);
            for(int i=0; i<file.jobs().count(); ++i)
            {
                Job job = file.jobs().at(i);
                job.setAutoRemove(autoRemove);
                if (i<titles.count())
                    job.setTitle(titles.at(i));

                jobs << job;
            }

            project->setMetadata(file.metaData());
        }
        catch (const QString &err)
        {
            errors << err;
        }
    }

    if (!jobs.isEmpty())
        addJobs(jobs);

    if (!errors.isEmpty())
        throw errors.join("\n\n");

}


/************************************************

 ************************************************/
void Project::save(const QString &fileName)
{
    ProjectFile file;
    file.setMetadata(mMetaData);
    file.setJobs(mJobs);
    file.save(fileName);
}
