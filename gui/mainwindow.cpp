/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2013 Alexander Sokoloff
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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "kernel/psproject.h"
#include "settings.h"
#include "kernel/printer.h"
#include "printersettings/printersettings.h"
#include "psrender.h"

#include <QRadioButton>
#include <QMessageBox>
#include <QPushButton>
#include <QTemporaryFile>
#include <QDir>
#include <QProcess>
#include <QDebug>



/************************************************

 ************************************************/
QIcon findIcon(const QString theme1, const QString fallback)
{
    QIcon icon = QIcon::fromTheme(theme1);
    if (!icon.isNull())
        return icon;

    return QIcon(fallback);
}


/************************************************

 ************************************************/
MainWindow::MainWindow(PsProject *project, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mProject(project),
    mCurrentSheet(-1)
{
    mRender = new PsRender(mProject, this);
    ui->setupUi(this);
    ui->preview->setRender(mRender);


    setWindowIcon(findIcon("document-print", ":/images/print-48x48"));
    setWindowTitle(tr("Boomaga"));

    setStyleSheet("QListView::item { padding: 2px;}");


    ui->previewFrame->setBackgroundRole(QPalette::Dark);
    ui->previewFrame->setAutoFillBackground(true);

    ui->filesView->setProject(mProject);

    foreach(Printer *printer, availablePrinters())
    {
        ui->printersCbx->addPrinter(printer);
    }

    initStatusBar();
    initActions();

    ui->layout1UpBtn->setPsLayout(PsProject::Layout1Up);
    ui->layout2UpBtn->setPsLayout(PsProject::Layout2Up);
    ui->layout4UpBtn->setPsLayout(PsProject::Layout4Up);
    ui->layout8UpBtn->setPsLayout(PsProject::Layout8Up);
    ui->layoutBookletBtn->setPsLayout(PsProject::LayoutBooklet);


    loadSettings();
    switchPrinter();
    updateWidgets();
    updateStatusBar();

    connect(ui->layout1UpBtn,     SIGNAL(clicked(bool)), this, SLOT(switchLayout()));
    connect(ui->layout2UpBtn,     SIGNAL(clicked(bool)), this, SLOT(switchLayout()));
    connect(ui->layout4UpBtn,     SIGNAL(clicked(bool)), this, SLOT(switchLayout()));
    connect(ui->layout8UpBtn,     SIGNAL(clicked(bool)), this, SLOT(switchLayout()));
    connect(ui->layoutBookletBtn, SIGNAL(clicked(bool)), this, SLOT(switchLayout()));

    connect(ui->filesView, SIGNAL(fileSelected(PsFile*)), this, SLOT(switchToFile(PsFile*)));


    connect(ui->printersCbx, SIGNAL(activated(int)), this, SLOT(switchPrinter()));

    connect(mProject, SIGNAL(changed()), this, SLOT(updateWidgets()));
    connect(mProject, SIGNAL(changed()), this, SLOT(updateStatusBar()));
    connect(mProject, SIGNAL(changed()), this, SLOT(updateCurrentSheet()));

    connect(ui->printerConfigBtn, SIGNAL(clicked()), this, SLOT(showPrinterSettingsDialog()));


    connect(mProject, SIGNAL(fileAdded(const PsFile*)), mRender, SLOT(refresh()));
    connect(mProject, SIGNAL(fileRemoved()), mRender, SLOT(refresh()));
    connect(mProject, SIGNAL(fileRemoved()), this, SLOT(updateCurrentSheet()));

    connect(mRender, SIGNAL(finished()), this, SLOT(updateWidgets()));

    connect(ui->preview, SIGNAL(whellScrolled(int)), this, SLOT(psViewWhell(int)));
    mRender->refresh();


    setCurrentSheet(0);
}


/************************************************

 ************************************************/
MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}


/************************************************

 ************************************************/
QList<Printer *> MainWindow::availablePrinters()
{
    if (mAvailablePrinters.isEmpty())
    {
        QList<QPrinterInfo> printers = QPrinterInfo::availablePrinters();
        foreach (const QPrinterInfo &pi, printers)
        {
            Printer *printer = new Printer(pi);

            if (printer->deviceUri() != CUPS_BACKEND_URI)
                mAvailablePrinters << printer;
            else
                delete printer;
        }
    }

    return mAvailablePrinters;
}


