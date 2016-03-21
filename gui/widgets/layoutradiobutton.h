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


#ifndef LAYOUTRADIOBUTTON_H
#define LAYOUTRADIOBUTTON_H

#include <QRadioButton>
class Layout;

class LayoutRadioButton : public QRadioButton
{
    Q_OBJECT
public:
    explicit LayoutRadioButton(QWidget *parent=0);
    explicit LayoutRadioButton(const QString &text, QWidget *parent=0);


    Layout *layout() const { return mLayout; }
    void setLayout(Layout *value) { mLayout = value; }
signals:
    
public slots:
    
private:
    Layout *mLayout;
};

#endif // LAYOUTRADIOBUTTON_H
