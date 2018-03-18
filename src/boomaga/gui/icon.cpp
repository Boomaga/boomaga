/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2014 Boomaga team https://github.com/Boomaga
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


#include "icon.h"

#include <QHash>
#include <QDebug>
#include <QDir>

/************************************************
 *
 ************************************************/
QIcon loadIcon(const QString &iconName, bool loadDisable)
{
    QVector<int> sizes;
    sizes << 16 << 22 << 24 << 32 << 48 << 64 << 128 << 256 << 512;

    QIcon res;
    foreach (int size, sizes)
        res.addFile(QString(":%2/%1").arg(iconName).arg(size), QSize(size, size), QIcon::Normal);


    if (loadDisable)
    {
        foreach (int size, sizes)
            res.addFile(QString(":%2/%1_disable").arg(iconName).arg(size), QSize(size, size), QIcon::Disabled);
    }

    return res;
}


/************************************************
 *
 ************************************************/
QIcon loadMainIcon(const QString &iconName)
{
    if (QIcon::themeName() == "hicolor")
    {
        QStringList failback;
        failback << "oxygen";
        failback << "Tango";
        failback << "Prudence-icon";
        failback << "Humanity";
        failback << "elementary";
        failback << "gnome";


        QDir usrDir("/usr/share/icons/");
        QDir usrLocalDir("/usr/local/share/icons/");
        foreach (QString s, failback)
        {
            if (usrDir.exists(s) || usrLocalDir.exists(s))
            {
                QIcon::setThemeName(s);
                break;
            }
        }
    }

    return QIcon::fromTheme(iconName, loadIcon("mainicon", false));
}
