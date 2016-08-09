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

#include <QListWidget>
#include <QListWidgetItem>
#include <QApplication>
#include <QAbstractButton>
#include <QPainter>
#include <QRect>
#include <QDebug>
#include <QMessageBox>
#include "settings.h"


class ProfileItem: public QListWidgetItem
{
public:
    explicit ProfileItem(const PrinterProfile &profile):
        QListWidgetItem(),
        mProfile(profile),
        mIsCurrent(false)
    {
        setFlags(flags() | Qt::ItemIsEditable);
        setText(profile.name());
    }

    virtual ~ProfileItem() { }

    PrinterProfile *profile() { return &mProfile; }

    bool isCurrent() const { return mIsCurrent; }
    void setIsCurrent(bool value) { mIsCurrent = value; }

private:
    PrinterProfile mProfile;
    bool mIsCurrent;
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
    mPrinter(0),
    mUnit(UnitMillimeter)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    ui->leftMarginSpin->installEventFilter(this);
    ui->rightMarginSpin->installEventFilter(this);
    ui->topMarginSpin->installEventFilter(this);
    ui->bottomMarginSpin->installEventFilter(this);
    ui->internalMarginSpin->installEventFilter(this);
    ui->duplexTypeComboBox->addItem(tr("Printer has duplexer"), DuplexAuto);
    ui->duplexTypeComboBox->addItem(tr("Manual with reverse (suitable for most printers)"), DuplexManualReverse);
    ui->duplexTypeComboBox->addItem(tr("Manual without reverse"), DuplexManual);


    QPalette pal(palette());
    pal.setColor(QPalette::Background, QColor(105, 101, 98));
    ui->marginsPereview->setPalette(pal);
    ui->marginsPereview->setAutoFillBackground(true);

    int n = qMax(ui->addProfileButton->minimumSizeHint().width(), ui->addProfileButton->minimumSizeHint().height());
    ui->addProfileButton->setMinimumSize(n, n);
    ui->delProfileButton->setMinimumSize(n, n);

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(btnClicked(QAbstractButton*)));
    connect(ui->borderCbx, SIGNAL(toggled(bool)), this, SLOT(updatePreview()));

    connect(ui->profilesList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(selectProfile(QListWidgetItem*,QListWidgetItem*)));

    connect(ui->profilesList->itemDelegate(), SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
            this, SLOT(profileRenamed(QWidget*,QAbstractItemDelegate::EndEditHint)));

    connect(ui->addProfileButton, SIGNAL(clicked()), this, SLOT(addProfile()));
    connect(ui->delProfileButton, SIGNAL(clicked()), this, SLOT(delProfile()));

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

    connect(ui->reverseOrderCbx, SIGNAL(clicked()),
            this, SLOT(updateProfile()));

    connect(ui->borderCbx, SIGNAL(clicked()),
            this, SLOT(updateProfile()));

    connect(ui->resetButton, SIGNAL(clicked()),
            this, SLOT(resetToDefault()));

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


    ui->profilesList->clear();
    foreach (const PrinterProfile &profile, *(mPrinter->profiles()))
    {
        ui->profilesList->addItem(new ProfileItem(profile));
    }

    ui->profilesList->setCurrentRow(mPrinter->currentProfile());
    currentItem()->setIsCurrent(true);

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

    return QDialog::eventFilter(obj, event);
}


/************************************************
 *
 ************************************************/
void PrinterSettings::updateProfile()
{
    PrinterProfile *profile = currentProfile();

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
}


/************************************************
 *
 ************************************************/
void PrinterSettings::updateWidgets()
{
    PrinterProfile *profile = currentProfile();

    int n = ui->duplexTypeComboBox->findData(profile->duplexType());
    ui->duplexTypeComboBox->setCurrentIndex(n);
    ui->borderCbx->setChecked(profile->drawBorder());
    ui->reverseOrderCbx->setChecked(profile->reverseOrder());
    ui->leftMarginSpin->setValue(profile->leftMargin(mUnit));
    ui->rightMarginSpin->setValue(profile->rightMargin(mUnit));
    ui->topMarginSpin->setValue(profile->topMargin(mUnit));
    ui->bottomMarginSpin->setValue(profile->bottomMargin(mUnit));
    ui->internalMarginSpin->setValue(profile->internalMargin(mUnit));
    ui->delProfileButton->setEnabled(ui->profilesList->count() > 1);
}


/************************************************
 *
 ************************************************/
void PrinterSettings::selectProfile(QListWidgetItem * current, QListWidgetItem * previous)
{
    updateWidgets();
}


/************************************************
 *
 ************************************************/
void PrinterSettings::addProfile()
{
    updateProfile();

    ProfileItem *item = new ProfileItem(*(currentItem()->profile()));
    item->setText(tr("Profile %1", "Defaul name for created printer profile in the Printer Settings diaog").arg(ui->profilesList->count() + 1));
    item->profile()->setName(item->text());
    ui->profilesList->addItem(item);
    ui->profilesList->setCurrentItem(item);
    ui->profilesList->editItem(item);
}


