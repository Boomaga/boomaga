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


#ifndef PRINTERSETTINGS_H
#define PRINTERSETTINGS_H

#include "kernel/printer.h"
#include <QDialog>
#include <QList>
#include <QHash>

namespace Ui {
class PrinterSettings;
}

class QAbstractButton;
class ProfileItem;


class PrinterSettings : public QDialog
{
    Q_OBJECT

public slots:
    static PrinterSettings *execute(Printer *printer);

public:
    explicit PrinterSettings(QWidget *parent = 0);
    ~PrinterSettings();
    
    Printer *currentPrinter() const { return mPrinter; }
    void setCurrentPrinter(Printer *printer);

    void save();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void updateWidgets();
    void updateProfile();
    void updatePreview();

    void btnClicked(QAbstractButton *button);
    void addProfile();
    void delProfile();
    void profileRenamed(QWidget * editor);
    void resetToDefault();

private:
    Ui::PrinterSettings *ui;
    Printer *mPrinter;
    Unit mUnit;

    void applyUpdates();
    ProfileItem *currentItem() const;
    PrinterProfile *currentProfile() const;
};

#endif // PRINTERSETTINGS_H
