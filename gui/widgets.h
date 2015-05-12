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


#ifndef WIDGETS_H
#define WIDGETS_H

#include <QRadioButton>
#include <QComboBox>
#include <QListWidget>
#include <kernel/job.h>

class Printer;
class PrinterProfile;
class Layout;


class LayoutRadioButton : public QRadioButton
{
    Q_OBJECT
public:
    explicit LayoutRadioButton(QWidget *parent=0);
    explicit LayoutRadioButton(const QString &text, QWidget *parent=0);


    Layout *layout() const { return mLayout; }
    void setLayout(Layout *value) { mLayout = value; }
signals:
    
public slots:
    
private:
    Layout *mLayout;
};


class PrintersComboBox: public QComboBox
{
    Q_OBJECT
public:
    explicit PrintersComboBox(QWidget *parent = 0);
    ~PrintersComboBox();

    Printer *currentPrinter() const {return itemPrinter(currentIndex()); }
    int currentProfile() const { return itemProfile(currentIndex()); }
    void setCurrentPrinterProfile(const QString &printerName, int profileIndex = 0);
    void setCurrentPrinterProfile(const Printer *printer, int profileIndex = 0);

    int addPrinter(Printer *printer);
    int findItem(const QString &printerName, int profileIndex = 0) const;
    int findItem(const Printer *printer, int profileIndex = 0) const;

    int findFirstProfile() const;
private:
    QList<Printer*> mPrinters;

    int addProfile(Printer *printer, int profileIndex);
    Printer *itemPrinter(int index) const;
    int itemProfile(int index) const;
};


class JobsListView: public QListWidget
{
    Q_OBJECT
public:
    explicit JobsListView(QWidget *parent = 0);
    Job currentJob() const;

public slots:
    void setSheetNum(int sheetNum);
    void updateItems();

signals:
    void jobSelected(Job job);
    void contextMenuRequested(Job job);

private slots:
    void showContextMenu(const QPoint &pos);
    void layoutChanged();

protected:
    void mouseReleaseEvent(QMouseEvent *event);

};


#endif // WIDGETS_H
