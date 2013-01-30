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


#ifndef PSVIEW_H
#define PSVIEW_H


#include <QFrame>
#include <QImage>

class PsSheet;
class PsRender;

class PsView : public QFrame
{
    Q_OBJECT
public:
    explicit PsView(QWidget *parent = 0);
    ~PsView();
    
    int currentSheet() const { return mSheetNum; }

    PsRender *render() const { return  mRender; }
    void setRender(PsRender *value);

public slots:
    void setCurrentSheet(int sheetNum);

signals:
    void whellScrolled(int delta);

protected:
    void paintEvent(QPaintEvent *event);
    void wheelEvent(QWheelEvent *event);

private slots:
    void renderChanged(int sheetNum);

private:
    QImage mImage;
    int mSheetNum;
    const PsSheet *mSheet;
    PsRender *mRender;
};

#endif // PSVIEW_H
