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
#include "export/exporttopdfprinter.h"
#include "icon.h"
#include "configdialog/configdialog.h"
#include "boomagatypes.h"
#include "export/exporttopdf.h"

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


/************************************************

 ************************************************/
MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowIcon(Icon::icon(Icon::ApplicationIcon));
    setWindowTitle(tr("Boomaga"));

    setStyleSheet("QListView::item { padding: 2px;}");

    foreach(Printer *printer, Printer::availablePrinters())
    {
        ui->printersCbx->addPrinter(printer);
    }
    ui->printersCbx->insertSeparator(99999);
    mExportPrinter = new ExportToPDFPrinter();
    ui->printersCbx->addPrinter(mExportPrinter);

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
    switchPrinter();
    updateWidgets();
    updateWidgets();

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

    connect(ui->jobsView, SIGNAL(jobSelected(Job)),
            this, SLOT(switchToJob(Job)));

    connect(ui->printersCbx, SIGNAL(activated(int)),
            this, SLOT(switchPrinter()));

    connect(project, SIGNAL(changed()),
            this, SLOT(updateWidgets()));

    connect(ui->printerConfigBtn, SIGNAL(clicked()),
            this, SLOT(showPrinterSettingsDialog()));

    connect(project, SIGNAL(progress(int,int)),
            this, SLOT(updateProgressBar(int, int)), Qt::QueuedConnection);

    connect(ui->preview, SIGNAL(changed(int)),
            this, SLOT(updateWidgets()));

    connect(ui->preview, SIGNAL(changed(int)),
            ui->jobsView, SLOT(setSheetNum(int)));

    connect(ui->preview, SIGNAL(contextMenuRequested(int)),
            this, SLOT(showPreviewContextMenu(int)));

    connect(ui->jobsView, SIGNAL(contextMenuRequested(Job)),
            this, SLOT(showJobViewContextMenu(Job)));

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

 ************************************************/
void MainWindow::closeEvent(QCloseEvent *event)
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
    if (printer)
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
    act->setIcon(Icon::icon(Icon::Print));
    connect(act, SIGNAL(triggered()),
            this, SLOT(print()));

    act = ui->actionPrintAndClose;
    act->setIcon(Icon::icon(Icon::Print));
    connect(act, SIGNAL(triggered()),
            this, SLOT(printAndClose()));

    act = ui->actionExit;
    connect(act, SIGNAL(triggered()),
            this, SLOT(close()));

    act = ui->actionPreviousSheet;
    act->setIcon(Icon::icon(Icon::Previous));
    connect(act, SIGNAL(triggered()),
            ui->preview, SLOT(prevSheet()));

    act = ui->actionNextSheet;
    act->setIcon(Icon::icon(Icon::Next));
    connect(act, SIGNAL(triggered()),
            ui->preview, SLOT(nextSheet()));

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

    ui->actionPrint->setEnabled(project->pageCount() > 0);
    ui->actionPrintAndClose->setEnabled(ui->actionPrint->isEnabled());

    ui->actionPreviousSheet->setEnabled(ui->preview->currentSheet() > 0);
    ui->actionNextSheet->setEnabled(ui->preview->currentSheet() < project->previewSheetCount() - 1);

    ui->actionSave->setEnabled(project->pageCount() > 0);
    ui->actionSaveAs->setEnabled(ui->actionSave->isEnabled());

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

        QString pagesTmpl = (project->pageCount() > 1) ? tr("%1 pages", "Status bar") : tr("%1 page", "Status bar");
        QString sheetsTmpl = (sheetsCount > 1) ? tr("%1 sheets", "Status bar") : tr("%1 sheet", "Status bar");
        mStatusBarSheetsLabel.setText(pagesTmpl.arg(project->pageCount()) +
                                   " ( " + sheetsTmpl.arg(sheetsCount) + " )");

        mStatusBarCurrentSheetLabel.setText(tr("Sheet %1 of %2", "Status bar")
                                .arg(ui->preview->currentSheet() + 1)
                                .arg(project->previewSheetCount()));
    }
    else
    {
        mStatusBarSheetsLabel.clear();
        mStatusBarCurrentSheetLabel.clear();
    }

    ui->printerConfigBtn->setEnabled(ui->printersCbx->currentPrinter() != 0);
}


/************************************************

 ************************************************/
