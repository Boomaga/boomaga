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

#include "printersettings.h"
#include "ui_printersettings.h"

#include <QStandardItem>
#include <QPainter>
#include <QDebug>
#include <QMessageBox>
#include "settings.h"


class ProfileItem: public QStandardItem
{
public:
    explicit ProfileItem(const PrinterProfile &profile):
        QStandardItem(profile.name()),
        mProfile(profile)
    {
        setFlags(flags() | Qt::ItemIsEditable);
    }

    explicit ProfileItem():
        QStandardItem()
    {
        setFlags(flags() | Qt::ItemIsEditable);
    }

    virtual ~ProfileItem() { }
    PrinterProfile *profile() { return &mProfile; }

private:
    PrinterProfile mProfile;
};


/************************************************

 ************************************************/
PrinterSettings *PrinterSettings::execute(Printer *printer)
{
    PrinterSettings *instance = findExistingForm<PrinterSettings>();
    QRect geometry;


    if (instance && (instance->currentPrinter() != printer))
    {
        geometry = instance->geometry();
        instance->deleteLater();
        instance = 0;
    }

    if (!instance)
    {
        instance = new PrinterSettings(qApp->activeWindow());
        if (!geometry.isNull())
            instance->setGeometry(geometry);

        instance->setCurrentPrinter(printer);
    }

    instance->show();
    instance->raise();
    instance->activateWindow();
    return instance;
}


/************************************************

 ************************************************/
PrinterSettings::PrinterSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrinterSettings),
    mPrinter(nullptr),
    mUnit(UnitMillimeter)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    ui->profilesList->setModel(new QStandardItemModel(this));

    if (settings->value(Settings::AllowNegativeMargins).toBool())
    {
        ui->leftMarginSpin->setMinimum(-(ui->leftMarginSpin->maximum()));
        ui->rightMarginSpin->setMinimum(-(ui->rightMarginSpin->maximum()));
        ui->topMarginSpin->setMinimum(-(ui->topMarginSpin->maximum()));
        ui->bottomMarginSpin->setMinimum(-(ui->bottomMarginSpin->maximum()));
        ui->internalMarginSpin->setMinimum(-(ui->internalMarginSpin->maximum()));
    }

    ui->leftMarginSpin->installEventFilter(this);
    ui->rightMarginSpin->installEventFilter(this);
    ui->topMarginSpin->installEventFilter(this);
    ui->bottomMarginSpin->installEventFilter(this);
    ui->internalMarginSpin->installEventFilter(this);
    ui->duplexTypeComboBox->addItem(tr("Printer has duplexer"), DuplexAuto);
    ui->duplexTypeComboBox->addItem(tr("Manual with reverse (suitable for most printers)"), DuplexManualReverse);
    ui->duplexTypeComboBox->addItem(tr("Manual without reverse"), DuplexManual);

    ui->colorModeCombo->addItem(tr("Default"), ColorModeAuto);
    ui->colorModeCombo->addItem(tr("Force grayscale"), ColorModeGrayscale);
    ui->colorModeCombo->addItem(tr("Force color"), ColorModeColor);


    QPalette pal(palette());
    pal.setColor(QPalette::Background, QColor(105, 101, 98));
    ui->marginsPereview->setPalette(pal);
    ui->marginsPereview->setAutoFillBackground(true);
    ui->marginsPereview->installEventFilter(this);

    int n = qMax(ui->addProfileButton->minimumSizeHint().width(), ui->addProfileButton->minimumSizeHint().height());
    ui->addProfileButton->setMinimumSize(n, n);
    ui->delProfileButton->setMinimumSize(n, n);

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),
            this, SLOT(btnClicked(QAbstractButton*)));

    connect(ui->borderCbx, SIGNAL(toggled(bool)),
            this, SLOT(updatePreview()));

    connect(ui->profilesList->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(updateWidgets()));

    connect(ui->profilesList->itemDelegate(), SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
            this, SLOT(profileRenamed(QWidget*)));

    connect(ui->addProfileButton, SIGNAL(clicked()),
            this, SLOT(addProfile()));

    connect(ui->delProfileButton, SIGNAL(clicked()),
            this, SLOT(delProfile()));

    connect(ui->leftMarginSpin, SIGNAL(editingFinished()),
            this, SLOT(updateProfile()));

    connect(ui->rightMarginSpin, SIGNAL(editingFinished()),
            this, SLOT(updateProfile()));

    connect(ui->topMarginSpin, SIGNAL(editingFinished()),
            this, SLOT(updateProfile()));

    connect(ui->bottomMarginSpin, SIGNAL(editingFinished()),
            this, SLOT(updateProfile()));

    connect(ui->internalMarginSpin, SIGNAL(editingFinished()),
            this, SLOT(updateProfile()));

    connect(ui->duplexTypeComboBox, SIGNAL(activated(int)),
            this, SLOT(updateProfile()));

    connect(ui->duplexTypeComboBox, SIGNAL(activated(int)),
            this, SLOT(updateWidgets()));

    connect(ui->reverseOrderCbx, SIGNAL(clicked()),
            this, SLOT(updateProfile()));

    connect(ui->borderCbx, SIGNAL(clicked()),
            this, SLOT(updateProfile()));

    connect(ui->resetButton, SIGNAL(clicked()),
            this, SLOT(resetToDefault()));

    connect(ui->flipLongEdgeCheck, SIGNAL(clicked(bool)),
            this, SLOT(updateProfile()));

    connect(ui->flipShortEdgeCheck, SIGNAL(clicked(bool)),
            this, SLOT(updateProfile()));

    restoreGeometry(settings->value(Settings::PrinterSettingsDialog_Geometry).toByteArray());
}


