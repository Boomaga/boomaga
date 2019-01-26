/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2016 Boomaga team https://github.com/Boomaga
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


#ifndef PROJECTPAGE_H
#define PROJECTPAGE_H

#include <QObject>
#include <QRectF>
#include "boomagatypes.h"
#include "../pdfmerger/pdfmergeripc.h"

class Sheet;

class ProjectPage : public QObject
{
    Q_OBJECT
    friend class Project;
public:
    explicit ProjectPage(QObject *parent = 0);
    explicit ProjectPage(int jobPageNum, QObject *parent = 0);
    virtual ~ProjectPage();

    int jobPageNum() const { return mJobPageNum; }
    int pageNum() const { return mPageNum; }
    Sheet *sheet() const { return mSheet; }

    virtual QRectF rect() const;
    Rotation pdfRotation() const;
    Rotation manualRotation() const { return mManualRotation; }
    void setManualRotation(Rotation value) { mManualRotation = value; }

    PdfPageInfo pdfInfo() const { return mPdfInfo; }
    void setPdfInfo(const PdfPageInfo &value) { mPdfInfo = value; }

    bool visible() const { return mVisible;}
    void setVisible(bool value);
    void hide() { setVisible(false); }
    void show() { setVisible(true); }

    bool isBlankPage() const;

    bool isStartSubBooklet() const { return mManualStartSubBooklet || mAutoStartSubBooklet; }
    bool isManualStartSubBooklet() const { return mManualStartSubBooklet; }
    void setManualStartSubBooklet(bool value);
    bool isAutoStartSubBooklet() const { return mAutoStartSubBooklet; }
    void setAutoStartSubBooklet(bool value);


    ProjectPage *clone(QObject *parent = 0);

protected:
    void setPageNum(int pageNum) { mPageNum = pageNum; }
    void setSheet(Sheet *sheet) { mSheet = sheet; }

private:
    Q_DISABLE_COPY(ProjectPage)
    int mJobPageNum;
    int mPageNum;
    Sheet *mSheet;
    bool mVisible;
    PdfPageInfo mPdfInfo;
    Rotation mManualRotation;
    bool mManualStartSubBooklet;
    bool mAutoStartSubBooklet;
};

#endif // PROJECTPAGE_H