void MainWindow::showPrinterSettingsDialog()
{
    Printer *printer = ui->printersCbx->currentPrinter();
    if (printer)
    {
        PrinterSettings *dialog = PrinterSettings::create(printer);
        connect(dialog, SIGNAL(accepted()), this, SLOT(applyPrinterSettings()));
        dialog->show();
    }
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
void MainWindow::switchToJob(const Job &job)
{
    ProjectPage *page = job.firstVisiblePage();
    for (int i=0; i<project->previewSheetCount(); ++i)
    {
        const Sheet *sheet = project->previewSheet(i);

        if (sheet->indexOfPage(page) > -1)
        {
            ui->preview->setCurrentSheet(i);
            return;
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
bool MainWindow::print()
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
                 infoDialog = showPrintDialog(tr("Print the all pages on %1.").arg(project->printer()->printerName()));
             }


             res = project->printer()->print(keeper.sheets_1, "", false, 1);
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
                               "and click the Continue button.").arg(project->printer()->printerName()));

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
                infoDialog = showPrintDialog(tr("Print the even pages on %1.").arg(project->printer()->printerName()));

             res = project->printer()->print(keeper.sheets_2, "", false, 1);
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
            infoDialog = showPrintDialog(tr("Print the all pages on %1.").arg(project->printer()->printerName()));

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

        res = project->printer()->print(keeper.sheets_1, "", project->doubleSided(), 1);
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
void MainWindow::printAndClose()
{
    if (print())
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
void MainWindow::showPreviewContextMenu(int pageNum)
{
    int sheetNum = ui->preview->currentSheet();
    Sheet *sheet = 0;
    ProjectPage *page = 0;
    Job job;

    if (sheetNum > -1)
    {
        sheet = project->previewSheet(sheetNum);
        page = (pageNum < 0) ? 0 : sheet->page(pageNum);
    }

    if (page)
    {
        int n = project->jobs()->indexOfProjectPage(page);
        if (n<0)
            return;

        job = project->jobs()->at(n);
    }

    QMenu *menu = new QMenu(this);

    // New subbooklet ................................
    if (page && (page != project->page(0)))
    {
        if (!page->isStartSubBooklet())
        {
            PageAction *act = new PageAction(tr("Start new booklet from this page"), sheet, page, menu);
            connect(act, SIGNAL(triggered()),
                    this, SLOT(startBooklet()));
            menu->addAction(act);
        }
        else
        {
            PageAction *act = new PageAction(tr("Don't start new booklet from this page"), sheet, page, menu);
            connect(act, SIGNAL(triggered()),
                    this, SLOT(dontStartBooklet()));
            menu->addAction(act);
        }

        menu->addSeparator();
    }

    // New subbooklet ................................

    // Rotation ......................................
    if (page)
    {
        PageAction *act;

        act = new PageAction(Icon::icon(Icon::RotateLeft), tr("Rotate page to the left"), sheet, page, menu);
        menu->addAction(act);
        connect(act, SIGNAL(triggered()),
                this, SLOT(rotatePageLeft()));

        act = new PageAction(Icon::icon(Icon::RotateRight), tr("Rotate page to the right"), sheet, page, menu);
        menu->addAction(act);
        connect(act, SIGNAL(triggered()),
                this, SLOT(rotatePageRight()));
    }
    // Rotation ......................................


    // Blank page ....................................
    if (page)
    {
        PageAction *act;
        act = new PageAction(tr("Insert blank page before this page"), sheet, page, menu);
        menu->addAction(act);
        connect(act, SIGNAL(triggered()),
                this, SLOT(insertBlankPageBefore()));

        act = new PageAction(tr("Insert blank page after this page"), sheet, page, menu);
        menu->addAction(act);
        connect(act, SIGNAL(triggered()),
                this, SLOT(insertBlankPageAfter()));

    }
    // ...............................................

    menu->addSeparator();

    // Delete page ...................................
    if (page)
    {
        PageAction *act;
        act = new PageAction(tr("Delete this page"), sheet, page, menu);
        connect(act, SIGNAL(triggered()),
                this, SLOT(deletePage()));
        menu->addAction(act);

        int n = job.indexOfPage(page);
        bool hasVisible = false;
        for (int p=n+1; p<job.pageCount(); ++p)
        {
            if (job.page(p)->visible())
            {
                hasVisible = true;
                break;
            }
        }
        if (hasVisible)
        {
            act = new PageAction(tr("Delete pages until the end of the job."), sheet, page, menu);
            connect(act, SIGNAL(triggered()),
                    this, SLOT(deletePagesEnd()));
            menu->addAction(act);
        }
    }
    // ...............................................

    // Undo delete ...................................
    QMenu *undelMenu = menu->addMenu(tr("Undo delete"));
    undelMenu->setEnabled(false);

    for(int j=0; j<project->jobs()->count(); ++j)
    {
        Job job = project->jobs()->at(j);
        QMenu *jm = undelMenu->addMenu(QString("%1 %2").arg(j+1).arg(job.title()));

        for(int p=0; p<job.pageCount(); ++p)
        {
            ProjectPage *page = job.page(p);
            if (!page->visible())
            {
                PageAction *act;
                act = new PageAction(tr("Page %1", "'Undo deletion' menu item").arg(p+1), 0, page, menu);
                connect(act, SIGNAL(triggered()),
                        this, SLOT(undoDeletePage()));

                jm->addAction(act);
            }
        }
        jm->setEnabled(!jm->isEmpty());
        undelMenu->setEnabled(undelMenu->isEnabled() || jm->isEnabled());
    }
    // ...............................................

    menu->popup(QCursor::pos());
}


/************************************************

 ************************************************/
void MainWindow::showJobViewContextMenu(Job job)
{
    QMenu *menu = new QMenu(this);

    JobAction *act;

    act = new JobAction(tr("Rename job"), job, menu);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()),
            this, SLOT(renameJob()));


    act = new JobAction(Icon::icon(Icon::RotateLeft), tr("Rotate job to the left"), job, menu);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()),
            this, SLOT(rotateJobLeft()));

    act = new JobAction(Icon::icon(Icon::RotateRight), tr("Rotate job to the right"), job, menu);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()),
            this, SLOT(rotateJobRight()));


    menu->addSeparator();

    act = new JobAction(tr("Delete job"), job, menu);
    menu->addAction(act);
    connect(act, SIGNAL(triggered()),
            this, SLOT(deleteJob()));

    menu->popup(QCursor::pos());
}


