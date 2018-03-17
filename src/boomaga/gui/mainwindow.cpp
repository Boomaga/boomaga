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
#include "kernel/job.h"
#include "kernel/printer.h"
#include "kernel/layout.h"
#include "kernel/inputfile.h"
#include "printersettings/printersettings.h"
#include "aboutdialog/aboutdialog.h"
#include "actions.h"
#include "icon.h"
#include "configdialog/configdialog.h"
#include "boomagatypes.h"
#include "export/exporttopdf.h"
#include "printdialog/printdialog.h"
#include "boomagatypes.h"

#include <math.h>
#include <QRadioButton>
#include <QMessageBox>
#include <QPushButton>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QTimer>
#include <QKeyEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QInputDialog>
#include <QDateTime>
#include <QMenu>


/************************************************

 ************************************************/
MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setContextMenuPolicy(Qt::NoContextMenu);

    setWindowIcon(Icon::icon(Icon::ApplicationIcon));
    setWindowTitle(tr("Boomaga"));

    setStyleSheet("QListView::item { padding: 2px;}");

    initStatusBar();
    initActions();

    Layout *layout = new LayoutNUp(1, 1);
    mAvailableLayouts << layout;
    ui->layout1UpBtn->setLayout(layout);

    layout = new LayoutNUp(2, 1);
    mAvailableLayouts << layout;
    ui->layout2UpBtn->setLayout(layout);

    layout = new LayoutNUp(2, 2, Qt::Horizontal);
    mAvailableLayouts << layout;
    ui->layout4UpHorizBtn->setLayout(layout);

    layout = new LayoutNUp(2, 2, Qt::Vertical);
    mAvailableLayouts << layout;
    ui->layout4UpVertBtn->setLayout(layout);

    layout = new LayoutNUp(4, 2, Qt::Horizontal);
    mAvailableLayouts << layout;
    ui->layout8UpHorizBtn->setLayout(layout);

    layout = new LayoutNUp(4, 2, Qt::Vertical);
    mAvailableLayouts << layout;
    ui->layout8UpVertBtn->setLayout(layout);

    layout = new LayoutBooklet();
    mAvailableLayouts << layout;
    ui->layoutBookletBtn->setLayout(layout);

    loadSettings();
    fillPrintersCombo();


    connect(ui->layout1UpBtn,     SIGNAL(clicked(bool)),
            this, SLOT(switchLayout()));

    connect(ui->layout2UpBtn,     SIGNAL(clicked(bool)),
            this, SLOT(switchLayout()));

    connect(ui->layout4UpHorizBtn, SIGNAL(clicked(bool)),
            this, SLOT(switchLayout()));

    connect(ui->layout4UpVertBtn, SIGNAL(clicked(bool)),
            this, SLOT(switchLayout()));

    connect(ui->layout8UpHorizBtn, SIGNAL(clicked(bool)),
            this, SLOT(switchLayout()));

    connect(ui->layout8UpVertBtn, SIGNAL(clicked(bool)),
            this, SLOT(switchLayout()));

    connect(ui->layoutBookletBtn, SIGNAL(clicked(bool)),
            this, SLOT(switchLayout()));

    connect(ui->doubleSidedCbx, SIGNAL(clicked(bool)),
            project, SLOT(setDoubleSided(bool)));

    connect(ui->jobsView, SIGNAL(pageSelected(int)),
            project, SLOT(setCurrentPage(int)));

    connect(ui->subBookletView, SIGNAL(pageSelected(int)),
            project, SLOT(setCurrentPage(int)));

    connect(ui->printersCombo, SIGNAL(activated(int)),
            this, SLOT(switchPrinterProfile()));

    connect(project, SIGNAL(changed()),
            this, SLOT(updateWidgets()));

    connect(project, SIGNAL(currentPageChanged(int)),
            this, SLOT(updateWidgets()));


    connect(project, SIGNAL(changed()),
            ui->preview, SLOT(refresh()));

    connect(project, SIGNAL(currentPageChanged(int)),
            ui->preview, SLOT(refresh()));

    connect(ui->printerConfigBtn, SIGNAL(clicked()),
            this, SLOT(showPrinterSettingsDialog()));

    connect(project, SIGNAL(progress(int,int)),
            this, SLOT(updateProgressBar(int, int)), Qt::QueuedConnection);

    connect(ui->preview, SIGNAL(contextMenuRequested(Sheet*,ProjectPage*)),
            this, SLOT(showPreviewContextMenu(Sheet*,ProjectPage*)));

    connect(ui->jobsView, SIGNAL(contextMenuRequested(Job)),
            this, SLOT(showJobViewContextMenu(Job)));

    connect(ui->menuFile, SIGNAL(aboutToShow()),
            this, SLOT(fillRecentFiles()));

    connect(ui->menuEditPage, SIGNAL(aboutToShow()),
            this, SLOT(showEditPageMainMenu()));

    connect(ui->menuEditJob, SIGNAL(aboutToShow()),
            this, SLOT(showEditJobMainMenu()));

    ui->preview->setFocusPolicy(Qt::StrongFocus);
    ui->preview->setFocus();

    ui->preview->setAcceptDrops(false);
    setAcceptDrops(true);
}


