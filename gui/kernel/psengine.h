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


#ifndef PSENGINE_H
#define PSENGINE_H

#include "kernel/psproject.h"

#include <QObject>
#include <QTextStream>


class PsSheet;

class PsEngine
{
public:
    explicit PsEngine(PsProject *project);
    virtual ~PsEngine() {}

    PsProject *project() const { return mProject; }

    virtual void fillSheets(QList<PsSheet*> *sheets) = 0;
    virtual void fillPreviewSheets(QList<PsSheet*> *sheets);

    void writeDocument(const QList<const PsSheet *> &sheets, QTextStream *out);

protected:
    // Helper functions
    void writeMatrixText(double xOffset, double yOffset, int rotate, double scale, QTextStream *out);
    void writeClipText(double width, double height, QTextStream *out);
    void writeBorderText(double borderWidth, QTextStream *out);
    void writeProcSetText(QTextStream *out);
    void writePage(const PsProjectPage *page, QTextStream *out);

    void writeMatrix(const PsSheetPageSpec &spec, QTextStream *out);
    void writeSheet(const PsSheet &sheet, QTextStream *out);

protected:
    PsProject *mProject;
};


class EngineNUp: public PsEngine
{
public:
    explicit EngineNUp(PsProject *project, int pageCountVert, int pageCountHoriz);

    void fillSheets(QList<PsSheet *> *sheets);

private:
    PsSheetPageSpec pageSpecForPage(const PsProjectPage *page, int pageNumOnSheet);
    int mPageCountVert;
    int mPageCountHoriz;
    bool mRotate;
};


class EngineBooklet: public PsEngine
{
public:
    explicit EngineBooklet(PsProject *project): PsEngine(project) {}

    void fillSheets(QList<PsSheet *> *sheets);
    void fillPreviewSheets(QList<PsSheet*> *sheets);

private:
    void fillSheetsForBook(int bookStart, int bookLength, QList<PsSheet *> *sheets);
    void fillPreviewSheetsForBook(int bookStart, int bookLength, QList<PsSheet *> *sheets);
    PsSheetPageSpec pageSpecForPage(const PsProjectPage *page, int pageNumOnSheet);
};

#endif // PSENGINE_H