/************************************************

 ************************************************/
void MainWindow::loadSettings()
{
    restoreGeometry(settings->mainWindowGeometry());
    restoreState(settings->mainWindowState());


   ui->printersCbx->setCurrentPrinter(settings->currentPrinter());
   if (ui->printersCbx->currentIndex() < 0)
       ui->printersCbx->setCurrentIndex(0);

}


/************************************************

 ************************************************/
void MainWindow::saveSettings()
{
    settings->setMainWindowGeometry(saveGeometry());
    settings->setMainWindowState(saveState());
    settings->sync();

    Printer *printer = ui->printersCbx->currentPrinter();
    settings->setCurrentPrinter(printer->printerName());
}


/************************************************

 ************************************************/
void MainWindow::initActions()
{
    QAction *act;
    act = ui->actionPrint;
    act->setIcon(findIcon("document-print", ":/images/print-48x48"));
    connect(act, SIGNAL(triggered()), this, SLOT(print()));

    act = ui->actionPrintAndClose;
    act->setIcon(findIcon("document-print", ":/images/print-48x48"));
    connect(act, SIGNAL(triggered()), this, SLOT(printAndClose()));

    act = ui->actionExit;
    connect(act, SIGNAL(triggered()), this, SLOT(close()));

    act = ui->actionPreviousSheet;
    act->setIcon(findIcon("go-previous-view", ":/images/previous"));
    connect(act, SIGNAL(triggered()), this, SLOT(showPrevSheet()));

    act = ui->actionNextSheet;
    act->setIcon(findIcon("go-next-view", ":/images/next"));
    connect(act, SIGNAL(triggered()), this, SLOT(showNextSheet()));
}


/************************************************

 ************************************************/
void MainWindow::initStatusBar()
{
    ui->statusbar->addPermanentWidget(&mStatusBarCurrentSheetLabel);

    ui->statusbar->addPermanentWidget(&mStatusBarSheetsLabel);
    mStatusBarSheetsLabel.setMinimumWidth(200);
    mStatusBarSheetsLabel.setAlignment(Qt::AlignRight);
}


/************************************************

 ************************************************/
void MainWindow::updateWidgets()
{
    foreach (LayoutRadioButton* btn, this->findChildren<LayoutRadioButton*>())
    {
        btn->setChecked(btn->psLayout() == mProject->layout());
    }

    ui->actionPrint->setEnabled(mProject->pageCount() > 0);
    ui->actionPrintAndClose->setEnabled(ui->actionPrint->isEnabled());

    ui->actionPreviousSheet->setEnabled(mCurrentSheet > 0);
    ui->actionNextSheet->setEnabled(mCurrentSheet < mProject->previewSheetCount() - 1);
}


/************************************************

 ************************************************/
void MainWindow::updateStatusBar()
{
    if (mProject->pageCount())
    {
        QString pagesTmpl = (mProject->pageCount() > 1) ? tr("%1 pages") : tr("%1 page");
        QString sheetsTmpl = (mProject->sheetCount() > 2) ? tr("%1 sheets") : tr("%1 sheet");
        mStatusBarSheetsLabel.setText(pagesTmpl.arg(mProject->pageCount()) +
                                   " ( " +
                                   sheetsTmpl.arg(mProject->sheetCount() / 2) +
                                   " )"
                                      );

        mStatusBarCurrentSheetLabel.setText(tr("Sheet %1 of %2")
                                .arg(mCurrentSheet + 1)
                                .arg(mRender->sheetCount()));
    }
    else
    {
        mStatusBarSheetsLabel.clear();
        mStatusBarCurrentSheetLabel.clear();
    }
}


/************************************************

 ************************************************/
void MainWindow::showPrinterSettingsDialog()
{
    PrinterSettings *dialog = PrinterSettings::create(ui->printersCbx->currentPrinter());
    connect(dialog, SIGNAL(accepted()), this, SLOT(applyPrinterSettings()));
    dialog->show();
}


/************************************************

 ************************************************/
void MainWindow::applyPrinterSettings()
{
    mProject->printer()->saveSettings();
    switchPrinter();
}


/************************************************

 ************************************************/
