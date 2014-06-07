/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2014 Boomaga team https://github.com/Boomaga
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


#ifndef PROJECTFILE_H
#define PROJECTFILE_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include "job.h"
#include "project.h"

class QFile;

class BackendOptions
{
public:
    BackendOptions(const QString &options);

    QList<int> pages() const { return mPages; }

private:
    void parsePages(const QString &value);

    QList<int> mPages;
    QHash<QString, QString> mOptions;
};


/************************************************
 * File format
 *
 * First line - \x1B%-12345X@PJL BOOMAGA_PROGECT
 *
 * Project meta info:
 *  @PJL BOOMAGA META_AUTHOR="str"      - author
 *  @PJL BOOMAGA META_TITLE="str"       - title
 *  @PJL BOOMAGA META_SUBJECT="str"     - subject
 *  @PJL BOOMAGA META_KEYWORDS="str"    - keywords
 *
 * Job:
 *  @PJL BOOMAGA JOB_TITLE="str"        - title of job
 *  @PJL BOOMAGA JOB_PAGES="pagesSpec"  - define pages state
 *                                          hidden, inserted
 *
 * Pages spec is a list of the pageSpec.
 *  Page1Spec,PageNSpec,...
 *
 * Page spec is PageNum:Hidden:Rotation:StarBooklet
 *  PageNum  -  number of the page in the source PDF. If page
 *              is a inserted blank page PageNum is letter 'B'
 *  Hidden   -  if page is hidden then use letter 'H', otherwise
 *              this field is empty or omitted.
 *  Rotation -  One of 0,90,180,270. If rotation is 0 this field
 *              can be omitted.
 * StarBooklet- if it's first page in booklet use letter 'S',
 *              otherwise this field is empty or omitted.
 *
 * Example:
 *  @PJL BOOMAGA JOB_PAGES="1,2::180,B,3:H:90,4:H"
 *
 ************************************************/

class ProjectFile : public QObject
{
    Q_OBJECT
public:
    explicit ProjectFile(QObject *parent = 0);
    virtual ~ProjectFile();

    void load(const QString &fileName, const QString &options = "");
    void save(const QString &fileName);

    JobList jobs() const { return mJobs; }
    void setJobs(const JobList &value) { mJobs = value; }

    const MetaData metaData() const { return mMetaData; }
    void setMetadata(const MetaData &value) { mMetaData = value; }

signals:
    
protected:
    class PageSpec
    {
    public:
        PageSpec(int pageNum, bool hidden, Rotation rotation, bool startBooklet);
        PageSpec(const QString &spec);
        QString asString();

        int pageNum() const { return mPageNum; }
        bool isHidden() const { return mHidden; }
        bool isStartBooklet() const { return mStartBooklet; }
        bool isblank() const { return mPageNum < 0; }
        Rotation rotation() { return mRotation; }

        static QList<PageSpec> readPagesSpec(const QString &str);
    private:
        int  mPageNum;
        bool mHidden;
        Rotation mRotation;
        bool mStartBooklet;
    };
    
private:
    QString mFileName;
    JobList mJobs;
    MetaData mMetaData;

    QByteArray readJobPDF(const Job &job);
    void write(QFile *out, const QByteArray &data);
    void writeCommand(QFile *out, const QString &command, const QString &data);
    void writeCommand(QFile *out, const QString &command, const QList<int> &data);
};

#endif // PROJECTFILE_H
