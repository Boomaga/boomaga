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

#include "kernel/project.h"
#include <QSettings>
#include <QSet>
#include "boomagatypes.h"

class Settings : public QSettings
{
    Q_OBJECT
public:
    enum Key{
        Layout,
        Printer,
        DoubleSided,

        SaveDir,
        SubBookletsEnabled,
        SubBookletSize,

        // Printer ******************************
        Printer_CurrentProfile,
        Printer_CopiesNum,
        Printer_CollateCopies,

        // PrinterProfile ******************************
        PrinterProfile_Name,
        PrinterProfile_DuplexType,
        PrinterProfile_DrawBorder,
        PrinterProfile_ReverseOrder,

        PrinterProfile_LeftMargin,
        PrinterProfile_RightMargin,
        PrinterProfile_TopMargin,
        PrinterProfile_BottomMargin,
        PrinterProfile_InternalMargin,

        // PrinterSettingsDialog ****************
        PrinterSettingsDialog_Geometry,

        // MainWindow ***************************
        MainWindow_Geometry,
        MainWindow_State,
        MainWindow_SplitterPos,
        MainWindow_PageListIconSize,
        MainWindow_PageListTab,

        // PrinterDialog ************************
        PrinterDialog_Geometry,

        // ExportPDF ****************************
        ExportPDF_FileName

    };

    static Settings *instance();
    static void setFileName(const QString &fileName);

    QVariant value(Key key, const QVariant &defaultValue = QVariant()) const;
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    void setValue(Key key, const QVariant &value);
    void setValue(const QString &key, const QVariant &value);

private:
    explicit Settings(const QString &organization, const QString &application);
    explicit Settings(const QString &fileName);
    virtual ~Settings();

    void init();
    void setDefaultValue(const QString &key, const QVariant &defaultValue);
    void setDefaultValue(Key key, const QVariant &defaultValue);

    QString keyToString(Key key) const;
    static QString mFileName;
};

#define settings Settings::instance()


#endif // SETTINGS_H
