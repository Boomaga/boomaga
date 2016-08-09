/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2016 Boomaga team https://github.com/Boomaga
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


#include "consolekit.h"

#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusMetaType>
#include <QDebug>
#include <unistd.h>
#include <sys/types.h>
#include "common.h"


/************************************************
 *
 ************************************************/
QString getActiveSessionDisplayConsoleKit()
{
    QDBusInterface dbusManager("org.freedesktop.ConsoleKit",
                               "/org/freedesktop/ConsoleKit/Manager",
                               "org.freedesktop.ConsoleKit.Manager",
                               QDBusConnection::systemBus()
                               );

    if (!dbusManager.isValid())
    {
        debug(QString("Can't connect to %1.").arg(dbusManager.interface()));
        return "";
    }

    uid_t uid = getuid();
    QDBusReply<QList<QDBusObjectPath> > paths = dbusManager.call("GetSessionsForUnixUser", uid);
    if (!paths.isValid())
    {
        warning(QString("DBUS error: %1\n%2").arg(
                    paths.error().name(),
                    paths.error().message()));
        return "";
    }

    QString res;
    foreach (const QDBusObjectPath path, paths.value())
    {

        QDBusInterface dbus("org.freedesktop.ConsoleKit",
                            path.path(),
                            "org.freedesktop.ConsoleKit.Session",
                            QDBusConnection::systemBus()
                            );

        if (!dbus.isValid())
        {
            warning(QString("DBUS error: %1 %2 is invalid.").arg(
                        dbus.interface(),
                        dbus.path()));
            return "";
        }

        QString xDisplay = QDBusReply<QString>(dbus.call("GetX11Display"));
        if (!xDisplay.isEmpty())
            res = xDisplay;

        if (QDBusReply<bool>(dbus.call("IsActive")))
            return res;
    }

    return res;
}
