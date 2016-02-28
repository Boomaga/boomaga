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


#ifndef SYSTEMD_H
#define SYSTEMD_H

#include <QString>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusInterface>

QString getActiveSessionDisplaySystemd();

struct SystemdSession{
    QString mSessionId;
    quint32 mUserId;
    QString mUserName;
    QString mSeatId;
    QDBusObjectPath mSessionPath;
};
Q_DECLARE_METATYPE(SystemdSession)

typedef QList<SystemdSession> SystemdSessionList;
Q_DECLARE_METATYPE(SystemdSessionList)

QDBusArgument &operator<<(QDBusArgument &argument, const SystemdSession &session);
const QDBusArgument &operator>>(const QDBusArgument &argument, SystemdSession &session);

QDebug operator<<(QDebug dbg, const SystemdSession &session);


#endif // SYSTEMD_H
