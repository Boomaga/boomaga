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

class Layout;
class Project;
class Printer;
class InputFile;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
    QList<Printer*> availablePrinters();

private slots:
    void switchLayout();
    void switchPrinter();

    void switchToFile(InputFile *file);

    void print(bool close = false);
    void printAndClose();

    void updateWidgets();

    void showPrinterSettingsDialog();
    void applyPrinterSettings();

    void showAboutDialog();

    void updateProgressBar(int value, int all);

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;

    QList<Printer*> mAvailablePrinters;
    QList<Layout*> mAvailableLayouts;

    QLabel mStatusBarSheetsLabel;
    QLabel mStatusBarCurrentSheetLabel;

    QProgressBar mProgressBar;

    void initActions();
    void initStatusBar();

    void loadSettings();
    void saveSettings();
};

#endif // MAINWINDOW_H
