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


#ifndef LAYOUT_H
#define LAYOUT_H

#include <QList>
#include <QRectF>
#include <QString>
#include "project.h"

class Sheet;
class Project;

struct TransformSpec
{
public:
    QRectF rect;
    Rotation rotation;
    double scale;
};

class Layout
{
public:
    Layout();
    virtual ~Layout();

    virtual QString id() const = 0;

    virtual void fillSheets(QList<Sheet*> *sheets) const = 0;
    virtual void fillPreviewSheets(QList<Sheet*> *sheets) const;

    virtual TransformSpec transformSpec(const Sheet *sheet, int pageNumOnSheet) const = 0;

    virtual Rotation rotate() const = 0;

protected:
    struct PagePosition
    {
        uint col;
        uint row;
    };

    virtual PagePosition calcPagePosition(const Sheet *sheet, int pageNumOnSheet,
                                          int pageCountHoriz, int pageCountVert,
                                          Qt::Orientation orientation) const;


    TransformSpec calcTransformSpec(const Sheet *sheet, int pageNumOnSheet,
                                    int pageCountHoriz, int pageCountVert,
                                    Qt::Orientation orientation) const;
};


class LayoutNUp: public Layout
{
public:
    explicit LayoutNUp(int pageCountVert, int pageCountHoriz, Qt::Orientation orientation = Qt::Horizontal);

    virtual QString id() const;

    void fillSheets(QList<Sheet*> *sheets) const;
    TransformSpec transformSpec(const Sheet *sheet, int pageNumOnSheet) const;
    virtual Rotation rotate() const;

private:
    int mPageCountVert;
    int mPageCountHoriz;
    Qt::Orientation mOrientation;
};


class LayoutBooklet: public Layout
{
public:
    explicit LayoutBooklet();

    virtual QString id() const { return "Booklet"; }

    void fillSheets(QList<Sheet*> *sheets) const;
    void fillPreviewSheets(QList<Sheet*> *sheets) const;

    TransformSpec transformSpec(const Sheet *sheet, int pageNumOnSheet) const;
    virtual Rotation rotate() const { return Rotate90; }

private:
    void fillSheetsForBook(int bookStart, int bookLength, QList<Sheet *> *sheets) const;
    void fillPreviewSheetsForBook(int bookStart, int bookLength, QList<Sheet *> *sheets) const;
};


#endif // LAYOUT_H
