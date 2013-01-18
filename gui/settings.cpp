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


#include "settings.h"

#define MAINWINDOW_GROUP "MainWindow"
#define PROJECT_GROUP "Project"
#define PRINTERS_GROUP "Printer"

/************************************************

 ************************************************/
Settings *Settings::instance()
{
    static Settings *inst = 0;
    if (!inst)
    {
        inst = new Settings();
    }
    return inst;
}


/************************************************

 ************************************************/
Settings::Settings():
    mSettings("boomaga", "boomaga")
{

}


/************************************************

 ************************************************/
void Settings::sync()
{
    mSettings.sync();
}


/************************************************

 ************************************************/
QByteArray Settings::mainWindowGeometry()
{
    return mSettings.value(MAINWINDOW_GROUP "/Geometry").toByteArray();
}


/************************************************

 ************************************************/
void Settings::setMainWindowGeometry(const QByteArray &value)
{
    mSettings.setValue(MAINWINDOW_GROUP "/Geometry", value);
}


/************************************************

 ************************************************/
QByteArray Settings::mainWindowState()
{
    return mSettings.value(MAINWINDOW_GROUP "/State").toByteArray();
}


/************************************************

 ************************************************/
void Settings::setMainWindowState(const QByteArray &value)
{
    mSettings.setValue(MAINWINDOW_GROUP "/State", value);
}


/************************************************

 ************************************************/
PsProject::PsLayout Settings::layout()
{
    return PsProject::strToLayout(
                mSettings.value(PROJECT_GROUP "/Layout", "").toString()
                );
}


/************************************************

 ************************************************/
void Settings::setLayout(PsProject::PsLayout value)
{
    mSettings.setValue(PROJECT_GROUP "/Layout",
                       PsProject::layoutToStr(value)
                       );
}


/************************************************

 ************************************************/
QString Settings::currentPrinter() const
{
    return mSettings.value(PROJECT_GROUP "/Printer", "").toString();
}


/************************************************

 ************************************************/
void Settings::setCurrentPrinter(const QString &value)
{
    mSettings.setValue(PROJECT_GROUP "/Printer", value);
}


/************************************************

 ************************************************/
QString printersKey(const QString &printerName, const QString &key)
{
    return QString("%1_%2/%3").arg(PRINTERS_GROUP, printerName, key);
}

/************************************************

 ************************************************/
bool Settings::printerDuplex(const QString &printerName, bool defaultValue)
{
    return mSettings.value(printersKey(printerName, "Duplex"), defaultValue).toBool();
}


/************************************************

 ************************************************/
void Settings::setPrinterDuplex(const QString &printerName, bool value)
{
    mSettings.setValue(printersKey(printerName, "Duplex") , value);
}


/************************************************

 ************************************************/
bool Settings::printerBorder(const QString &printerName, bool defaultValue)
{
    return mSettings.value(printersKey(printerName, "Border"), defaultValue).toBool();
}


/************************************************

 ************************************************/
void Settings::setPrinterBorder(const QString &printerName, bool value)
{
   mSettings.setValue(printersKey(printerName, "Border") , value);
}


/************************************************

 ************************************************/
QSize Settings::printerPageSize(const QString &printerName, QSize defaultValue)
{
    return mSettings.value(printersKey(printerName, "PageSize"), defaultValue).toSize();
}


/************************************************

 ************************************************/
void Settings::setPrinterPageSize(const QString &printerName, QSize value)
{
    mSettings.setValue(printersKey(printerName, "PageSize") , value);
}


/************************************************

 ************************************************/
double Settings::printerMargin(const QString &printerName, const QString &margin, double defaultValue)
{
    return mSettings.value(printersKey(printerName, margin + "Margin"), defaultValue).toDouble();
}

/************************************************

 ************************************************/
void Settings::setPrinterMargin(const QString &printerName, const QString &margin, double value)
{
    mSettings.setValue(printersKey(printerName, margin + "Margin") , value);
}


