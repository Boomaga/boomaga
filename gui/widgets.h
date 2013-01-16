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


#ifndef WIDGETS_H
#define WIDGETS_H

#include "kernel/psproject.h"

#include <QRadioButton>
#include <QComboBox>
#include <QListWidget>

class Printer;

class LayoutRadioButton : public QRadioButton
{
    Q_OBJECT
public:
    explicit LayoutRadioButton(QWidget *parent=0);
    explicit LayoutRadioButton(const QString &text, QWidget *parent=0);


    PsProject::PsLayout psLayout() const { return mPsLayout; }
    void setPsLayout(PsProject::PsLayout value) { mPsLayout = value; }
signals:
    
public slots:
    
private:
    PsProject::PsLayout mPsLayout;
};


class PrintersComboBox: public QComboBox
{
    Q_OBJECT
public:
    explicit PrintersComboBox(QWidget *parent = 0);
    ~PrintersComboBox();

    Printer *currentPrinter();
    void setCurrentPrinter(const QString &printerName);

    int addPrinter(Printer *printer);

    Printer *itemPrinter(int index);

private:
    QList<Printer*> mPrinters;
};


class PsFilesListView: public QListWidget
{
    Q_OBJECT
public:
    explicit PsFilesListView(QWidget *parent = 0);

    PsProject *project() const { return mProject; }
    void setProject(PsProject *project);

signals:
    void fileSelected(PsFile *file);

private slots:
    void updateItems();
    void showContextMenu(const QPoint &pos);
    void deleteFile();

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    PsProject *mProject;
};


#endif // WIDGETS_H