/************************************************

 ************************************************/
MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}


/************************************************
 *
 ************************************************/
void MainWindow::fillPrintersCombo()
{
    ui->printersCombo->setUpdatesEnabled(false);
    ui->printersCombo->clear();

    foreach(Printer *printer, Printer::availablePrinters())
    {
        ui->printersCombo->addPrinter(printer);
    }

    ui->printersCombo->setUpdatesEnabled(true);
    updateWidgets();
}


/************************************************

 ************************************************/
void MainWindow::closeEvent(QCloseEvent*)
{
    project->free();
}


/************************************************
 *
 * ***********************************************/
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    foreach(QUrl url, event->mimeData()->urls())
    {
        QString file = url.toLocalFile().toUpper();
        if (file.endsWith("PDF") || file.endsWith("BOO"))
        {
            event->acceptProposedAction();
            return;
        }
    }
}


/************************************************
 *
 * ***********************************************/
void MainWindow::dropEvent(QDropEvent *event)
{
    QStringList files;
    foreach(QUrl url, event->mimeData()->urls())
        files << url.toLocalFile();

    project->load(files);
}


/************************************************

 ************************************************/
void MainWindow::loadSettings()
{
    restoreGeometry(settings->value(Settings::MainWindow_Geometry).toByteArray());
    restoreState(settings->value(Settings::MainWindow_State).toByteArray());
    ui->optionsPanel->setGeometry(0, 0, settings->value(Settings::MainWindow_SplitterPos, 0).toInt(), 0);
    ui->pagesViewTab->setCurrentIndex(settings->value(Settings::MainWindow_PageListTab).toInt());

    QString s = settings->value(Settings::Printer).toString();
    Printer *currentPrinter = Printer::printerByName(s);
    if (!currentPrinter)
    {
        QList<Printer*> pl = Printer::availablePrinters();
        if (!pl.isEmpty())
            currentPrinter = pl.first();
    }

    if (!currentPrinter)
        currentPrinter = Printer::nullPrinter();

    project->setPrinterProfile(currentPrinter, currentPrinter->currentProfileIndex(), false);

    QString layoutId = settings->value(Settings::Layout).toString();

    foreach(Layout *layout, mAvailableLayouts)
    {
        if (layout->id() == layoutId)
            project->setLayout(layout);
    }

    if (!project->layout())
        project->setLayout(mAvailableLayouts.at(0));

    project->setDoubleSided(settings->value(Settings::DoubleSided).toBool());

    ui->jobsView->setIconSize(settings->value(Settings::MainWindow_PageListIconSize).toInt());
    ui->subBookletView->setIconSize(settings->value(Settings::MainWindow_PageListIconSize).toInt());
}


/************************************************

 ************************************************/
void MainWindow::saveSettings()
{
    settings->setValue(Settings::MainWindow_Geometry, saveGeometry());
    settings->setValue(Settings::MainWindow_State, saveState());
    settings->setValue(Settings::MainWindow_SplitterPos, ui->optionsPanel->width());
    settings->setValue(Settings::MainWindow_PageListTab, ui->pagesViewTab->currentIndex());

    Printer *printer = ui->printersCombo->currentPrinter();
    if (printer)
        settings->setValue(Settings::Printer, printer->name());

    settings->setValue(Settings::Layout, project->layout()->id());
    settings->setValue(Settings::DoubleSided, project->doubleSided());


    if (project->printer() != Printer::nullPrinter())
        project->printer()->saveSettings();

    settings->setValue(Settings::MainWindow_PageListIconSize, ui->jobsView->iconSize());
    settings->setValue(Settings::MainWindow_PageListIconSize, ui->subBookletView->iconSize());
    settings->sync();
}