/************************************************

 ************************************************/
PrinterSettings::~PrinterSettings()
{
    settings->setValue(Settings::PrinterSettingsDialog_Geometry, saveGeometry());
    delete ui;
}


/************************************************

 ************************************************/
void PrinterSettings::setCurrentPrinter(Printer *printer)
{
    mPrinter = printer;
    setWindowTitle(tr("Preferences of \"%1\"").arg(mPrinter->name()));
    ui->duplexTypeComboBox->setEnabled(printer->canChangeDuplexType());


    QStandardItem *root = new QStandardItem(tr("Profiles"));
    root->setFlags(root->flags() & (~Qt::ItemIsSelectable));
    root->setFlags(Qt::NoItemFlags);
    QFont font = root->font();
    font.setBold(true);
    root->setFont(font);
    root->setSizeHint(QSize(0, 30));

    static_cast<QStandardItemModel*>(ui->profilesList->model())
        ->invisibleRootItem()->appendRow(root);

    for (int i=0; i<mPrinter->profiles().count(); ++i )
    {
        ProfileItem *item = new ProfileItem(mPrinter->profiles().at(i));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        root->appendRow(item);
        if (i == mPrinter->currentProfileIndex())
        {
            ui->profilesList->setCurrentIndex(item->index());
        }

    }

    ui->profilesList->expandAll();
    updateWidgets();
}


/************************************************

 ************************************************/
bool PrinterSettings::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
    {
        if (obj == ui->leftMarginSpin ||
            obj == ui->rightMarginSpin ||
            obj == ui->topMarginSpin ||
            obj == ui->bottomMarginSpin ||
            obj == ui->internalMarginSpin)
            updatePreview();
    }

    if (obj == ui->marginsPereview)
    {
        if (event->type() == QEvent::Resize)
            updatePreview();
    }

    return QDialog::eventFilter(obj, event);
}


/************************************************
 *
 ************************************************/
void PrinterSettings::updateProfile()
{
    PrinterProfile *profile = currentProfile();
    if (!profile)
        return;

    profile->setName(currentItem()->text());
    profile->setLeftMargin(ui->leftMarginSpin->value(), mUnit);
    profile->setRightMargin(ui->rightMarginSpin->value(), mUnit);
    profile->setTopMargin(ui->topMarginSpin->value(), mUnit);
    profile->setBottomMargin(ui->bottomMarginSpin->value(), mUnit);
    profile->setInternalMargin(ui->internalMarginSpin->value(), mUnit);

    QVariant v =ui->duplexTypeComboBox->itemData(ui->duplexTypeComboBox->currentIndex());
    profile->setDuplexType(static_cast<DuplexType>(v.toInt()));
    profile->setDrawBorder(ui->borderCbx->isChecked());
    profile->setReverseOrder(ui->reverseOrderCbx->isChecked());

    v = ui->colorModeCombo->itemData(ui->colorModeCombo->currentIndex());
    profile->setColorMode(static_cast<ColorMode>(v.toInt()));
    if (ui->flipShortEdgeCheck->isChecked())
        profile->setFlipType(FlipType::ShortEdge);
    else
        profile->setFlipType(FlipType::LongEdge);
}


/************************************************
 *
 ************************************************/
