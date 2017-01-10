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


#include "printdialog.h"
#include "ui_printdialog.h"
#include "../settings.h"

static int mCopiesSave = 1;

/************************************************

 ************************************************/
PrintDialog::PrintDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrintDialog)
{
    ui->setupUi(this);
    ui->copiesEdit->setValue(mCopiesSave);

    connect(ui->copiesEdit, SIGNAL(valueChanged(int)),
            this, SLOT(CopiesEditValueChanged(int)));

    restoreGeometry(settings->value(Settings::PrinterDialog_Geometry).toByteArray());
    loadSettings();
}


/************************************************

 ************************************************/
PrintDialog::~PrintDialog()
{
    settings->setValue(Settings::PrinterDialog_Geometry, saveGeometry());
    delete ui;
}

/************************************************

 ************************************************/
void PrintDialog::loadSettings()
{
    ui->copiesEdit->setValue(settings->value(Settings::Printer_CopiesNum, 1).toInt());
    ui->collateCheckBox->setChecked(settings->value(Settings::Printer_CollateCopies, true).toBool());
}

/************************************************

 ************************************************/
void PrintDialog::saveSettings()
{
    settings->setValue(Settings::Printer_CopiesNum, ui->copiesEdit->value());
    settings->setValue(Settings::Printer_CollateCopies, ui->collateCheckBox->isChecked());
}


/************************************************

 ************************************************/
int PrintDialog::copies() const
{
    return ui->copiesEdit->value();
}


/************************************************

 ************************************************/
void PrintDialog::setCopies(int value)
{
    ui->copiesEdit->setValue(value);
    CopiesEditValueChanged(value);
}


/************************************************

 ************************************************/
bool PrintDialog::collate() const
{
    return ui->collateCheckBox->isChecked();
}


/************************************************

 ************************************************/
void PrintDialog::setCollate(bool value)
{
    ui->collateCheckBox->setChecked(value);
}


/************************************************

 ************************************************/
void PrintDialog::done(int res)
{
    if (res)
        saveSettings();

    QDialog::done(res);
}


/************************************************

 ************************************************/
void PrintDialog::CopiesEditValueChanged(int value)
{
    mCopiesSave = value;
}

