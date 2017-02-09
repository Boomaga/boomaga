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


#include "settings.h"
#include <QDir>
#include <QDebug>

#define MAINWINDOW_GROUP "MainWindow"
#define PROJECT_GROUP "Project"
#define PRINTERS_GROUP "Printer"

QString Settings::mFileName;

/************************************************

 ************************************************/
Settings *Settings::instance()
{
    static Settings *inst = 0;

    if (!inst)
    {
        if (mFileName.isEmpty())
            inst = new Settings("boomaga", "boomaga");
        else
            inst = new Settings(mFileName);
    }

    return inst;
}


/************************************************

 ************************************************/
void Settings::setFileName(const QString &fileName)
{
    mFileName = fileName;
}


/************************************************

 ************************************************/
Settings::Settings(const QString &organization, const QString &application):
    QSettings(organization, application)
{
    init();
}


/************************************************

 ************************************************/
Settings::Settings(const QString &fileName):
    QSettings(fileName, QSettings::IniFormat)
{
    init();
}


/************************************************

 ************************************************/
Settings::~Settings()
{

}

/************************************************

 ************************************************/
QString Settings::keyToString(Settings::Key key) const
{
    switch (key)
    {
    case Layout:                        return "Project/Layout";
    case Printer:                       return "Project/Printer";
    case DoubleSided:                   return "Project/DoubleSided";

    case SaveDir:                       return "Project/SaveDir";
    case SubBookletsEnabled:            return "Project/SubBookletsEnable";
    case SubBookletSize:                return "Project/SubBookletSize";

    // Printer ******************************
    case Printer_CurrentProfile:        return "CurrentProfile";
    case Printer_CopiesNum:             return "Printer/CopiesNum";
    case Printer_CollateCopies:         return "Printer/CollateCopies";

    // PrinterProfile ***********************
    case PrinterProfile_Name:           return "Name";
    case PrinterProfile_DuplexType:     return "DuplexType";
    case PrinterProfile_DrawBorder:     return "Border";
    case PrinterProfile_ReverseOrder:   return "ReverseOrder";

    case PrinterProfile_LeftMargin:     return "LeftMargin";
    case PrinterProfile_RightMargin:    return "RightMargin";
    case PrinterProfile_TopMargin:      return "TopMargin";
    case PrinterProfile_BottomMargin:   return "BottomMargin";
    case PrinterProfile_InternalMargin: return "InternalMargin";
    case PrinterProfile_ColorMode:      return "ColorMode";

    // PrinterSettingsDialog ****************
    case PrinterSettingsDialog_Geometry:return "PrinterSettingsDialog/Geometry";

    // MainWindow **************************
    case MainWindow_Geometry:           return "MainWindow/Geometry";
    case MainWindow_State:              return "MainWindow/State";
    case MainWindow_SplitterPos:        return "MainWindow/SplitterPos";
    case MainWindow_PageListIconSize:   return "MainWindow/PageListIconSize";
    case MainWindow_PageListTab:        return "MainWindow/PageListTab";

    // PrinterDialog ************************
    case PrinterDialog_Geometry:        return "PrinterDialog/Geometry";

    // ExportPDF ****************************
    case ExportPDF_FileName:            return "ExportPDF/FileName";

    }

    return "";
}


/************************************************

 ************************************************/
void Settings::init()
{
    setIniCodec("UTF-8");
    setDefaultValue(Layout,   "1up");
    setDefaultValue(DoubleSided, true);
    setDefaultValue(ExportPDF_FileName, tr("~/Untitled.pdf"));
    setDefaultValue(SaveDir, QDir::homePath());
    setDefaultValue(SubBookletsEnabled, true);
    setDefaultValue(SubBookletSize, 20);
    setDefaultValue(MainWindow_PageListIconSize, 64);
    setDefaultValue(MainWindow_PageListTab, 0);
}


/************************************************

 ************************************************/
void Settings::setDefaultValue(const QString &key, const QVariant &defaultValue)
{
    setValue(key, value(key, defaultValue));
}


/************************************************

 ************************************************/
void Settings::setDefaultValue(Settings::Key key, const QVariant &defaultValue)
{
    setValue(key, value(key, defaultValue));
}


/************************************************

 ************************************************/
QVariant Settings::value(Settings::Key key, const QVariant &defaultValue) const
{
    return value(keyToString(key), defaultValue);
}


/************************************************

 ************************************************/
QVariant Settings::value(const QString &key, const QVariant &defaultValue) const
{
    return QSettings::value(key, defaultValue);
}


/************************************************

 ************************************************/
void Settings::setValue(Settings::Key key, const QVariant &value)
{
    setValue(keyToString(key), value);
}


/************************************************

 ************************************************/
void Settings::setValue(const QString &key, const QVariant &value)
{
    QSettings::setValue(key, value);
}



