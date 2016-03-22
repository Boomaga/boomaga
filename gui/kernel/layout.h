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

    virtual int calcSheetCount() const = 0;

    virtual void fillSheets(QList<Sheet*> *sheets) const = 0;
    virtual void fillPreviewSheets(QList<Sheet*> *sheets) const;

    virtual Rotation calcPageRotation(const Page *page, Rotation sheetRotation) const = 0;
    virtual TransformSpec transformSpec(const Sheet *sheet, int pageNumOnSheet, Rotation sheetRotation) const = 0;

    virtual Rotation rotation() const = 0;

protected:
    struct PagePosition
    {
        uint col;
        uint row;
    };

    virtual PagePosition calcPagePosition(int pageNumOnSheet, Rotation sheetRotation) const = 0;
    virtual Rotation calcSheetRotation(int sheetNum, Rotation projectRotation, DuplexType printerType, bool doubleSided) const = 0;
};


class LayoutNUp: public Layout
{
public:
    explicit LayoutNUp(int pageCountVert, int pageCountHoriz, Qt::Orientation orientation = Qt::Horizontal);

    virtual QString id() const;

    virtual int calcSheetCount() const;
    void fillSheets(QList<Sheet*> *sheets) const;
    void fillPreviewSheets(QList<Sheet*> *sheets) const;
    TransformSpec transformSpec(const Sheet *sheet, int pageNumOnSheet, Rotation sheetRotation) const;
    virtual Rotation rotation() const;

protected:
    void doFillSheets(QList<Sheet*> *sheets, bool forPreview) const;
    virtual Rotation calcPageRotation(const Page *page, Rotation sheetRotation) const;
    virtual PagePosition calcPagePosition(int pageNumOnSheet, Rotation sheetRotation) const;
    virtual Rotation calcSheetRotation(int sheetNum, Rotation projectRotation, DuplexType printerType, bool doubleSided) const;

    int mPageCountVert;
    int mPageCountHoriz;
    Qt::Orientation mOrientation;
};


class LayoutBooklet: public LayoutNUp
{
public:
    explicit LayoutBooklet();

    virtual QString id() const { return "Booklet"; }

    virtual int calcSheetCount() const;
    void fillSheets(QList<Sheet*> *sheets) const;
    void fillPreviewSheets(QList<Sheet*> *sheets) const;

protected:
    struct BookletInfo
    {
        int     start;
        int     end;
        bool    manualStart;
        bool    manualEnd;
    };

    QList<BookletInfo> split(const QList<Page*> &pages) const;
    virtual Rotation calcSheetRotation(int sheetNum, Rotation projectRotation, DuplexType printerType, bool doubleSided) const;

private:
    void fillSheetsForBook(int bookStart, int bookLength, QList<Sheet *> *sheets) const;
    void fillPreviewSheetsForBook(int bookStart, int bookLength, QList<Sheet *> *sheets) const;
};


#endif // LAYOUT_H
