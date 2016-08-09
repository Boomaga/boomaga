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

#include "layoutradiobutton.h"
//#include "widgets.h"
//#include "settings.h"
//#include "kernel/printer.h"
//#include "kernel/inputfile.h"
//#include "kernel/job.h"
//#include "kernel/sheet.h"
//#include "icon.h"

//#include <QMenu>
//#include <QMouseEvent>
//#include <QStandardItem>
//#include <QDebug>



/************************************************

 ************************************************/
LayoutRadioButton::LayoutRadioButton(QWidget *parent):
    QRadioButton(parent),
    mLayout(0)
{
}


/************************************************

 ************************************************/
LayoutRadioButton::LayoutRadioButton(const QString &text, QWidget *parent):
    QRadioButton(text, parent),
    mLayout(0)
{
}