/************************************************
 *
 * ***********************************************/
void MainWindow::deletePage()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;

    if (act->page()->isBlankPage())
    {
        int n = project->jobs()->indexOfProjectPage(act->page());
        if (n<0)
            return;

        project->jobs()->value(n).removePage(act->page());
    }
    else
    {
        act->page()->hide();
    }
    project->update();
}


/************************************************

 ************************************************/
void MainWindow::undoDeletePage()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;

    if (!act->page()->visible())
    {
        act->page()->show();
        project->update();
    }
}


/************************************************

 ************************************************/
void MainWindow::deletePagesEnd()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;

    int n = project->jobs()->indexOfProjectPage(act->page());
    if (n<0)
        return;

    Job job = project->jobs()->value(n);

    QList<ProjectPage*> deleted;
    for (int p=job.pageCount()-1; p>=job.indexOfPage(act->page()); --p)
    {
        ProjectPage *page = job.page(p);

        if (page->isBlankPage())
            deleted << page;
        else
            page->hide();
    }

    job.removePages(deleted);
}


/************************************************
 *
 * ***********************************************/
void MainWindow::insertBlankPageBefore()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;

    int j = project->jobs()->indexOfProjectPage(act->page());
    if (j<0)
        return;

    Job job = project->jobs()->value(j);
    int n = job.indexOfPage(act->page());
    job.insertBlankPage(n);
}


/************************************************
 *
 * ***********************************************/
void MainWindow::insertBlankPageAfter()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;

    int j = project->jobs()->indexOfProjectPage(act->page());
    if (j<0)
        return;

    Job job = project->jobs()->value(j);
    int n = job.indexOfPage(act->page());
    job.insertBlankPage(n+1);
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

    ProjectPage *page = act->page();
    page->setManualRotation(page->manualRotation() - Rotate90);
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

    ProjectPage *page = act->page();
    page->setManualRotation(page->manualRotation() + Rotate90);
    project->update();
}


/************************************************

 ************************************************/
void MainWindow::startBooklet()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;

    act->page()->setStartSubBooklet(true);
    project->update();

    int sheetNum = project->previewSheets().indexOfPage(act->page());
    if (sheetNum > -1)
        ui->preview->setCurrentSheet(sheetNum);
}


/************************************************

 ************************************************/
void MainWindow::dontStartBooklet()
{
    PageAction *act = qobject_cast<PageAction*>(sender());
    if (!act || !act->page())
        return;
    qDebug() << "";
    act->page()->setStartSubBooklet(false);
    project->update();

    int sheetNum = project->previewSheets().indexOfPage(act->page());
    if (sheetNum > -1)
        ui->preview->setCurrentSheet(sheetNum);
}


/************************************************

 ************************************************/
void MainWindow::showConfigDialog()
{
    ConfigDialog::createAndShow(this);
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
    }

    mSaveFile = file;
    settings->setValue(Settings::SaveDir, QFileInfo(file).path());


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
    Printer *prevPrinter = project->printer();
    project->setPrinter(mExportPrinter, false);
    print();
    project->setPrinter(prevPrinter, false);
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
