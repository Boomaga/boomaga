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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QMouseEvent>
#include <kernel/job.h>

class Layout;
class Project;
class Printer;
class QMessageBox;
class ExportToPDFPrinter;
class Sheet;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void switchLayout();
    void switchPrinterProfile();

    bool print(uint count = 1, bool collate = true);
    void printAndClose(uint count = 1, bool collate = true);
    void printWithOptions();

    void updateWidgets();
    void showPrinterSettingsDialog();
    void applyPrinterSettings();

    void showAboutDialog();

    void updateProgressBar(int value, int all);

    void showPreviewContextMenu(Sheet *sheet, ProjectPage *page);
    void showJobViewContextMenu(Job job);
    void showEditPageMainMenu();
    void showEditJobMainMenu();
    void deletePage();
    void undoDeletePage();
    void deletePagesEnd();
    void insertBlankPageBefore();
    void insertBlankPageAfter();
    void cloneJob();
    void deleteJob();
    void renameJob();
    void rotateJobLeft();
    void rotateJobRight();
    void rotatePageLeft();
    void rotatePageRight();

    void startBooklet();
    void dontStartBooklet();

    void showConfigDialog();


    void save();
    void saveAs(const QString &fileName = "");
    void exportAs();
    void load();

    void fillRecentFiles();
    void loadAuto();

protected:
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    Ui::MainWindow *ui;

    QList<Layout*> mAvailableLayouts;

    QLabel mStatusBarSheetsLabel;
    QLabel mStatusBarCurrentSheetLabel;

    QProgressBar mProgressBar;
    QString      mSaveFile;

    void fillPrintersCombo();
    void initActions();
    void initStatusBar();

    void loadSettings();
    void saveSettings();

    QMessageBox *showPrintDialog(const QString &text);
    void saveAuto();

    void addToRecentFiles(const QString &file);
    void fillPageEditMenu(ProjectPage *page, QMenu *menu);
    void fillJobEditMenu(const Job job, QMenu *menu);
};

#endif // MAINWINDOW_H
