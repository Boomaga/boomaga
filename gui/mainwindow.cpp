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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "kernel/project.h"
#include "settings.h"
#include "kernel/printer.h"
#include "kernel/layout.h"
#include "kernel/inputfile.h"
#include "printersettings/printersettings.h"
#include "aboutdialog/aboutdialog.h"

#include <QRadioButton>
#include <QMessageBox>
#include <QPushButton>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QTimer>
#include <QKeyEvent>

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
MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    delete ui->menuPreferences;

    setWindowIcon(findIcon("document-print", ":/images/print-48x48"));
    setWindowTitle(tr("Boomaga"));

    setStyleSheet("QListView::item { padding: 2px;}");

    foreach(Printer *printer, availablePrinters())
    {
        ui->printersCbx->addPrinter(printer);
    }
    initStatusBar();
    initActions();

    Layout *layout = new LayoutNUp(1, 1);
    mAvailableLayouts << layout;
    ui->layout1UpBtn->setLayout(layout);

    layout = new LayoutNUp(2, 1);
    mAvailableLayouts << layout;
    ui->layout2UpBtn->setLayout(layout);

    layout = new LayoutNUp(2, 2);
    mAvailableLayouts << layout;
    ui->layout4UpBtn->setLayout(layout);

    layout = new LayoutNUp(4, 2);
    mAvailableLayouts << layout;
    ui->layout8UpBtn->setLayout(layout);

    layout = new LayoutBooklet();
    mAvailableLayouts << layout;
    ui->layoutBookletBtn->setLayout(layout);


    loadSettings();
    switchPrinter();
    updateWidgets();
    updateWidgets();

    connect(ui->layout1UpBtn,     SIGNAL(clicked(bool)),
            this, SLOT(switchLayout()));

    connect(ui->layout2UpBtn,     SIGNAL(clicked(bool)),
            this, SLOT(switchLayout()));

    connect(ui->layout4UpBtn,     SIGNAL(clicked(bool)),
            this, SLOT(switchLayout()));

    connect(ui->layout8UpBtn,     SIGNAL(clicked(bool)),
            this, SLOT(switchLayout()));

    connect(ui->layoutBookletBtn, SIGNAL(clicked(bool)),
            this, SLOT(switchLayout()));

    connect(ui->doubleSidedCbx, SIGNAL(clicked(bool)),
            project, SLOT(setDoubleSided(bool)));

    connect(ui->filesView, SIGNAL(fileSelected(InputFile*)),
            this, SLOT(switchToFile(InputFile*)));

    connect(ui->printersCbx, SIGNAL(activated(int)),
            this, SLOT(switchPrinter()));

    connect(project, SIGNAL(changed()),
            this, SLOT(updateWidgets()));

    connect(ui->printerConfigBtn, SIGNAL(clicked()),
            this, SLOT(showPrinterSettingsDialog()));

    connect(project, SIGNAL(progress(int,int)),
            this, SLOT(updateProgressBar(int, int)));

    connect(ui->preview, SIGNAL(changed(int)),
            this, SLOT(updateWidgets()));

    connect(ui->preview, SIGNAL(changed(int)),
            ui->filesView, SLOT(setSheetNum(int)));

    ui->preview->setFocusPolicy(Qt::StrongFocus);
    ui->preview->setFocus();

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
    restoreGeometry(settings->value(Settings::MainWindow_Geometry).toByteArray());
    restoreState(settings->value(Settings::MainWindow_State).toByteArray());


    ui->printersCbx->setCurrentPrinter(settings->value(Settings::Printer).toString());
    if (ui->printersCbx->currentIndex() < 0)
        ui->printersCbx->setCurrentIndex(0);

    QString layoutId = settings->value(Settings::Layout).toString();

    foreach(Layout *layout, mAvailableLayouts)
    {
        if (layout->id() == layoutId)
            project->setLayout(layout);
    }

    if (!project->layout())
        project->setLayout(mAvailableLayouts.at(0));

    project->setDoubleSided(settings->value(Settings::DoubleSided).toBool());
}


/************************************************

 ************************************************/
