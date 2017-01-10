/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2015 Boomaga team https://github.com/Boomaga
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


#ifndef PRINTDIALOG_H
#define PRINTDIALOG_H

#include <QDialog>

namespace Ui {
class PrintDialog;
}

class PrintDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PrintDialog(QWidget *parent = 0);
    ~PrintDialog();

    int copies() const;
    void setCopies(int value);

    bool collate() const;
    void setCollate(bool value);

public slots:
    void done(int res);

private slots:
    void CopiesEditValueChanged(int value);
    void loadSettings();
    void saveSettings();

private:
    Ui::PrintDialog *ui;
};

#endif // PRINTDIALOG_H
