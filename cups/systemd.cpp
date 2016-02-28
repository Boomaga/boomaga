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


#include "systemd.h"

#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusMetaType>
#include <QDebug>
#include <unistd.h>
#include <sys/types.h>
#include "common.h"

#define SYSTEMD_LOGIN_SERVICE       "org.freedesktop.login1"
#define SYSTEMD_MANAGER_INTERFACE   "org.freedesktop.login1.Manager"
#define SYSTEMD_SESSION_INTERFACE   "org.freedesktop.login1.Session"


/************************************************
 *
 ************************************************/
QString getActiveSessionDisplaySystemd()
{
    qDBusRegisterMetaType<SystemdSession>();
    qDBusRegisterMetaType<SystemdSessionList>();

    QDBusInterface dbusManager(SYSTEMD_LOGIN_SERVICE,
                               "/org/freedesktop/login1",
                               SYSTEMD_MANAGER_INTERFACE,
                               QDBusConnection::systemBus()
                               );

    if (!dbusManager.isValid())
    {
        debug(QString("Can't connect to %1.").arg(dbusManager.interface()));
        return "";
    }

    QDBusReply<SystemdSessionList> reply = dbusManager.call("ListSessions");
    if (!reply.isValid())
    {
        warning(QString("DBUS error: %1\n%2").arg(
                    reply.error().name(),
                    reply.error().message()));
        return "";
    }

    uid_t uid = getuid();
    QString res;
    foreach (const SystemdSession session, reply.value())
    {
        if (session.mUserId != uid)
            continue;

        QDBusInterface dbus(SYSTEMD_LOGIN_SERVICE,
                            session.mSessionPath.path(),
                            SYSTEMD_SESSION_INTERFACE,
                            QDBusConnection::systemBus()
                            );

        if (!dbus.isValid())
        {
            warning(QString("DBUS error: %1 %2 is invalid.").arg(
                        dbus.interface(),
                        dbus.path()));
            return "";
        }

        if (dbus.property("Type") != "x11")
            continue;

        QString display = qvariant_cast<QString>(dbus.property("Display"));

        if (!display.isEmpty())
            res = display;

        if (qvariant_cast<bool>(dbus.property("Active")))
            return res;
    }

    return res;
}


/************************************************
 *
 ************************************************/
QDBusArgument &operator<<(QDBusArgument &argument, const SystemdSession &session)
{
    argument.beginStructure();
    argument << session.mSessionId;
    argument << session.mUserId;
    argument << session.mUserName;
    argument << session.mSeatId;
    argument << session.mSessionPath;
    argument.endStructure();
    return argument;
}


/************************************************
 *
 ************************************************/
const QDBusArgument &operator>>(const QDBusArgument &argument, SystemdSession &session)
{
    argument.beginStructure();
    argument >> session.mSessionId;
    argument >> session.mUserId;
    argument >> session.mUserName;
    argument >> session.mSeatId;
    argument >> session.mSessionPath;
    argument.endStructure();
    return argument;
}


/************************************************
 *
 ************************************************/
QDebug operator<<(QDebug dbg, const SystemdSession &session)
{
    dbg.space() << "{"
                << "\n  SessionId: " << session.mSessionId
                << "\n  UserId:" << session.mUserId
                << "\n  UserName:" << session.mUserName
                << "\n  seatId" << session.mSeatId
                << "\n  SessionPath:" << session.mSessionPath.path()
                << "\n}";
    return dbg;
}