void MainWindow::saveSettings()
{
    settings->setValue(Settings::MainWindow_Geometry, saveGeometry());
    settings->setValue(Settings::MainWindow_State, saveState());

    Printer *printer = ui->printersCbx->currentPrinter();
    settings->setValue(Settings::Printer, printer->printerName());

    settings->setValue(Settings::Layout, project->layout()->id());
    settings->setValue(Settings::DoubleSided, project->doubleSided());

    settings->sync();
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
    connect(act, SIGNAL(triggered()), ui->preview, SLOT(prevSheet()));

    act = ui->actionNextSheet;
    act->setIcon(findIcon("go-next-view", ":/images/next"));
    connect(act, SIGNAL(triggered()), ui->preview, SLOT(nextSheet()));

    act = ui->actionAbout;
    connect(act, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
}


/************************************************

 ************************************************/
void MainWindow::initStatusBar()
{
    ui->statusbar->addPermanentWidget(&mStatusBarCurrentSheetLabel);

    ui->statusbar->addPermanentWidget(&mStatusBarSheetsLabel);
    mStatusBarSheetsLabel.setMinimumWidth(200);
    mStatusBarSheetsLabel.setAlignment(Qt::AlignRight);

    ui->statusbar->addWidget(&mProgressBar);
    mProgressBar.setFixedWidth(200);
    mProgressBar.setSizePolicy(mProgressBar.sizePolicy().horizontalPolicy(),
                               QSizePolicy::Maximum);

    mProgressBar.setFormat(tr("%v of %m", "Format for QProgressBar (http://qt-project.org/doc/qt-4.8/qprogressbar.html#format-prop)"));
    mProgressBar.hide();
}


/************************************************

 ************************************************/
void MainWindow::updateWidgets()
{
    foreach (LayoutRadioButton* btn, this->findChildren<LayoutRadioButton*>())
    {
        btn->setChecked(btn->layout() == project->layout());
    }

    ui->actionPrint->setEnabled(project->pageCount() > 0);
    ui->actionPrintAndClose->setEnabled(ui->actionPrint->isEnabled());

    ui->actionPreviousSheet->setEnabled(ui->preview->currentSheet() > 0);
    ui->actionNextSheet->setEnabled(ui->preview->currentSheet() < project->previewSheetCount() - 1);


    if (project->layout()->id() == "Booklet")
    {
        ui->doubleSidedCbx->setChecked(true);
        ui->doubleSidedCbx->setEnabled(false);
    }
    else
    {
        ui->doubleSidedCbx->setChecked(project->doubleSided());
        ui->doubleSidedCbx->setEnabled(true);
    }

    // Update status bar ..........................
    if (project->pageCount())
    {
        QString pagesTmpl = (project->pageCount() > 1) ? tr("%1 pages", "Status bar") : tr("%1 page", "Status bar");
        QString sheetsTmpl = (project->sheetCount() > 2) ? tr("%1 sheets", "Status bar") : tr("%1 sheet", "Status bar");
        mStatusBarSheetsLabel.setText(pagesTmpl.arg(project->pageCount()) +
                                   " ( " +
                                   sheetsTmpl.arg(project->sheetCount() / 2) +
                                   " )"
                                      );

        mStatusBarCurrentSheetLabel.setText(tr("Sheet %1 of %2", "Status bar")
                                .arg(ui->preview->currentSheet() + 1)
                                .arg(project->previewSheetCount()));
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
    project->printer()->saveSettings();
    switchPrinter();
}


/************************************************

 ************************************************/
void MainWindow::switchLayout()
{
    LayoutRadioButton *btn = qobject_cast<LayoutRadioButton*>(sender());
    if (btn)
    {
        project->setLayout(btn->layout());
    }
}


/************************************************

 ************************************************/
void MainWindow::switchPrinter()
{
    project->setPrinter(ui->printersCbx->currentPrinter());
}


/************************************************

 ************************************************/
void MainWindow::switchToFile(InputFile *file)
{
    ProjectPage *page = file->pages().first();
    for (int i=0; i<project->previewSheetCount(); ++i)
    {
        const Sheet *sheet = project->previewSheet(i);
        for (int j=0; j<sheet->count(); ++j)
        {
            if (sheet->page(j) == page)
            {
                ui->preview->setCurrentSheet(i);
                return;
            }
        }
    }
}



/************************************************
 *
 ************************************************/
QMessageBox *MainWindow::showPrintDialog(const QString &text)
{
    QMessageBox *infoDialog = new QMessageBox(this);
    infoDialog->setWindowTitle(this->windowTitle() + " ");
    infoDialog->setIconPixmap(QPixmap(":/images/print-48x48"));
    infoDialog->setStandardButtons(QMessageBox::NoButton);

    infoDialog->setText(text);
    infoDialog->show();
    qApp->processEvents();

    return infoDialog;
}


/************************************************

 ************************************************/
void MainWindow::print()
{
    if (!project->sheetCount())
        return;

    QMessageBox *infoDialog = 0;
    Sheet emptySheet(1, 0);

    bool split = project->doubleSided() &&
                 project->printer()->duplexType() != DuplexAuto;

    if (split)
    {
        Project::PagesOrder order_1;
        Project::PagesOrder order_2;
        Project::PagesType  pagesType_1;
        Project::PagesType  pagesType_2;

        if (project->printer()->duplexType() == DuplexManual)
        {
            if (!project->printer()->reverseOrder())
            {
                order_1 = Project::ForwardOrder;
                order_2 = Project::ForwardOrder;
                pagesType_1 = Project::OddPages;
                pagesType_2 = Project::EvenPages;
            }
            else
            {
                order_1 = Project::BackOrder;
                order_2 = Project::BackOrder;
                pagesType_1 = Project::EvenPages;
                pagesType_2 = Project::OddPages;
            }
        }
        else
        {
            if (!project->printer()->reverseOrder())
            {
                order_1 = Project::ForwardOrder;
                order_2 = Project::BackOrder;
                pagesType_1 = Project::OddPages;
                pagesType_2 = Project::EvenPages;
            }
            else
            {
                order_1 = Project::BackOrder;
                order_2 = Project::ForwardOrder;
                pagesType_1 = Project::EvenPages;
                pagesType_2 = Project::OddPages;
            }
        }


         QList<Sheet*> sheets_1 = project->selectSheets(pagesType_1, order_1);
         QList<Sheet*> sheets_2 = project->selectSheets(pagesType_2, order_2);


         if (sheets_1.count())
         {
             if (order_1 == Project::BackOrder)
             {
                 while (sheets_1.count() < sheets_2.count())
                     sheets_1.insert(0, &emptySheet);
             }
             else
             {
                 while (sheets_1.count() < sheets_2.count())
                     sheets_1.append(&emptySheet);
             }


             if (! sheets_2.count())
             {
                 infoDialog = showPrintDialog(tr("Print the all pages on %1.").arg(project->printer()->printerName()));
             }


             project->printer()->print(sheets_1, "", false, 1);
         }


         // Show dialog ....................................
         if (sheets_1.count() && sheets_2.count())
         {
             QMessageBox dialog(this);
             dialog.setWindowTitle(this->windowTitle() + " ");
             dialog.setIconPixmap(QPixmap(":/images/print-48x48"));

             dialog.setText(tr("Print the odd pages on %1.<p>"
                               "When finished, turn the pages, insert them into the printer<br>"
                               "and click the Continue button.").arg(project->printer()->printerName()));

             dialog.addButton(QMessageBox::Abort);
             QPushButton *btn = dialog.addButton(QMessageBox::Ok);
             btn->setText(tr("Continue"));

             if (dialog.exec() != QMessageBox::Ok)
                 return;
         }
         // ................................................


         if (sheets_2.count())
         {
             if (order_2 == Project::BackOrder)
             {
                 while (sheets_2.count() < sheets_1.count())
                     sheets_2.insert(0, &emptySheet);
             }
             else
             {
                 while (sheets_2.count() < sheets_1.count())
                     sheets_2.append(&emptySheet);
             }

             infoDialog = showPrintDialog(tr("Print the even pages on %1.").arg(project->printer()->printerName()));

             project->printer()->print(sheets_2, "", false, 1);
         }

    }
    else
    {
        infoDialog = showPrintDialog(tr("Print the all pages on %1.").arg(project->printer()->printerName()));

        Project::PagesOrder order;
        if (project->printer()->reverseOrder())
            order = Project::BackOrder;
        else
            order = Project::ForwardOrder;

        QList<Sheet*> sheets = project->selectSheets(Project::AllPages, order);

        if (project->doubleSided() && sheets.count() % 2)
        {
            if (order == Project::BackOrder)
            {
                sheets.insert(0, &emptySheet);
                qDebug() << Q_FUNC_INFO << "insert";
            }
            else
            {
                sheets.append(&emptySheet);
                qDebug() << Q_FUNC_INFO << "append";
            }
        }

        project->printer()->print(sheets, "", project->doubleSided(), 1);
    }



    if (infoDialog)
        QTimer::singleShot(200, infoDialog, SLOT(deleteLater()));
}


/************************************************

 ************************************************/
void MainWindow::printAndClose()
{
    print();
    QTimer::singleShot(200, this, SLOT(close()));
}


/************************************************

 ************************************************/
void MainWindow::showAboutDialog()
{
    AboutDialog dialog(this);
    dialog.exec();

}


/************************************************

 ************************************************/
void MainWindow::updateProgressBar(int value, int all)
{
    if (all <1)
    {
        mProgressBar.hide();
        return;
    }

    if (mProgressBar.maximum() != all)
        mProgressBar.setMaximum(all);

    mProgressBar.setValue(value);

    if (all > 0)
        mProgressBar.show();
}


/************************************************

 ************************************************/
void MainWindow::closeEvent(QCloseEvent *event)
{
    project->free();
}



