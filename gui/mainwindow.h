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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

class PsProject;
class PsRender;
class QTemporaryFile;
class PsFile;
class Printer;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(PsProject *project, QWidget *parent = 0);
    ~MainWindow();
    
    PsProject *project() const { return mProject; }

    QList<Printer*> availablePrinters();

private slots:
    void switchLayout();
    void switchPrinter();

    void switchToFile(PsFile *file);

    void print();
    void printAndClose();

    void updateCurrentSheet();
    void updateWidgets();
    void updateStatusBar();

    void showPrinterSettingsDialog();
    void applyPrinterSettings();

    void showPrevSheet();
    void showNextSheet();

    void psViewWhell(int delta);

private:
    Ui::MainWindow *ui;
    PsProject *mProject;
    PsRender *mRender;
    int mCurrentSheet;

    QList<Printer*> mAvailablePrinters;

    QLabel mStatusBarSheetsLabel;
    QLabel mStatusBarCurrentSheetLabel;

    void initActions();
    void initStatusBar();

    void loadSettings();
    void saveSettings();
    QTemporaryFile *getTmpFile();

    int currentSheet() const { return mCurrentSheet; }
    void setCurrentSheet(int value);
};

#endif // MAINWINDOW_H