/************************************************

 ************************************************/
void MainWindow::initActions()
{
    QAction *act;
    act = ui->actionPrint;
    act->setIcon(Icon::icon(Icon::Print));
    connect(act, SIGNAL(triggered()),
            this, SLOT(print()));

    act = ui->actionPrintAndClose;
    act->setIcon(Icon::icon(Icon::Print));
    connect(act, SIGNAL(triggered()),
            this, SLOT(printAndClose()));

    act = ui->actionPrintWithOptions;
    act->setIcon(Icon::icon(Icon::Print));
    connect(act, SIGNAL(triggered()),
            this, SLOT(printWithOptions()));

    act = ui->actionExit;
    connect(act, SIGNAL(triggered()),
            this, SLOT(close()));

    act = ui->actionPreviousSheet;
    act->setIcon(Icon::icon(Icon::Previous));
    connect(act, SIGNAL(triggered()),
            project, SLOT(prevSheet()));

    act = ui->actionNextSheet;
    act->setIcon(Icon::icon(Icon::Next));
    connect(act, SIGNAL(triggered()),
            project, SLOT(nextSheet()));

    act = ui->actionOpen;
    act->setIcon(Icon::icon(Icon::Open));
    connect(act, SIGNAL(triggered()),
            this, SLOT(load()));

    act = ui->actionSave;
    act->setIcon(Icon::icon(Icon::Save));
    connect(act, SIGNAL(triggered()),
            this, SLOT(save()));

    act = ui->actionSaveAs;
    act->setIcon(Icon::icon(Icon::SaveAs));
    connect(act, SIGNAL(triggered()),
            this, SLOT(saveAs()));

    act = ui->actionExport;
    act->setIcon(Icon::icon(Icon::SaveAs));
    connect(act, SIGNAL(triggered()),
            this, SLOT(exportAs()));

    act = ui->actionPreferences;
    act->setIcon(Icon::icon(Icon::Configure));
    connect(act, SIGNAL(triggered()),
            this, SLOT(showConfigDialog()));

    act = ui->actionAbout;
    connect(act, SIGNAL(triggered()),
            this, SLOT(showAboutDialog()));


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

    ui->printersCombo->setCurrentPrinterProfile(project->printer(), project->printer()->currentProfileIndex());


    ui->actionPrint->setEnabled(project->pageCount() > 0);
    ui->actionPrintAndClose->setEnabled(ui->actionPrint->isEnabled());

    ui->actionPreviousSheet->setEnabled(project->currentSheetNum() > 0);
    ui->actionNextSheet->setEnabled(project->currentSheetNum() < project->previewSheetCount() - 1);

    ui->actionSave->setEnabled(project->pageCount() > 0);
    ui->actionSaveAs->setEnabled(ui->actionSave->isEnabled());
    ui->actionExport->setEnabled(ui->actionSave->isEnabled());

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
        int sheetsCount = project->doubleSided() ?
                          ceil(project->sheetCount() / 2.0) :
                          project->sheetCount();


        QString pagesStr;
        if (project->currentPage())
        {
            pagesStr = tr("Page %1 of %2", "Status bar")
                    .arg(project->currentPageNum() + 1)
                    .arg(project->pageCount());
        }
        else
        {
            pagesStr = ((project->pageCount() > 1) ? tr("%1 pages", "Status bar") : tr("%1 page", "Status bar"))
                    .arg(project->pageCount());
        }

        QString sheetsStr = ((sheetsCount > 1) ? tr("%1 sheets", "Status bar") : tr("%1 sheet", "Status bar"))
                    .arg(sheetsCount);

        mStatusBarSheetsLabel.setText(pagesStr + " ( " + sheetsStr + " )");
        mStatusBarCurrentSheetLabel.setText(tr("Sheet %1 of %2", "Status bar")
                                .arg(project->currentSheetNum() + 1)
                                .arg(project->previewSheetCount()));

    }
    else
    {
        mStatusBarSheetsLabel.clear();
        mStatusBarCurrentSheetLabel.clear();
    }

    ui->printerConfigBtn->setEnabled(ui->printersCombo->currentPrinter() != 0);
}


