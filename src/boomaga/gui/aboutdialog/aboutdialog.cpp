/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
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

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
//#include "aboutdialog_p.h"
#include "translations/translatorsinfo/translatorsinfo.h"
#include <QDebug>
#include <QtCore/QDate>

//AboutDialogPrivate::AboutDialogPrivate()
AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
//    this->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);

    QString css="<style TYPE='text/css'> "
                    "body { font-family: sans-serif;} "
                    ".name { font-size: 16pt; } "
                    "a { white-space: nowrap ;} "
                    "h2 { font-size: 10pt;} "
                    "li { line-height: 120%;} "
                    ".techInfoKey { white-space: nowrap ; margin: 0 20px 0 16px; } "
                "</style>"
            ;

    ui->iconLabel->setFixedSize(48, 48);
    ui->iconLabel->setScaledContents(true);
    //iconLabel->setPixmap(QPixmap(QString(SHARE_DIR) + "/graphics/razor_logo.png"));

    ui->nameLabel->setText(css + titleText());

    ui->aboutBrowser->setHtml(css + aboutText());
    ui->aboutBrowser->viewport()->setAutoFillBackground(false);

    ui->autorsBrowser->setHtml(css + authorsText());
    ui->autorsBrowser->viewport()->setAutoFillBackground(false);

    ui->thanksBrowser->setHtml(css + thanksText());
    ui->thanksBrowser->viewport()->setAutoFillBackground(false);
    ui->thanksTab->setVisible(false);

    ui->translationsBrowser->setHtml(css + translationsText());
    ui->translationsBrowser->viewport()->setAutoFillBackground(false);


    //    show();
}


/************************************************

 ************************************************/
AboutDialog::~AboutDialog()
{
    delete ui;
}


/************************************************

 ************************************************/
QString AboutDialog::titleText() const
{
    return QString("<div class=name>%1</div><div class=ver>%2</div>").arg(
                "Boomaga",
                tr("Version: %1").arg(FULL_VERSION));

}


/************************************************

 ************************************************/
QString AboutDialog::aboutText() const
{
    return  QString("<br>%1<br><br><br>%2<hr>%3<p>%4").arg(
                tr("Boomaga provides a virtual printer for CUPS. This can be used for print preview or for print booklets."),
                tr("Copyright: %1-%2 %3").arg("2012", QDate::currentDate().toString("yyyy"), "Boomaga team"),

                tr("Homepage: %1").arg("<a href='http://boomaga.github.io'>http://boomaga.github.io</a>"),
                tr("License: %1").arg("<a href='http://www.gnu.org/licenses/gpl-2.0.html'>GNU General Public License version 2</a>"
                                      "and partly under the "
                                      "<a href='http://www.gnu.org/licenses/lgpl-2.1.html'>GNU Lesser General Public License version 2.1 or later</a> "
                                      )
                );
}


/************************************************

 ************************************************/
QString AboutDialog::authorsText() const
{
    return QString("%1<p>%2").arg(
                tr("Boomaga is developed by the <a %1>Boomaga Team and contributors</a> on GitHub.")
                    .arg(" href='https://github.com/Boomaga?tab=members'"),
                tr("If you are interested in working with our development team, <a %1>join us</a>.")
                    .arg(" href='https://github.com/Boomaga/boomaga'")
                );
}


/************************************************

 ************************************************/
QString AboutDialog::thanksText() const
{
    return QString(
                "%1"
                "<ul>"
                "<li>CUPS project (http://www.cups.org)</li>"
                "<li>FlatIcon (https://www.flaticon.com) - main icon for application</li>"
                "<li>Icons8 (https://icons8.com/) - icons for application</li>"
                "</ul>"
                ).arg(tr("Special thanks to:"));
}


/************************************************

 ************************************************/
QString AboutDialog::translationsText() const
{
    TranslatorsInfo translatorsInfo;
    return QString("%1<p><ul>%2</ul>").arg(
                tr("If you want to help translate, we will be glad to see you in our translation team on <a %1>Transifex server</a>.")
                    .arg(" href='https://www.transifex.com/projects/p/boomaga/'"),
                translatorsInfo.asHtml()
                );
}



