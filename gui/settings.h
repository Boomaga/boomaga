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


#ifndef SETTINGS_H
#define SETTINGS_H

#include "kernel/psproject.h"
#include <QSettings>

class Settings
{
public:
    static Settings *instance();

    QByteArray mainWindowGeometry();
    void setMainWindowGeometry(const QByteArray &value);

    QByteArray mainWindowState();
    void setMainWindowState(const QByteArray &value);


    PsProject::PsLayout layout();
    void setLayout(PsProject::PsLayout value);

    QString currentPrinter() const;
    void setCurrentPrinter(const QString &value);

    bool printerDuplex(const QString &printerName, bool defaultValue);
    void setPrinterDuplex(const QString &printerName, bool value);

    bool printerBorder(const QString &printerName, bool defaultValue);
    void setPrinterBorder(const QString &printerName, bool value);

    QSize printerPageSize(const QString &printerName, QSize defaultValue);
    void setPrinterPageSize(const QString &printerName, QSize value);

    double printerMargin(const QString &printerName, const QString &margin, double defaultValue);
    void setPrinterMargin(const QString &printerName, const QString &margin, double value);

    void sync();
protected:
    Settings();

private:
    QSettings mSettings;

};

#define settings Settings::instance()

#endif // SETTINGS_H