/************************************************

 ************************************************/
void MainWindow::showPrinterSettingsDialog()
{
    Printer *printer = ui->printersCombo->currentPrinter();
    if (printer)
    {
        PrinterSettings *dialog = PrinterSettings::execute(printer);
        connect(dialog, SIGNAL(accepted()),
                this, SLOT(applyPrinterSettings()));
    }
}


/************************************************

 ************************************************/
void MainWindow::applyPrinterSettings()
{
    project->printer()->saveSettings();
    fillPrintersCombo();
    project->update();
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
void MainWindow::switchPrinterProfile()
{
    project->setPrinterProfile(ui->printersCombo->currentPrinter(),
                               ui->printersCombo->currentProfile());
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
bool MainWindow::print(uint count, bool collate)
{
    struct Keeper
    {
        QList<Sheet*> sheets_1;
        QList<Sheet*> sheets_2;

        ~Keeper()
        {
            qDeleteAll(sheets_1);
            qDeleteAll(sheets_2);
        }
    };

    Keeper keeper;
    if (!project->sheetCount())
        return false;

    saveAuto();

    bool res = true;
    bool showDialog = project->printer()->isShowProgressDialog();
    QMessageBox *infoDialog = 0;

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


         keeper.sheets_1 = project->selectSheets(pagesType_1, order_1);
         keeper.sheets_2 = project->selectSheets(pagesType_2, order_2);


         if (keeper.sheets_1.count())
         {
             if (order_1 == Project::BackOrder)
             {
                 while (keeper.sheets_1.count() < keeper.sheets_2.count())
                     keeper.sheets_1.insert(0, new Sheet(1, 0));
             }
             else
             {
                 while (keeper.sheets_1.count() < keeper.sheets_2.count())
                     keeper.sheets_1.append(new Sheet(1, 0));
             }


             if (showDialog && !keeper.sheets_2.count())
             {
                 infoDialog = showPrintDialog(tr("Print the all pages on %1.").arg(project->printer()->name()));
             }


             res = project->printer()->print(keeper.sheets_1, "", false, count, collate);
             if (!res)
             {
                 delete(infoDialog);
                 return false;
             }
         }


         // Show dialog ....................................
         if (keeper.sheets_1.count() && keeper.sheets_2.count())
         {
             QMessageBox dialog(this);
             dialog.setWindowTitle(this->windowTitle() + " ");
             dialog.setIconPixmap(QPixmap(":/images/print-48x48"));

             dialog.setText(tr("Print the odd pages on %1.<p>"
                               "When finished, turn the pages, insert them into the printer<br>"
                               "and click the Continue button.").arg(project->printer()->name()));

             dialog.addButton(QMessageBox::Abort);
             QPushButton *btn = dialog.addButton(QMessageBox::Ok);
             btn->setText(tr("Continue"));

             if (dialog.exec() != QMessageBox::Ok)
             {
                 delete(infoDialog);
                 return false;
             }
         }
         // ................................................


         if (keeper.sheets_2.count())
         {
             if (order_2 == Project::BackOrder)
             {
                 while (keeper.sheets_2.count() < keeper.sheets_1.count())
                     keeper.sheets_2.insert(0, new Sheet(1, 0));
             }
             else
             {
                 while (keeper.sheets_2.count() < keeper.sheets_1.count())
                     keeper.sheets_2.append(new Sheet(1, 0));
             }

             if (showDialog)
                infoDialog = showPrintDialog(tr("Print the even pages on %1.").arg(project->printer()->name()));

             res = project->printer()->print(keeper.sheets_2, "", false, count, collate);
             if (!res)
             {
                 delete(infoDialog);
                 return false;
             }
         }

    }
    else //if (split) no
    {
        if (showDialog)
            infoDialog = showPrintDialog(tr("Print the all pages on %1.").arg(project->printer()->name()));

        Project::PagesOrder order;
        if (project->printer()->reverseOrder())
            order = Project::BackOrder;
        else
            order = Project::ForwardOrder;

        keeper.sheets_1 = project->selectSheets(Project::AllPages, order);

        if (project->doubleSided() && keeper.sheets_1.count() % 2)
        {
            if (order == Project::BackOrder)
            {
                keeper.sheets_1.insert(0, new Sheet(1, 0));
            }
            else
            {
                keeper.sheets_1.append(new Sheet(1, 0));
            }
        }

        res = project->printer()->print(keeper.sheets_1, "", project->doubleSided(), count, collate);
        if (!res)
        {
            delete(infoDialog);
            return false;
        }
    }



    if (infoDialog)
        QTimer::singleShot(200, infoDialog, SLOT(deleteLater()));

    return true;
}


/************************************************

 ************************************************/
void MainWindow::printAndClose(uint count, bool collate)
{
    if (print(count, collate))
        QTimer::singleShot(200, this, SLOT(close()));
}


/************************************************

 ************************************************/
void MainWindow::printWithOptions()
{
    PrintDialog dialog;
    if (dialog.exec())
    {
        print(dialog.copies(), dialog.collate());
    }
}


/************************************************

 ************************************************/
void MainWindow::showAboutDialog()
{
    project->printer()->saveSettings();
    AboutDialog dialog(this);
    dialog.exec();

}


/************************************************

 ************************************************/
void MainWindow::updateProgressBar(int value, int all)
{
    if (all <1 && mProgressBar.isVisible())
    {
        mProgressBar.hide();
        return;
    }

    if (mProgressBar.maximum() != all)
        mProgressBar.setMaximum(all);

    mProgressBar.setValue(value);

    if (all > 0 && mProgressBar.isHidden())
        mProgressBar.show();
}


/************************************************
 *
 * ***********************************************/
void MainWindow::showPreviewContextMenu(Sheet*, ProjectPage *page)
{
    QMenu menu;
    fillPageEditMenu(page, &menu);
    menu.exec(QCursor::pos());
}


/************************************************

 ************************************************/
void MainWindow::showJobViewContextMenu(Job job)
{
    QMenu menu;
    fillJobEditMenu(job, &menu);
    menu.exec(QCursor::pos());
}


/************************************************
 *
 ************************************************/
void MainWindow::showEditPageMainMenu()
{
    ui->menuEditPage->clear();
    fillPageEditMenu(project->currentPage(), ui->menuEditPage);
}


/************************************************
 *
 ************************************************/
void MainWindow::showEditJobMainMenu()
{
    ProjectPage *page = project->currentPage();
    Job job;
    if (page)
    {
        int n = project->jobs()->indexOfProjectPage(page);
        if (n >= 0)
            job = project->jobs()->at(n);
    }

    ui->menuEditJob->clear();
    fillJobEditMenu(job, ui->menuEditJob);
}


/************************************************
 *
 ************************************************/
void MainWindow::fillPageEditMenu(ProjectPage *page, QMenu *menu)
{
    PageAction *act;

    // New subbooklet ................................
    act = new PageAction(tr("Start new booklet from this page"), page, menu);
    connect(act, SIGNAL(triggered()), this, SLOT(startBooklet()));
    act->setEnabled(page && page != project->page(0));
    act->setVisible(page && !page->isManualStartSubBooklet());
    menu->addAction(act);


    act = new PageAction(tr("Don't start new booklet from this page"), page, menu);
    connect(act, SIGNAL(triggered()), this, SLOT(dontStartBooklet()));
    act->setEnabled(page && page != project->page(0));
    act->setVisible(page && page->isManualStartSubBooklet());
    menu->addAction(act);
    // New subbooklet ................................


    menu->addSeparator();


    // Rotation ......................................
    act = new PageAction(tr("Rotate page to the left"), page, menu);
    connect(act, SIGNAL(triggered()), this, SLOT(rotatePageLeft()));
    act->setEnabled(page);
    menu->addAction(act);

    act = new PageAction(tr("Rotate page to the right"), page, menu);
    connect(act, SIGNAL(triggered()), this, SLOT(rotatePageRight()));
    act->setEnabled(page);
    menu->addAction(act);
    // Rotation ......................................


    menu->addSeparator();


    // Blank page ....................................
    act = new PageAction(tr("Insert blank page before this page"), page, menu);
    connect(act, SIGNAL(triggered()), this, SLOT(insertBlankPageBefore()));
    act->setEnabled(page);
    menu->addAction(act);

    act = new PageAction(tr("Insert blank page after this page"), page, menu);
    connect(act, SIGNAL(triggered()), this, SLOT(insertBlankPageAfter()));
    act->setEnabled(page);
    menu->addAction(act);
    // Blank page ....................................


    menu->addSeparator();


    // Delete page ...................................
    act = new PageAction(tr("Delete this page"), page, menu);
    connect(act, SIGNAL(triggered()), this, SLOT(deletePage()));
    act->setEnabled(page);
    menu->addAction(act);


    act = new PageAction(tr("Delete pages until the end of the job"), page, menu);
    connect(act, SIGNAL(triggered()), this, SLOT(deletePagesEnd()));
    act->setEnabled(page);
    menu->addAction(act);
    // Delete page ...................................

    // Undo delete ...................................
    QMenu *undelMenu = menu->addMenu(tr("Undo delete"));
    undelMenu->setEnabled(false);


    for(int j=0; j<project->jobs()->count(); ++j)
    {
        Job job = project->jobs()->at(j);

        for(int p=0; p<job.pageCount(); ++p)
        {
            ProjectPage *page = job.page(p);

            if (page->visible())
                continue;

            act = new PageAction(tr("%1 %2: Page %3",
                                    "'Undo deletion' menu item, (%1 - is a job number, %2 - is a job title, %3 - is a page number in this job")
                                 .arg(j+1).arg(job.title()).arg(p+1),
                                 page, undelMenu);

            connect(act, SIGNAL(triggered()), this, SLOT(undoDeletePage()));
            undelMenu->addAction(act);
        }

        undelMenu->setStyleSheet("* {menu-scrollable: 1 }");
        undelMenu->setEnabled(undelMenu->actions().count() > 0);
    }
    // Undo delete ...................................
}


/************************************************
 *
 * ***********************************************/
void MainWindow::fillJobEditMenu(const Job job, QMenu *menu)
{
    JobAction *act;

    act = new JobAction(tr("Rename job"), job, menu);
    connect(act, SIGNAL(triggered()), this, SLOT(renameJob()));
    act->setEnabled(job.pageCount());
    menu->addAction(act);


    act = new JobAction(Icon::icon(Icon::RotateLeft), tr("Rotate job to the left"), job, menu);
    connect(act, SIGNAL(triggered()), this, SLOT(rotateJobLeft()));
    act->setEnabled(job.pageCount());
    menu->addAction(act);


    act = new JobAction(Icon::icon(Icon::RotateRight), tr("Rotate job to the right"), job, menu);
    connect(act, SIGNAL(triggered()), this, SLOT(rotateJobRight()));
    act->setEnabled(job.pageCount());
    menu->addAction(act);


    menu->addSeparator();


    act = new JobAction(tr("Clone job..."), job, menu);
    connect(act, SIGNAL(triggered()), this, SLOT(cloneJob()));
    act->setEnabled(job.pageCount());
    menu->addAction(act);


    menu->addSeparator();



    // Undo delete ...................................
    QMenu *undelMenu = menu->addMenu(tr("Undo delete page"));
    undelMenu->setEnabled(false);

    for(int p=0; p<job.pageCount(); ++p)
    {
        ProjectPage *page = job.page(p);
        if (page->visible())
            continue;

        PageAction *act;
        act = new PageAction(tr("Page %1", "'Undo deletion' menu item").arg(p+1), page, undelMenu);
        connect(act, SIGNAL(triggered()), this, SLOT(undoDeletePage()));

        undelMenu->addAction(act);
    }
    undelMenu->setEnabled(undelMenu->isEnabled() || !undelMenu->isEmpty());
    // ...............................................


    menu->addSeparator();

    act = new JobAction(tr("Delete current job"), job, menu);
    connect(act, SIGNAL(triggered()), this, SLOT(deleteJob()));
    act->setEnabled(job.pageCount());
    menu->addAction(act);
}


/************************************************
 *
 * ***********************************************/
void MainWindow::deletePage()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;

    project->deletePage(act->page());
}


