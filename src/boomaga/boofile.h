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


#ifndef BOOFILE_H
#define BOOFILE_H

#include "kernel/project.h"
#include "boomagatypes.h"
#include "kernel/job.h"

/************************************************
 * File format
 *
 * First line - \x1B%-12345X@PJL BOOMAGA_PROJECT
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
class BooFile : public QObject
{
    Q_OBJECT
public:
    explicit BooFile(QObject *parent = 0);

    void setMetadata(const MetaData &value) { mMetaData = value; }
    void setJobs(const JobList &value) { mJobs = value; }
    void save(const QString &fileName);

public:
    struct PageSpec{
        explicit PageSpec(int pageNum, bool hidden, Rotation rotation, bool startBooklet);
        explicit PageSpec(const QString &str);
        static QList<PageSpec> readPagesSpec(const QString &str);

        bool isblank() const { return pageNum < 0; }
        QString toString() const;
        int      pageNum;
        Rotation rotation;
        bool     hidden;
        bool     startBooklet;
    };
private:
    JobList mJobs;
    MetaData mMetaData;
};

#endif // BOOFILE_H
