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
#include "kernel/printer.h"
#include "kernel/psconstants.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QApplication>
#include <QAbstractButton>
#include <QPainter>
#include <QRect>
#include <QDebug>

PrinterSettings *PrinterSettings::mInstance = 0;

/************************************************

 ************************************************/
PrinterSettings *PrinterSettings::create(Printer *printer)
{
    if (!mInstance)
    {
        mInstance = new PrinterSettings(qApp->activeWindow());
    }

    mInstance->setCurrentPrinter(printer);
    return mInstance;
}



/************************************************

 ************************************************/
PrinterSettings::PrinterSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrinterSettings),
    mUnit(Printer::Millimeter)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);

    ui->leftMarginSpin->installEventFilter(this);
    ui->rightMarginSpin->installEventFilter(this);
    ui->topMarginSpin->installEventFilter(this);
    ui->bottomMarginSpin->installEventFilter(this);
    ui->internalMarginSpin->installEventFilter(this);

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(btnClicked(QAbstractButton*)));
    connect(ui->borderCbx, SIGNAL(toggled(bool)), this, SLOT(updatePreview()));
}


/************************************************

 ************************************************/
PrinterSettings::~PrinterSettings()
{
    mInstance = 0;
    delete ui;
}



/************************************************

 ************************************************/
void PrinterSettings::setCurrentPrinter(Printer *printer)
{
    mPrinter = printer;
    setWindowTitle(tr("Prefferences of \"%1\"").arg(mPrinter->printerName()));
    updateWidgets();
}


/************************************************

 ************************************************/
void PrinterSettings::showEvent(QShowEvent *)
{
    mInstance->raise();
    mInstance->activateWindow();
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

 ************************************************/
void PrinterSettings::updateWidgets()
{
    ui->duplexCbx->setChecked(mPrinter->duplex());
    ui->borderCbx->setChecked(mPrinter->drawBorder());

    ui->leftMarginSpin->setValue(mPrinter->leftMargin(mUnit));
    ui->rightMarginSpin->setValue(mPrinter->rightMargin(mUnit));
    ui->topMarginSpin->setValue(mPrinter->topMargin(mUnit));
    ui->bottomMarginSpin->setValue(mPrinter->bottomMargin(mUnit));
    ui->internalMarginSpin->setValue(mPrinter->internalMarhin(mUnit));
}


/************************************************

 ************************************************/
void PrinterSettings::btnClicked(QAbstractButton *button)
{
    if (ui->buttonBox->standardButton(button) == QDialogButtonBox::Ok ||
        ui->buttonBox->standardButton(button) == QDialogButtonBox::Apply)
    {
        mPrinter->setLeftMargin(ui->leftMarginSpin->value(), mUnit);
        mPrinter->setRightMargin(ui->rightMarginSpin->value(), mUnit);
        mPrinter->setTopMargin(ui->topMarginSpin->value(), mUnit);
        mPrinter->setBottomMargin(ui->bottomMarginSpin->value(), mUnit);
        mPrinter->setInternalMargin(ui->internalMarginSpin->value(), mUnit);

        mPrinter->setDuplex(ui->duplexCbx->isChecked());
        mPrinter->setDrawBorder(ui->borderCbx->isChecked());

        if (ui->buttonBox->standardButton(button) == QDialogButtonBox::Ok)
            close();

        emit accepted();
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



