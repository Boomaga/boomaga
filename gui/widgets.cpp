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
#include "kernel/inputfile.h"
#include "kernel/sheet.h"

#include <QMenu>
#include <QMouseEvent>

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
InputFilesListView::InputFilesListView(QWidget *parent):
    QListWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(this->model(), SIGNAL(layoutChanged()), this, SLOT(layoutChanged()));
    connect(project, SIGNAL(changed()), this, SLOT(updateItems()));
    updateItems();
}


/************************************************

 ************************************************/
void InputFilesListView::setSheetNum(int sheetNum)
{
    if (count()<1)
        return;

    if (sheetNum >= project->previewSheetCount())
    {
        item(0)->setSelected(true);
        return;
    }


    Sheet *sheet = project->previewSheet(sheetNum);
    for (int i=0; i<sheet->count(); ++i)
    {
        ProjectPage *page = sheet->page(i);
        if (!page)
            continue;

        int n = project->inputFiles()->indexOf(page->inputFile());
        if (n>-1)
        {
            item(n)->setSelected(true);
            return;
        }

//        for (int j=0; j<project->filesCount(); ++j)
//        {
//            if (project->file(j) == page->inputFile())
//            {
//                item(j)->setSelected(true);
//                return;
//            }
//        }
    }

    this->clearSelection();
}


/************************************************

 ************************************************/
void InputFilesListView::updateItems()
{
    clear();
    for (int i=0; i<project->filesCount(); ++i)
    {
        QListWidgetItem *item = new QListWidgetItem(this);
        item->setData(Qt::UserRole, i);
        InputFile *file = project->file(i);

        item->setText(tr("( %1 pages ) ").arg(file->pageCount()) +
                    (file->title().isEmpty() ? tr("Untitled") : file->title()));

        addItem(item);
    }
}


/************************************************

 ************************************************/
void InputFilesListView::showContextMenu(const QPoint &pos)
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
void InputFilesListView::deleteFile()
{
    QAction *act = qobject_cast<QAction*>(sender());
    if (act)
    {
        int index = act->data().toInt();
        project->removeFile(index);
    }
    updateItems();
}


/************************************************

 ************************************************/
void InputFilesListView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QListWidgetItem *item = this->itemAt(event->pos());
        if (item)
        {
            setCurrentItem(item);
            int n = item->data(Qt::UserRole).toInt();
            if (n<project->filesCount())
            {
                emit fileSelected(project->file(n));
            }
        }
    }
    QListWidget::mouseReleaseEvent(event);
}


/************************************************

 ************************************************/
void InputFilesListView::layoutChanged()
{
    for (int i=0; i<count()-1; ++i)
    {
        int from = item(i)->data(Qt::UserRole).toInt();
        if (from != i)
            project->moveFile(from, i);
    }
}