/************************************************

 ************************************************/
void MainWindow::undoDeletePage()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;

    project->undoDeletePage(act->page());
}


/************************************************

 ************************************************/
void MainWindow::deletePagesEnd()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;

    project->deletePagesEnd(act->page());
}


/************************************************
 *
 * ***********************************************/
void MainWindow::insertBlankPageBefore()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;

    project->insertBlankPageBefore(act->page());
}


/************************************************
 *
 * ***********************************************/
void MainWindow::insertBlankPageAfter()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;

    project->insertBlankPageAfter(act->page());
}


/************************************************

 ************************************************/
void MainWindow::renameJob()
{
    JobAction *act = qobject_cast<JobAction*>(sender());
    if (!act)
        return;

    bool ok;
    QString s = QInputDialog::getText(this, tr("Rename job"), tr("Job title:"),
                                      QLineEdit::Normal, act->job().title(false), &ok);

    if (ok)
    {
        act->job().setTitle(s);
        ui->jobsView->updateItems();
    }
}


/************************************************

 ************************************************/
void MainWindow::rotateJobLeft()
{
    JobAction *act = qobject_cast<JobAction*>(sender());
    if (!act)
        return;

    Job job = act->job();
    for (int i=0; i< job.pageCount(); ++i)
    {
        ProjectPage *page = job.page(i);
        page->setManualRotation(page->manualRotation() - Rotate90);
    }
    project->update();
}


