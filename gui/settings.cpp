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
    case Layout:                    return "Project/Layout";
    case Printer:                   return "Project/Printer";
    case DoubleSided:               return "Project/DoubleSided";

        // Printer ******************************
    case Printer_DuplexType:        return "DuplexType";
    case Printer_DrawBorder:        return "Border";
    case Printer_ReverseOrder:      return "ReverseOrder";

    case Printer_LeftMargin:        return "LeftMargin";
    case Printer_RightMargin:       return "RightMargin";
    case Printer_TopMargin:         return "TopMargin";
    case Printer_BottomMargin:      return "BottomMargin";
    case Printer_InternalMargin:    return "InternalMargin";


    // MainWindow **************************
    case MainWindow_Geometry:       return "MainWindow/Geometry";
    case MainWindow_State:          return "MainWindow/State";


    // ExportPDF ****************************
    case ExportPDF_FileName:        return "ExportPDF/FileName";

    }

    return "";
}


/************************************************

 ************************************************/
void Settings::init()
{
    setDefaultValue(Layout,   "1up");
    setDefaultValue(DoubleSided, true);
    setDefaultValue(ExportPDF_FileName, tr("~/Untitled.pdf"));
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
QVariant Settings::printerValue(const QString &printerId, Settings::Key key, const QVariant &defaultValue) const
{
    QString keyStr = QString("Printer_%1/%2").arg(printerId, keyToString(key));
    if (key == Printer_DuplexType)
    {
        QString s = value(keyStr, duplexTypetoStr(static_cast<DuplexType>(defaultValue.toInt()))).toString();
        return strToDuplexType(s);
    }

    return value(keyStr, defaultValue);
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


/************************************************

 ************************************************/
void Settings::setPrinterValue(const QString &printerId, Key key, const QVariant &value)
{
    QVariant v;
    if (key == Printer_DuplexType)
    {
        v = duplexTypetoStr(static_cast<DuplexType>(value.toInt()));
    }
    else
        v = value;

    setValue(QString("Printer_%1/%2").arg(printerId, keyToString(key)), v);
}




