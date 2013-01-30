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


#ifndef PSSHEET_H
#define PSSHEET_H


#include "psfile.h"


#include <QObject>
#include <QVector>
#include <QList>
#include <QRect>
#include <QDebug>

class PsProject;
class PsProjectPage;

class PsSheetPageSpec
{
public:
    enum Rotation
    {
        NoRotate  = 0,
        Rotate90  = 90,
        Rotate180 = 180,
        Rotate270 = 270
    };

    PsSheetPageSpec();
    PsSheetPageSpec(const QRectF &rect, double scale, Rotation rotete);

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

class PsSheet
{
public:
    enum Hint{
        HintOnlyLeft         = 1,
        HintOnlyRight        = 2,
        HintDrawFold         = 4,
        HintLandscapePreview = 8
    };

    Q_DECLARE_FLAGS(Hints, Hint)

    explicit PsSheet(PsProject *project, int count);
    ~PsSheet();

    PsProject *project() const { return mProject; }

    int count() const { return mPages.count(); }

    PsProjectPage *page(int index) { return mPages[index]; }
    const PsProjectPage *page(int index) const { return mPages.at(index); }
    void setPage(int index, PsProjectPage *page);

    PsSheetPageSpec pageSpec(int index) const { return mSpecs[index]; }
    void setPageSpec(int index, PsSheetPageSpec spec);

    int sheetNum() const { return mSheetNum; }
    void setSheetNum(int value) { mSheetNum = value; }

    Hints hints() const { return mHints; }
    void setHints(Hints value);
    void setHint(Hint hint, bool enable);

    //void write(QTextStream *out) const;
signals:
    
public slots:
    
protected:
    PsProject *mProject;
    QVector<PsProjectPage*> mPages;
    QVector<PsSheetPageSpec> mSpecs;
    int mSheetNum;
    Hints mHints;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PsSheet::Hints)

QDebug operator<<(QDebug dbg, const PsSheet &sheet);
QDebug operator<<(QDebug dbg, const PsSheet *sheet);

QDebug operator<<(QDebug dbg, const QList<PsSheet *> &sheets);
QDebug operator<<(QDebug dbg, const QList<PsSheet *> *sheets);


QDebug operator<<(QDebug dbg, const PsSheetPageSpec &spec);
QDebug operator<<(QDebug dbg, const PsSheetPageSpec *spec);

#endif // PSSHEET_H