/************************************************

 ************************************************/
void MainWindow::rotateJobRight()
{
    JobAction *act = qobject_cast<JobAction*>(sender());
    if (!act)
        return;

    Job job = act->job();
    for (int i=0; i< job.pageCount(); ++i)
    {
        ProjectPage *page = job.page(i);
        page->setManualRotation(page->manualRotation() + Rotate90);
    }
    project->update();
}


/************************************************
 *
 * ***********************************************/
void MainWindow::rotatePageLeft()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;

    act->page()->setManualRotation(act->page()->manualRotation() - Rotate90);
    project->update();
}


/************************************************
 *
 * ***********************************************/
void MainWindow::rotatePageRight()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;

    act->page()->setManualRotation(act->page()->manualRotation() + Rotate90);
    project->update();
}


/************************************************

 ************************************************/
void MainWindow::startBooklet()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;

    act->page()->setManualStartSubBooklet(true);
    project->update();

    int sheetNum = project->previewSheets().indexOfPage(act->page());
    if (sheetNum > -1)
        project->setCurrentSheet(sheetNum);
}


/************************************************

 ************************************************/
void MainWindow::dontStartBooklet()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;

    act->page()->setManualStartSubBooklet(false);
    project->update();

    int sheetNum = project->previewSheets().indexOfPage(act->page());
    if (sheetNum > -1)
        project->setCurrentSheet(sheetNum);
}


