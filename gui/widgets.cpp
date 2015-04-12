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
#include "kernel/job.h"
#include "kernel/sheet.h"

#include <QMenu>
#include <QMouseEvent>
#include <QStandardItem>
#include <QDebug>


#define CUSTOM_PROFILE_DELEGATE
#ifdef CUSTOM_PROFILE_DELEGATE
#include <QStyledItemDelegate>
#include <QPen>
#include <QPainter>
#include <QApplication>
#endif
#define PRINTER_ITEM_TEXT_ROLE      Qt::UserRole + 1
#define PRINTER_ITEM_PRINTER_ROLE   Qt::UserRole + 2
#define PRINTER_ITEM_PROFILE_ROLE   Qt::UserRole + 3


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


#ifdef CUSTOM_PROFILE_DELEGATE

class PrintersComboBoxDelegate: public QStyledItemDelegate
{
public:
    explicit PrintersComboBoxDelegate(QComboBox *cmb, QWidget *parent = 0):  QStyledItemDelegate(parent),  mCombo(cmb) {}
    void paint(QPainter *painter,  const QStyleOptionViewItem &option,  const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool isSeparator(const QModelIndex &index) const;
    QStyleOptionMenuItem getStyleOption(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    QComboBox *mCombo;
};


/************************************************
 *
 ************************************************/
bool PrintersComboBoxDelegate::isSeparator(const QModelIndex &index) const
{
    return index.data(Qt::AccessibleDescriptionRole).toString() == QLatin1String("separator");
}


/************************************************
 *
 ************************************************/
QSize PrintersComboBoxDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize res = QStyledItemDelegate::sizeHint(option, index);

    if (isSeparator(index))
    {
        int pm = mCombo->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, 0, mCombo);
        res.setHeight(pm);
    }

    return res;
}


/************************************************
 *
 ************************************************/
void PrintersComboBoxDelegate::paint(QPainter *painter,
           const QStyleOptionViewItem &option,
           const QModelIndex &index) const
{
    QStyleOptionMenuItem opt = getStyleOption(option, index);
    painter->fillRect(option.rect, opt.palette.background());

    QString type = index.data( Qt::AccessibleDescriptionRole ).toString();

    if (type == QLatin1String("printer"))
    {
        opt.state |= QStyle::State_Enabled;
        opt.font.setBold(true);
    }
    else if (type == QLatin1String("profile"))
    {
        opt.text = "    " + opt.text;
    }

    mCombo->style()->drawControl(QStyle::CE_MenuItem, &opt, painter, mCombo);
}


/************************************************
 *
 ************************************************/
QStyleOptionMenuItem PrintersComboBoxDelegate::getStyleOption(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionMenuItem menuOption;
    menuOption.menuHasCheckableItems = false;
    menuOption.checkType = QStyleOptionMenuItem::NotCheckable;

    QPalette resolvedpalette = option.palette.resolve(QApplication::palette("QMenu"));
    QVariant value = index.data(Qt::ForegroundRole);
    if (value.canConvert<QBrush>())
    {
        resolvedpalette.setBrush(QPalette::WindowText, qvariant_cast<QBrush>(value));
        resolvedpalette.setBrush(QPalette::ButtonText, qvariant_cast<QBrush>(value));
        resolvedpalette.setBrush(QPalette::Text, qvariant_cast<QBrush>(value));
    }

    menuOption.palette = resolvedpalette;
    menuOption.state = QStyle::State_None;
    menuOption.tabWidth = 0;
    menuOption.maxIconWidth =  0;

    if (mCombo->window()->isActiveWindow())
        menuOption.state = QStyle::State_Active;

    if ((option.state & QStyle::State_Enabled) && (index.model()->flags(index) & Qt::ItemIsEnabled))
        menuOption.state |= QStyle::State_Enabled;
    else
        menuOption.palette.setCurrentColorGroup(QPalette::Disabled);

    if (option.state & QStyle::State_Selected)
        menuOption.state |= QStyle::State_Selected;


    if (isSeparator(index))
        menuOption.menuItemType = QStyleOptionMenuItem::Separator;
    else
        menuOption.menuItemType = QStyleOptionMenuItem::Normal;


    if (index.data(Qt::BackgroundRole).canConvert<QBrush>())
    {
        menuOption.palette.setBrush(QPalette::All, QPalette::Background,
                                    qvariant_cast<QBrush>(index.data(Qt::BackgroundRole)));
    }

    menuOption.text = index.model()->data(index, PRINTER_ITEM_TEXT_ROLE).toString()
                           .replace(QLatin1Char('&'), QLatin1String("&&"));

    menuOption.menuRect = option.rect;
    menuOption.rect = option.rect;
    menuOption.font = mCombo->font();
    menuOption.fontMetrics = QFontMetrics(menuOption.font);

    return menuOption;
}
#endif


