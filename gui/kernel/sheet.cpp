#include "sheet.h"


/************************************************

 ************************************************/
SheetPageSpec::SheetPageSpec()
{
    mRect = QRect(0, 0, 0, 0);
    mScale = 1;
    mRotate = NoRotate;
}


/************************************************

 ************************************************/
SheetPageSpec::SheetPageSpec(const QRectF &rect, double scale, SheetPageSpec::Rotation rotete)
{
    mRect = rect;
    mScale = scale;
    mRotate = rotete;
}


/************************************************

 ************************************************/
Sheet::Sheet(int count, int sheetNum):
    mId(Sheet::genId()),
    mSheetNum(sheetNum),
    mHints(0)
{
    mPages.resize(count);
    mSpecs.resize(count);
    for (int i=0; i<count; ++i)
        mPages[i] = 0;
}


/************************************************

 ************************************************/
Sheet::~Sheet()
{
}


/************************************************

 ************************************************/
void Sheet::setPage(int index, ProjectPage *page)
{
    mPages[index] = page;
}


/************************************************

 ************************************************/
void Sheet::setPageSpec(int index, SheetPageSpec spec)
{
    mSpecs[index] = spec;
}


/************************************************

 ************************************************/
qint64 Sheet::genId()
{
    static qint64 id = 0;
    return ++id;
}


/************************************************

 ************************************************/
void Sheet::setHints(Sheet::Hints value)
{
    mHints = value;
}


/************************************************

 ************************************************/
void Sheet::setHint(Sheet::Hint hint, bool enable)
{
    if (enable)
        mHints = mHints | hint;
    else
        mHints = mHints & (~hint);
}