/************************************************

 ************************************************/
void MainWindow::showConfigDialog()
{
    ConfigDialog::createAndShow(this);
}


/************************************************

 ************************************************/
void MainWindow::cloneJob()
{
    JobAction *act = qobject_cast<JobAction*>(sender());
    if (!act)
        return;

    bool ok;
    int cnt = QInputDialog::getInt(this, tr("Clone job"), tr("Number of copies:"), 2, 2, 100, 1, &ok);
    if (ok)
    {
        for (int i=1; i<cnt; ++i)
        {
            Job job = act->job().clone();
            job.setTitle(QString("%1 [ %2 ]").arg(job.title()).arg(i+1));
            project->addJob(job);
        }
    }
}


/************************************************

 ************************************************/
void MainWindow::deleteJob()
{
    JobAction *act = qobject_cast<JobAction*>(sender());
    if (!act)
        return;


    int index = project->jobs()->indexOf(act->job());
    if (index > -1)
        project->removeJob(index);

    ui->jobsView->updateItems();
    ui->subBookletView->updateItems();
}


/************************************************

 ************************************************/
void MainWindow::save()
{
    saveAs(mSaveFile);
}


/************************************************

 ************************************************/
void MainWindow::saveAs(const QString &fileName)
{
    QString file = fileName;
    if (file.isEmpty())
    {
        file = QFileDialog::getSaveFileName(
                    this, this->windowTitle(),
                    settings->value(Settings::SaveDir).toString(),
                    tr("Boomaga files (*.boo);;All files (*.*)"));

        if (file.isEmpty())
            return;

        if (QFileInfo(file).suffix().toLower() != "boo")
            file += ".boo";
    }

    mSaveFile = file;
    settings->setValue(Settings::SaveDir, QFileInfo(file).path());
    addToRecentFiles(file);

    try
    {
        project->save(file);
        ui->statusbar->showMessage(tr("Project saved successfully."), 2000);
    }
    catch (QString &err)
    {
        project->error(err);
    }
}


