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


#ifndef SHEET_H
#define SHEET_H

#include <QtGlobal>
#include <QVector>
#include <QRectF>

class ProjectPage;

class SheetPageSpec
{
public:
    enum Rotation
    {
        NoRotate  = 0,
        Rotate90  = 90,
        Rotate180 = 180,
        Rotate270 = 270
    };

    SheetPageSpec();
    SheetPageSpec(const QRectF &rect, double scale, Rotation rotete);

    QRectF rect() const { return mRect; }
    void setRect(const QRectF &value) { mRect = value; }

    double scale() const { return mScale; }
    void setScale(double value) { mScale = value; }

    Rotation rotate() const { return mRotate; }
    void setRotate(Rotation value) { mRotate = value ;}

private:
    QRectF mRect;
    double mScale;
    Rotation mRotate;
};


class Sheet
{
public:
    enum Hint{
        HintOnlyLeft         = 1,
        HintOnlyRight        = 2,
        HintDrawFold         = 4,
        HintLandscapePreview = 8
    };

    Q_DECLARE_FLAGS(Hints, Hint)

    Hints hints() const { return mHints; }
    void setHints(Hints value);
    void setHint(Hint hint, bool enable);

    explicit Sheet(int count, int sheetNum);
    virtual ~Sheet();

    ProjectPage *page(int index) { return mPages[index]; }
    const ProjectPage *page(int index) const { return mPages.at(index); }
    void setPage(int index, ProjectPage *page);

    int count() const { return mPages.count(); }

    int sheetNum() const { return mSheetNum; }
    //void setSheetNum(int value) { mSheetNum = value; }

    SheetPageSpec pageSpec(int index) const { return mSpecs[index]; }
    void setPageSpec(int index, SheetPageSpec spec);

    qint64 id() const { return mId; }

private:
    QVector<ProjectPage*> mPages;
    QVector<SheetPageSpec> mSpecs;
    Hints mHints;
    int mSheetNum;
    qint64 mId;
    static qint64 genId();
};

typedef QList<Sheet*> SheetList;


Q_DECLARE_OPERATORS_FOR_FLAGS(Sheet::Hints)
#endif // SHEET_H