void PrinterSettings::updateWidgets()
{
    PrinterProfile *profile = currentProfile();
    if (profile == nullptr)
        return;



    int n = ui->duplexTypeComboBox->findData(profile->duplexType());
    ui->duplexTypeComboBox->setCurrentIndex(n);
    ui->borderCbx->setChecked(profile->drawBorder());
    ui->reverseOrderCbx->setChecked(profile->reverseOrder());
    ui->leftMarginSpin->setValue(profile->leftMargin(mUnit));
    ui->rightMarginSpin->setValue(profile->rightMargin(mUnit));
    ui->topMarginSpin->setValue(profile->topMargin(mUnit));
    ui->bottomMarginSpin->setValue(profile->bottomMargin(mUnit));
    ui->internalMarginSpin->setValue(profile->internalMargin(mUnit));

    bool enable = ui->duplexTypeComboBox->currentData().toInt() == DuplexAuto;
    ui->flipLongEdgeCheck->setEnabled(enable);
    ui->flipShortEdgeCheck->setEnabled(enable);

    if (mPrinter->isSupportColor())
    {
        ui->colorModeCombo->setEnabled(true);
        n = ui->colorModeCombo->findData(profile->colorMode());
        ui->colorModeCombo->setCurrentIndex(n);
    }
    else
    {
        ui->colorModeCombo->setEnabled(false);
        ui->colorModeCombo->setCurrentIndex(0);
    }

    ui->flipLongEdgeCheck-> setChecked(profile->flipType() == FlipType::LongEdge);
    ui->flipShortEdgeCheck->setChecked(profile->flipType() == FlipType::ShortEdge);

    updatePreview();
}


/************************************************
 *
 ************************************************/
void PrinterSettings::addProfile()
{
    updateProfile();

    ProfileItem *cur = currentItem();
    if (!cur)
        return;

    QStandardItem *parent = cur->parent();

    ProfileItem *item =new ProfileItem();
    item->setText(tr("Profile %1", "Defaul name for created printer profile in the Printer Settings diaog")
                  .arg(parent->rowCount()));
    parent->insertRow(cur->row()+1, item);

    ui->profilesList->setCurrentIndex(item->index());
    ui->profilesList->edit(item->index());
}


/************************************************
 *
 ************************************************/
void PrinterSettings::delProfile()
{
    ProfileItem *cur = currentItem();
    if (!cur)
        return;

    QStandardItem *parent = cur->parent();
    if (parent->rowCount() == 1)
    {
        QMessageBox::warning(this, windowTitle(), "I can't remove last profile.");
        return;
    }

    parent->removeRow(cur->row());
    updateWidgets();
}


/************************************************
 *
 ************************************************/
void PrinterSettings::profileRenamed(QWidget *editor)
{
    ProfileItem *item = currentItem();
    if (item)
        item->profile()->setName(item->text());
}


/************************************************
 *
 ************************************************/
void PrinterSettings::resetToDefault()
{
    PrinterProfile *profile = currentProfile();
    if (!profile)
        return;

    profile->setTopMargin(     currentPrinter()->defaultCupsProfile()->topMargin(mUnit),      mUnit);
    profile->setBottomMargin(  currentPrinter()->defaultCupsProfile()->bottomMargin(mUnit),   mUnit);
    profile->setLeftMargin(    currentPrinter()->defaultCupsProfile()->leftMargin(mUnit),     mUnit);
    profile->setRightMargin(   currentPrinter()->defaultCupsProfile()->rightMargin(mUnit),    mUnit);
    profile->setInternalMargin(currentPrinter()->defaultCupsProfile()->internalMargin(mUnit), mUnit);
    updateWidgets();
}


/************************************************

 ************************************************/
void PrinterSettings::applyUpdates()
{
    ProfileItem *item = currentItem();
    if (!item)
        return;

    QVector<PrinterProfile> profiles;
    QStandardItem *root = item->parent();
    for (int i=0; i< root->rowCount(); ++i)
    {
        profiles << *(static_cast<ProfileItem*>(root->child(i))->profile());
    }

    mPrinter->setProfiles(profiles);
    mPrinter->setCurrentProfile(item->row());
}


/************************************************
 *
 ************************************************/
PrinterProfile *PrinterSettings::currentProfile() const
{
    ProfileItem *item = currentItem();
    if (item)
         return item->profile();

    return nullptr;
}


/************************************************
 *
 ************************************************/
ProfileItem *PrinterSettings::currentItem() const
{
    QModelIndex idx = ui->profilesList->currentIndex();
    if (!idx.parent().isValid())
        return nullptr;

    return static_cast<ProfileItem*>(
        static_cast<QStandardItemModel*>(ui->profilesList->model())
            ->itemFromIndex(idx));
}


/************************************************

 ************************************************/
void PrinterSettings::save()
{
    updateProfile();
    applyUpdates();
    emit accepted();
}


/************************************************

 ************************************************/
