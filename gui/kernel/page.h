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


#ifndef PAGE_H
#define PAGE_H

#include <QObject>
#include "inputfile.h"

#include <QRectF>
#include "../boomagatypes.h"
#include "../pdfmerger/pdfmergeripc.h"

class Page : public QObject
{
    Q_OBJECT
public:
    explicit Page(QObject *parent = 0);
    Page(const InputFile &inputFile, int pageNum, QObject *parent = 0);
    explicit Page(const Page *other, QObject *parent = 0);
    virtual ~Page();

    InputFile inputFile() const { return mInputFile; }
    int pageNum() const { return mPageNum; }

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

    bool isStartSubBooklet() const { return mStartSubBooklet; }
    void setStartSubBooklet(bool value);

private:
    InputFile mInputFile;
    int mPageNum;
    bool mVisible;
    PdfPageInfo mPdfInfo;
    Rotation mManualRotation;
    bool mStartSubBooklet;


};


#endif // PAGE_H
