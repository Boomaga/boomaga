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


#include "widgets.h"
#include "settings.h"
#include "kernel/printer.h"


#include <QMenu>
#include <QMouseEvent>

/************************************************

 ************************************************/
LayoutRadioButton::LayoutRadioButton(QWidget *parent):
    QRadioButton(parent),
    mPsLayout(PsProject::LayoutBooklet)
{

}


/************************************************

 ************************************************/
LayoutRadioButton::LayoutRadioButton(const QString &text, QWidget *parent):
    QRadioButton(text, parent),
    mPsLayout(PsProject::LayoutBooklet)
{
}


/************************************************

 ************************************************/
PrintersComboBox::PrintersComboBox(QWidget *parent):
    QComboBox(parent)
{
}


/************************************************

 ************************************************/
PrintersComboBox::~PrintersComboBox()
{
}


/************************************************

 ************************************************/
Printer *PrintersComboBox::currentPrinter()
{
    return itemPrinter(currentIndex());
}


/************************************************

 ************************************************/
void PrintersComboBox::setCurrentPrinter(const QString &printerName)
{
    for (int i=0; i<count(); ++i)
    {
        if (itemPrinter(i)->printerName() == printerName)
        {
            setCurrentIndex(i);
            return;
        }
    }
}


/************************************************

 ************************************************/
int PrintersComboBox::addPrinter(Printer *printer)
{
    addItem(printer->printerName(), qVariantFromValue((void *)printer));
    return this->count() - 1;
}


/************************************************

 ************************************************/
Printer *PrintersComboBox::itemPrinter(int index)
{
    return (Printer*) itemData(index).value<void *>();
}


/************************************************

 ************************************************/
PsFilesListView::PsFilesListView(QWidget *parent):
    QListWidget(parent),
    mProject(0)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(this->model(), SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
}


/************************************************

 ************************************************/
void PsFilesListView::setProject(PsProject *project)
{
    mProject = project;
    connect(project, SIGNAL(changed()), this, SLOT(updateItems()));
    updateItems();
}


/************************************************

 ************************************************/
void PsFilesListView::updateItems()
{
    clear();
    for (int i=0; i<mProject->filesCount(); ++i)
    {
        QListWidgetItem *item = new QListWidgetItem(this);
        item->setData(Qt::UserRole, i);
        PsFile *file = mProject->file(i);

        item->setText(tr("( %1 pages ) ").arg(file->pageCount()) +
                    (file->title().isEmpty() ? tr("Untitled") : file->title()));

        addItem(item);
    }
}


/************************************************

 ************************************************/
void PsFilesListView::showContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = itemAt(pos);
    if (!item)
        return;

    QMenu menu(this);
    QAction *act = menu.addAction(tr("Delete job"));
    act->setData(item->data(Qt::UserRole));
    connect(act, SIGNAL(triggered()), this, SLOT(deleteFile()));
    menu.exec(mapToGlobal(pos));
}


/************************************************

 ************************************************/
void PsFilesListView::deleteFile()
{
    QAction *act = qobject_cast<QAction*>(sender());
    if (act)
    {
        int index = act->data().toInt();
        mProject->removeFile(index);
    }
    updateItems();
}


/************************************************

 ************************************************/
void PsFilesListView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QListWidgetItem *item = this->itemAt(event->pos());
        if (item)
        {
            setCurrentItem(item);
            int n = item->data(Qt::UserRole).toInt();
            if (n<mProject->filesCount())
            {
                emit fileSelected(mProject->file(n));
            }
        }
    }
    QListWidget::mouseReleaseEvent(event);
}


/************************************************

 ************************************************/
void PsFilesListView::layoutChanged()
{
    for (int i=0; i<count()-1; ++i)
    {
        int from = item(i)->data(Qt::UserRole).toInt();
        if (from != i)
            mProject->moveFile(from, i);
    }
}