class PrintersComboBoxItem: public QStandardItem
{
public:
    PrintersComboBoxItem(const QString &text):
        QStandardItem(text),
        mPrinter(0),
        mProfile(0)
    {}

    Printer *printer() const { return mPrinter; }
    void setPrinter(Printer *value) { mPrinter = value;}

    int profile() const { return mProfile; }
    void setProfile(int value) { mProfile = value; }
private:
    Printer *mPrinter;
    int mProfile;
};


/************************************************

 ************************************************/
PrintersComboBox::PrintersComboBox(QWidget *parent):
    QComboBox(parent)
{
#ifdef CUSTOM_PROFILE_DELEGATE
    setItemDelegate(new PrintersComboBoxDelegate(this, this));
#endif
}


/************************************************

 ************************************************/
PrintersComboBox::~PrintersComboBox()
{
}


/************************************************

 ************************************************/
void PrintersComboBox::setCurrentPrinterProfile(const QString &printerName, int profileIndex)
{
    setCurrentIndex(findItem(printerName, profileIndex));
}


/************************************************
 *
 ************************************************/
void PrintersComboBox::setCurrentPrinterProfile(const Printer *printer, int profileIndex)
{
    if (printer)
        setCurrentPrinterProfile(printer->name(), profileIndex);
    else
        setCurrentIndex(-1);
}


/************************************************

 ************************************************/
int PrintersComboBox::addPrinter(Printer *printer)
{
    QStandardItem *item = new QStandardItem(printer->name());
    item->setData(printer->name(), PRINTER_ITEM_TEXT_ROLE);
    item->setData(qVariantFromValue((void *)printer), PRINTER_ITEM_PRINTER_ROLE);
    item->setData(-1, PRINTER_ITEM_PROFILE_ROLE);

    item->setFlags( item->flags() & ~( Qt::ItemIsEnabled | Qt::ItemIsSelectable ) );
    item->setData("printer", Qt::AccessibleDescriptionRole );

    QFont font = item->font();
    font.setBold( true );
    item->setFont(font);

    QStandardItemModel* itemModel = (QStandardItemModel*)model();
    itemModel->appendRow(item);

    for (int i=0; i<printer->profiles()->count(); ++i)
        addProfile(printer, i);

    return this->count() - 1;
}


/************************************************
 *
 ************************************************/
int PrintersComboBox::addProfile(Printer *printer, int profileIndex)
{
    const PrinterProfile &profile = printer->profiles()->at(profileIndex);

    QStandardItem *item = new QStandardItem(printer->name() + " (" + profile.name() + ") ");
    item->setData(profile.name(), PRINTER_ITEM_TEXT_ROLE);
    item->setData(qVariantFromValue((void *)printer), PRINTER_ITEM_PRINTER_ROLE);
    item->setData(profileIndex, PRINTER_ITEM_PROFILE_ROLE);


    item->setData("profile", Qt::AccessibleDescriptionRole );

    QStandardItemModel* itemModel = (QStandardItemModel*)model();
    itemModel->appendRow(item);
    return this->count() - 1;
}


/************************************************

 ************************************************/
Printer *PrintersComboBox::itemPrinter(int index) const
{
    if (index < 0)
        return 0;

    QStandardItemModel* itemModel = (QStandardItemModel*)model();
    QVariant v = itemModel->index(index, 0).data(PRINTER_ITEM_PRINTER_ROLE);

    if (v.isNull())
        return 0;
    else
        return static_cast<Printer*>(v.value<void *>());
}


