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


#include "exporttopdf.h"
#include "ui_exporttopdf.h"

#include <QFileDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDebug>

/************************************************

 ************************************************/
ExportToPdf::ExportToPdf(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportToPdf)
{
    ui->setupUi(this);
    connect(ui->outFileSelectBtn, SIGNAL(clicked()),
            this, SLOT(selectFile()));

    connect(ui->outFileEdit, SIGNAL(textChanged(QString)),
            this, SLOT(outFileNameChanged()));

    outFileNameChanged();
}


/************************************************

 ************************************************/
ExportToPdf::~ExportToPdf()
{
    delete ui;
}


/************************************************

 ************************************************/
void ExportToPdf::selectFile()
{
    QString file = QFileDialog::getSaveFileName(
                this, this->windowTitle(),
                (this->outFileName().isEmpty() ? QDir::homePath() : this->outFileName()),
                tr("PDF files (*.pdf);;All files (*.*)"),
                0,
                QFileDialog::DontConfirmOverwrite);

    if (!file.isEmpty())
        setOutFileName(file);

    outFileNameChanged();
}


/************************************************

 ************************************************/
void ExportToPdf::outFileNameChanged()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!ui->outFileEdit->text().isEmpty());
}


/************************************************

 ************************************************/
QString ExportToPdf::outFileName() const
{
    return ui->outFileEdit->text();
}


/************************************************

 ************************************************/
void ExportToPdf::setOutFileName(const QString &value)
{
    ui->outFileEdit->setText(value);
    outFileNameChanged();
}


/************************************************

 ************************************************/
MetaData ExportToPdf::metaInfo() const
{
    MetaData res;
    res.setAuthor(ui->authorEdit->text());
    res.setTitle(ui->titleEdit->text());
    res.setSubject(ui->subjectEdit->text());
    res.setKeywords(ui->keywordsEdit->text());

    return res;
}


/************************************************

 ************************************************/
void ExportToPdf::setMetaInfo(const MetaData &value)
{
    ui->authorEdit->setText(value.author());
    ui->titleEdit->setText(value.title());
    ui->subjectEdit->setText(value.subject());
    ui->keywordsEdit->setText(value.keywords());
}


/************************************************
 *
 * ***********************************************/
void ExportToPdf::accept()
{
    QString fileName = outFileName();

    if (fileName.startsWith("~"))
        fileName.replace("~", QDir::homePath());

    if(QFileInfo(fileName).exists())
    {
        QMessageBox::StandardButton res = QMessageBox::warning(
                    this,
                    tr("Overwrite file?"),
                    tr("A file named \"%1\" already exists.\nAre you sure you want to overwrite it?").arg(outFileName()),
                    QMessageBox::Ok | QMessageBox::Cancel);

        if (res != QMessageBox::Ok)
        {
            return;
        }
    }

    QDialog::accept();
}