/************************************************
 *
 ************************************************/
void PrinterSettings::delProfile()
{
    if (ui->profilesList->count() == 1)
    {
        QMessageBox::warning(this, windowTitle(), "I can't remove last profile.");
        return;
    }

    delete currentItem();
    updateWidgets();
}


/************************************************
 *
 ************************************************/
void PrinterSettings::profileRenamed(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    currentProfile()->setName(ui->profilesList->currentItem()->text());
}


/************************************************
 *
 ************************************************/
void PrinterSettings::resetToDefault()
{
    currentProfile()->setTopMargin(     currentPrinter()->cupsProfile()->topMargin(mUnit),      mUnit);
    currentProfile()->setBottomMargin(  currentPrinter()->cupsProfile()->bottomMargin(mUnit),   mUnit);
    currentProfile()->setLeftMargin(    currentPrinter()->cupsProfile()->leftMargin(mUnit),     mUnit);
    currentProfile()->setRightMargin(   currentPrinter()->cupsProfile()->rightMargin(mUnit),    mUnit);
    currentProfile()->setInternalMargin(currentPrinter()->cupsProfile()->internalMargin(mUnit), mUnit);
    updateWidgets();
}


/************************************************

 ************************************************/
void PrinterSettings::applyUpdates()
{
    bool curIsSet = false;
    mPrinter->profiles()->clear();

    for(int i=0; i< ui->profilesList->count(); ++i)
    {
        ProfileItem *item = static_cast<ProfileItem*>(ui->profilesList->item(i));
        mPrinter->profiles()->append(*(item->profile()));

        if (item->isCurrent())
        {
            mPrinter->setCurrentProfile(i);
            curIsSet = true;
        }
    }


    if (!curIsSet)
        mPrinter->setCurrentProfile(0);
}


/************************************************

 ************************************************/
ProfileItem *PrinterSettings::currentItem() const
{
    return static_cast<ProfileItem*>(ui->profilesList->currentItem());
}


/************************************************
 *
 ************************************************/
PrinterProfile *PrinterSettings::currentProfile() const
{
    return currentItem()->profile();
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

    QRectF rect1UpPage = rect1Up.adjusted(3, 3, -3, -3);

    QRectF rect2UpPage1 = rect2Up.adjusted(3, 3, -3, -3);
    rect2UpPage1.setRight(rect2Up.center().x() - 2);

    QRectF rect2UpPage2 = rect2Up.adjusted(3, 3, -3, -3);
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
        QRectF rect;
        rect.setTop(rect1Up.top());
        rect.setLeft(rect1UpPage.left());
        rect.setBottom(rect1UpPage.top());
        rect.setRight(rect1UpPage.right());
        painter.fillRect(rect, mark);

        rect.setTop(rect2UpPage1.top());
        rect.setLeft(rect2Up.left());
        rect.setBottom(rect2UpPage1.bottom());
        rect.setRight(rect2UpPage1.left());
        painter.fillRect(rect, mark);
    }

    if (ui->bottomMarginSpin->hasFocus())
    {
        QRectF rect;
        rect.setTop(rect1UpPage.bottom());
        rect.setLeft(rect1UpPage.left());
        rect.setBottom(rect1Up.bottom());
        rect.setRight(rect1UpPage.right());
        painter.fillRect(rect, mark);

        rect.setTop(rect2UpPage1.top());
        rect.setLeft(rect2UpPage2.right() + 0.5);
        rect.setBottom(rect2UpPage1.bottom());
        rect.setRight(rect2Up.right());
        painter.fillRect(rect, mark);
    }

    if (ui->leftMarginSpin->hasFocus())
    {
        QRectF rect;
        rect.setTop(rect1UpPage.top());
        rect.setLeft(rect1Up.left());
        rect.setBottom(rect1UpPage.bottom());
        rect.setRight(rect1UpPage.left());
        painter.fillRect(rect, mark);

        rect.setTop(rect2UpPage1.bottom());
        rect.setLeft(rect2UpPage1.left());
        rect.setBottom(rect2Up.bottom());
        rect.setRight(rect2UpPage2.right());
        painter.fillRect(rect, mark);
    }


    if (ui->rightMarginSpin->hasFocus())
    {
        QRectF rect;
        rect.setTop(rect1UpPage.top());
        rect.setLeft(rect1Up.left());
        rect.setBottom(rect1UpPage.bottom());
        rect.setRight(rect1UpPage.left());
        painter.fillRect(rect, mark);

        rect.setTop(rect2Up.top());
        rect.setLeft(rect2UpPage1.left());
        rect.setBottom(rect2UpPage1.top());
        rect.setRight(rect2UpPage2.right());
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