/************************************************
 *
 * ***********************************************/
void MainWindow::exportAs()
{
    QString fileName = settings->value(Settings::ExportPDF_FileName).toString();

    ExportToPdf dialog;
    dialog.setOutFileName(fileName);
    dialog.setMetaInfo(project->metaData());

    dialog.setModal(true);
    if (!dialog.exec())
        return;

    fileName = dialog.outFileName();
    settings->setValue(Settings::ExportPDF_FileName, fileName);
    fileName = expandHomeDir(fileName);

    project->setMetadata(dialog.metaInfo());

    QList<Sheet*> sheets = project->selectSheets(Project::AllPages, Project::ForwardOrder);
    project->writeDocument(sheets, fileName);
}


/************************************************

 ************************************************/
void MainWindow::load()
{
    QString fileName = QFileDialog::getOpenFileName(
                this, this->windowTitle(),
                settings->value(Settings::SaveDir).toString(),
                tr("All supported files (*.pdf *.boo);;Boomaga files (*.boo);;PDF files (*.pdf);;All files (*.*)"));

    if (fileName.isEmpty())
        return;
    settings->setValue(Settings::SaveDir, QFileInfo(fileName).path());

    try
    {
        project->load(fileName);
    }
    catch (QString &err)
    {
        project->error(err);
    }
}

/************************************************
 *
 ************************************************/
void MainWindow::fillRecentFiles()
{
    ui->menuRecentFiles->clear();

    QStringList recentFiles = settings->value(Settings::RecentFiles).toStringList();
    foreach (QString file, recentFiles)
    {
        QAction *act = new QAction(ui->menuRecentFiles);
        act->setText(QFileInfo(file).fileName());
        act->setData(file);
        ui->menuRecentFiles->addAction(act);
        connect(act, SIGNAL(triggered()),
                this, SLOT(loadAuto()));
    }

    ui->menuRecentFiles->setEnabled(ui->menuRecentFiles->actions().count());
}


/************************************************

 ************************************************/
void MainWindow::addToRecentFiles(const QString &file)
{
    QStringList recentFiles = settings->value(Settings::RecentFiles).toStringList();
    recentFiles.removeAll(file);
    recentFiles << file;

    while (recentFiles.size() > 20)
        recentFiles.removeFirst();

    settings->setValue(Settings::RecentFiles, recentFiles);
}

/************************************************

 ************************************************/
void MainWindow::saveAuto()
{
    if (!settings->value(Settings::AutoSave).toBool())
            return;

    QString dir = settings->value(Settings::AutoSaveDir).toString();
    if (dir.startsWith("~"))
        dir.replace(0, 1, QDir::homePath());

    qDebug() << dir;

    if (!QDir().mkpath(dir))
    {
        qWarning() << QString("Can't create autosave directory '%1'").arg(dir);
        QMessageBox::warning(this, tr("Auto saving"), tr("I can't create directory \"%1\"")
                             .arg(dir));
        return;
    }

    QString file = QString("[%1]-%2.boo")
            .arg(QDateTime::currentDateTime().toString("yyyy.MM.dd-hh:mm:ss"))
            .arg(safeFileName(safeFileName(project->jobs()->first().title(true))));

    file = dir + "/" + file;
    addToRecentFiles(file);
    project->save(file);
}


/************************************************
 *
 ************************************************/
void MainWindow::loadAuto()
{
    QAction *act = qobject_cast<QAction*>(sender());
    if (!act)
        return;

    try
    {
        project->load(act->data().toString());
    }
    catch (QString &err)
    {
        project->error(err);
    }
}
