#ifndef LAYOUT_H
#define LAYOUT_H

#include <QList>
#include <QRectF>
#include <QString>

class Sheet;
class Project;

struct TransformSpec
{
public:
    enum Rotation
    {
        NoRotate  = 0,
        Rotate90  = 90,
        Rotate180 = 180,
        Rotate270 = 270
    };

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

protected:
    TransformSpec calcTransformSpec(const Sheet *sheet, int pageNumOnSheet,
                                        int pageCountHoriz, int pageCountVert) const;
};


class LayoutNUp: public Layout
{
public:
    explicit LayoutNUp(int pageCountVert, int pageCountHoriz);

    virtual QString id() const;

    void fillSheets(QList<Sheet*> *sheets) const;
    TransformSpec transformSpec(const Sheet *sheet, int pageNumOnSheet) const;

private:
    int mPageCountVert;
    int mPageCountHoriz;
    bool mRotate;
};


class LayoutBooklet: public Layout
{
public:
    explicit LayoutBooklet();

    virtual QString id() const { return "Booklet"; }

    void fillSheets(QList<Sheet*> *sheets) const;
    void fillPreviewSheets(QList<Sheet*> *sheets) const;

    TransformSpec transformSpec(const Sheet *sheet, int pageNumOnSheet) const;

private:
    void fillSheetsForBook(int bookStart, int bookLength, QList<Sheet *> *sheets) const;
    void fillPreviewSheetsForBook(int bookStart, int bookLength, QList<Sheet *> *sheets) const;
};


#endif // LAYOUT_H