/************************************************
 *
 ************************************************/
int PrintersComboBox::itemProfile(int index) const
{
    if (index < 0)
        return -1;

    QStandardItemModel* itemModel = (QStandardItemModel*)model();
    QVariant v = itemModel->index(index, 0).data(PRINTER_ITEM_PROFILE_ROLE);

    bool ok;
    int res = v.toInt(&ok);

    if (ok)
        return res;
    else
        return -1;
}


/************************************************
 *
 ************************************************/
int PrintersComboBox::findItem(const QString &printerName, int profileIndex) const
{
    for (int i=0; i<count(); ++i)
    {
        if (itemPrinter(i) &&
            itemPrinter(i)->name() == printerName &&
            itemProfile(i) == profileIndex )
        {
            return i;
        }
    }
    return -1;
}


/************************************************
 *
 ************************************************/
int PrintersComboBox::findItem(const Printer *printer, int profileIndex) const
{
    if (printer)
        return findItem(printer->name(), profileIndex);
    else
        return -1;
}


/************************************************

 ************************************************/
JobsListView::JobsListView(QWidget *parent):
    QListWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showContextMenu(QPoint)));

    connect(this->model(), SIGNAL(layoutChanged()),
            this, SLOT(layoutChanged()));

    connect(project, SIGNAL(changed()),
            this, SLOT(updateItems()));

    updateItems();
}


/************************************************
 *
 * ***********************************************/
Job JobsListView::currentJob() const
{
    if (currentItem())
    {
        int n = currentItem()->data(Qt::UserRole).toInt();
        if (n < project->jobs()->count())
            return project->jobs()->at(n);
    }

    return Job();
}


/************************************************

 ************************************************/
void JobsListView::setSheetNum(int sheetNum)
{
    if (count() < 1)
        return;

    if (sheetNum < 0)
    {
        item(0)->setSelected(true);
        return;
    }

    Sheet *sheet = project->previewSheet(sheetNum);
    Job job = currentJob();

    if (job.state() != Job::JobEmpty)
    {
        for (int i=0; i<sheet->count(); ++i)
        {
            ProjectPage *page = sheet->page(i);
            if (!page)
                continue;

            if (job.indexOfPage(page) > -1)
                return;
        }
    }


    for (int i=0; i<sheet->count(); ++i)
    {
        ProjectPage *page = sheet->page(i);
        if (!page)
            continue;

        for(int j=0; j<project->jobs()->count(); ++j)
        {
            Job job = project->jobs()->at(j);

            if (job.indexOfPage(page) > -1)
            {
                setCurrentItem(item(j));
                return;
            }
        }
    }

    this->clearSelection();
}


/************************************************

 ************************************************/
void JobsListView::updateItems()
{
    this->setUpdatesEnabled(false);
    clear();
    for (int i=0; i<project->jobs()->count(); ++i)
    {
        QListWidgetItem *item = new QListWidgetItem(this);
        item->setData(Qt::UserRole, i);
        Job job = project->jobs()->at(i);

        item->setText(tr("( %1 pages )").arg(job.visiblePageCount()) + " " + job.title());

        addItem(item);
    }
    this->setUpdatesEnabled(true);
}


/************************************************

 ************************************************/
void JobsListView::showContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = itemAt(pos);
    if (!item)
        return;

    bool ok;
    int n = item->data(Qt::UserRole).toInt(&ok);
    if (ok && n>-1 && n<project->jobs()->count())
        emit contextMenuRequested(project->jobs()->at(n));
}


/************************************************

 ************************************************/
void JobsListView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QListWidgetItem *item = this->itemAt(event->pos());
        if (item)
        {
            setCurrentItem(item);
            int n = item->data(Qt::UserRole).toInt();
            if (n < project->jobs()->count())
            {
                emit jobSelected(project->jobs()->at(n));
            }
        }
    }

    QListWidget::mouseReleaseEvent(event);
}


/************************************************

 ************************************************/
void JobsListView::layoutChanged()
{
    for (int i=0; i<count()-1; ++i)
    {
        int from = item(i)->data(Qt::UserRole).toInt();
        if (from != i)
            project->moveJob(from, i);
    }
}