void PrinterSettings::btnClicked(QAbstractButton *button)
{
    if (ui->buttonBox->standardButton(button) == QDialogButtonBox::Ok ||
        ui->buttonBox->standardButton(button) == QDialogButtonBox::Apply)
    {
        save();

        if (ui->buttonBox->standardButton(button) == QDialogButtonBox::Ok)
            close();
    }

    if (ui->buttonBox->standardButton(button) == QDialogButtonBox::Cancel)
        close();
}


/************************************************

 ************************************************/
void PrinterSettings::updatePreview()
{
    QRect rectImg(QPoint(0, 0), ui->marginsPereview->size());
    QImage img(rectImg.size(),  QImage::Format_ARGB32);
    img.fill(Qt::transparent);

    QPainter painter(&img);

    QRectF rect1Up(0.5, 5.5, 70.5, 100.5);
    QRectF rect2Up(0.5, 5.5, 101.5, 70.5);

    rect1Up.moveCenter(ui->marginsPereview->rect().center());
    rect1Up.moveLeft((rectImg.width() - rect1Up.width() - rect2Up.width()) / 3.0);
    rect2Up.moveCenter(ui->marginsPereview->rect().center());
    rect2Up.moveRight(rectImg.right() - rect1Up.left());

    painter.fillRect(rect1Up, Qt::white);
    painter.fillRect(rect2Up, Qt::white);

    QRectF rect1UpPage = rect1Up.adjusted(3.5, 3.5, -3.5, -3.5);

    QRectF rect2UpPage1 = rect2Up.adjusted(3.5, 3.5, -3.5, -3.5);
    rect2UpPage1.setRight(rect2Up.center().x() - 2);

    QRectF rect2UpPage2 = rect2Up.adjusted(3.5, 3.5, -3.5, -3.5);
    rect2UpPage2.setLeft(rect2Up.center().x() + 4);

    QPen pen = painter.pen();
    if (ui->borderCbx->isChecked())
    {
        pen.setColor(Qt::lightGray);
        pen.setStyle(Qt::SolidLine);
    }
    else
    {
        pen.setColor(Qt::lightGray);
        pen.setStyle(Qt::DotLine);
    }
    painter.setPen(pen);

    painter.drawRect(rect1UpPage);
    painter.drawRect(rect2UpPage1);
    painter.drawRect(rect2UpPage2);


    QColor mark("#CC7373");

    if (ui->topMarginSpin->hasFocus())
    {
        QRectF rect = rect1UpPage;
        rect.setHeight(3);
        rect.moveTop(rect1Up.top());
        painter.fillRect(rect, mark);

        rect = rect2UpPage1;
        rect.setWidth(3);
        rect.moveLeft(rect2Up.left());
        painter.fillRect(rect, mark);
    }

    if (ui->bottomMarginSpin->hasFocus())
    {
        QRectF rect = rect1UpPage;
        rect.setHeight(3);
        rect.moveBottom(rect1Up.bottom());
        painter.fillRect(rect, mark);

        rect = rect2UpPage1;
        rect.setWidth(3);
        rect.moveRight(rect2Up.right());
        painter.fillRect(rect, mark);
    }

    if (ui->leftMarginSpin->hasFocus())
    {
        QRectF rect = rect1UpPage;
        rect.setWidth(3);
        rect.moveLeft(rect1Up.left());
        painter.fillRect(rect, mark);

        rect = rect2UpPage1;
        rect.setHeight(3);
        rect.moveBottom(rect2Up.bottom());
        painter.fillRect(rect, mark);

        rect = rect2UpPage2;
        rect.setHeight(3);
        rect.moveBottom(rect2Up.bottom());
        painter.fillRect(rect, mark);
    }


    if (ui->rightMarginSpin->hasFocus())
    {
        QRectF rect = rect1UpPage;
        rect.setWidth(3);
        rect.moveRight(rect1Up.right());
        painter.fillRect(rect, mark);

        rect = rect2UpPage1;
        rect.setHeight(3);
        rect.moveTop(rect2Up.top());
        painter.fillRect(rect, mark);

        rect = rect2UpPage2;
        rect.setHeight(3);
        rect.moveTop(rect2Up.top());
        painter.fillRect(rect, mark);
    }

    if (ui->internalMarginSpin->hasFocus())
    {
        QRectF rect;
        rect.setTop(rect2UpPage1.top());
        rect.setLeft(rect2UpPage1.right());
        rect.setBottom(rect2UpPage1.bottom());
        rect.setRight(rect2Up.center().x());
        painter.fillRect(rect, mark);

        rect.setTop(rect2UpPage2.top());
        rect.setLeft(rect2Up.center().x() + 1);
        rect.setBottom(rect2UpPage2.bottom());
        rect.setRight(rect2UpPage2.left()-1);
        painter.fillRect(rect, mark);
    }

    ui->marginsPereview->setPixmap(QPixmap::fromImage(img));
}