void MainWindow::showPrevSheet()
{
    setCurrentSheet(mCurrentSheet-1);
}


/************************************************

 ************************************************/
void MainWindow::showNextSheet()
{
    setCurrentSheet(mCurrentSheet+1);
}


/************************************************

 ************************************************/
void MainWindow::psViewWhell(int delta)
{
    setCurrentSheet(mCurrentSheet + (delta < 0 ? 1 : -1));
}


/************************************************

 ************************************************/
void MainWindow::updateCurrentSheet()
{
    setCurrentSheet(mCurrentSheet);
}


/************************************************

 ************************************************/
void MainWindow::setCurrentSheet(int value)
{
    if (mProject->sheetCount())
    {
        int n = qBound(0, value, mProject->previewSheetCount()-1);
        if (n != mCurrentSheet)
        {
            mCurrentSheet = n;
            ui->preview->setCurrentSheet(mCurrentSheet);
        }
    }
    else
    {
        mCurrentSheet = 0;
        ui->preview->setCurrentSheet(mCurrentSheet);
    }
    updateWidgets();
    updateStatusBar();
}


/************************************************

 ************************************************/
void MainWindow::switchLayout()
{
    LayoutRadioButton *btn = qobject_cast<LayoutRadioButton*>(sender());
    if (btn)
    {
        mProject->setLayout(btn->psLayout());
        mRender->refresh();
    }
}


/************************************************

 ************************************************/
void MainWindow::switchPrinter()
{
    mProject->setPrinter(ui->printersCbx->currentPrinter());
    mRender->refresh();
}


/************************************************

 ************************************************/
void MainWindow::switchToFile(PsFile *file)
{
    for (int i=0; i<mProject->previewSheetCount(); ++i)
    {
        const PsSheet *sheet = mProject->previewSheet(i);
        for (int j=0; j<sheet->count(); ++j)
        {
            const PsProjectPage *page = sheet->page(j);
            if (page &&
                    page->file() == file &&
                    page->pageNum() == 0)
            {
                setCurrentSheet(i);
                return;
            }
        }
    }
}



/************************************************

 ************************************************/
QTemporaryFile *MainWindow::getTmpFile()
{
    QTemporaryFile *file = new QTemporaryFile(QDir::tempPath() + "/boomaga_XXXXXX.ps");
    if (!file->open()) //QFile::WriteOnly))
    {
        qWarning() << "Can't open temporary file:" << file->errorString();
        delete file;
        return 0;
    }

    //file->setAutoRemove(false);
    return file;
}



/************************************************

 ************************************************/
void MainWindow::print()
{
    if (mProject->printer()->duplex())
    {
        QTemporaryFile *file = getTmpFile();
        if (!file)
            return;

        QTextStream stream(file);
        mProject->writeDocument(PsProject::AllPages, &stream);
        file->close();

        mProject->printer()->print(file->fileName(), "", 1);
        delete file;
    }
    else
    {
        // Print odd pages ................................
        {
            QTemporaryFile *file = getTmpFile();
            if (!file)
                return;

            QTextStream stream(file);
            mProject->writeDocument(PsProject::OddPages, &stream);
            file->close();

            mProject->printer()->print(file->fileName(), "", 1);
            delete file;
        }

        // Show dialog ....................................
        QMessageBox dialog(this);
        dialog.setWindowTitle(this->windowTitle() + " ");
        dialog.setIconPixmap(QPixmap(":/images/print-48x48"));

        dialog.setText(tr("I print the odd pages.<p>"
                "When finished, turn the pages, insert them into the printer<br>"
                "and click the Continue button."));

        dialog.addButton(QMessageBox::Abort);

        QPushButton *btn = dialog.addButton(QMessageBox::Ok);
        btn->setText(tr("Continue"));

        if (dialog.exec() != QMessageBox::Ok)
            return;
        // ................................................

        // Print even pages ...............................
        {
            QTemporaryFile *file = getTmpFile();
            if (!file)
                return;

            QTextStream stream(file);
            mProject->writeDocument(PsProject::EvenPages, &stream);
            file->close();

            mProject->printer()->print(file->fileName(), "", 1);
            delete file;
        }
    }
}


/************************************************

 ************************************************/
void MainWindow::printAndClose()
{
    print();
    close();
}





