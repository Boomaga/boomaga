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


#ifndef PSPROJECT_H
#define PSPROJECT_H

#include "pssheet.h"
#include "printer.h"

#include <QObject>
#include <QList>


class PsFile;
class PsEngine;
class PsProjectPage;

class PsProject : public QObject
{
    Q_OBJECT
    friend class PsEngine;
    friend class PsProjectPage;
public:
    enum PsLayout
    {
        Layout1Up,
        Layout2Up,
        Layout4Up,
        Layout8Up,
        LayoutBooklet
    };

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

    explicit PsProject(QObject *parent = 0);
    ~PsProject();

    int filesCount() const { return mFiles.count(); }
    PsFile *file(int index) { return mFiles[index]; }

    int pageCount() const;
    PsProjectPage *page(int index);

    int sheetCount() const { return mSheets.count(); }
    PsSheet *sheet(int index) { return mSheets[index]; }

    int previewSheetCount() const { return mPreviewSheets.count(); }
    PsSheet *previewSheet(int index) { return mPreviewSheets[index]; }

    void writeDocument(const QList<const PsSheet*> &sheets, QTextStream *out);
    void writeDocument(PagesType pages, QTextStream *out);
    void writeDocument(PagesType pages, PagesOrder order, QTextStream *out);

    void writeDocument(const QList<const PsSheet*> &sheets, const QString &fileName);
    void writeDocument(PagesType pages, const QString &fileName);
    void writeDocument(PagesType pages, PagesOrder order, const QString &fileName);


    PsLayout layout() const { return mLayout; }

    Printer *printer() const { return mPrinter; }
    void setPrinter(Printer *value);


    static QString layoutToStr(PsLayout value);
    static PsLayout strToLayout(const QString &value);

public slots:
    void addFile(QString fileName);
    void removeFile(int index);
    void moveFile(int from, int to);

    void setLayout(PsLayout layout);

signals:
    void changed();
    void fileAboutToBeAdded(QString fileName);
    void fileAdded(const PsFile *file);
    void fileAboutToBeRemoved(const PsFile *file);
    void fileRemoved();
    void fileAboutToBeMoved(const PsFile *file);
    void fileMoved();


protected:
    GsMergeFile *psFile() { return mPsFile; }

private:
    PsLayout mLayout;
    QList<PsProjectPage*> mPages;
    QList<PsFile*> mFiles;
    QList<PsSheet*> mSheets;
    QList<PsSheet*> mPreviewSheets;


    Printer *mPrinter;
    PsEngine *mEngine;

    GsMergeFile *mPsFile;
    Printer mNullPrinter;

    void updateSheets();
};


class PsProjectPage
{
public:
    PsProjectPage(PsFile *file, int pageNum, PsProject *project);

    PsFile *file() const { return mFile; }

    bool visible() const { return mVisible;}
    void setVisible(bool value);

    int pageNum() const { return mPageNum; }

    QRect rect() const;
    void writePage(QTextStream *out) const;

private:
    PsFile *mFile;
    bool mVisible;

    int mPageNum;
    PsProject *mProject;
};



#endif // PSPROJECT_H
