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


#include "dbus.h"
#include "kernel/project.h"
#include "../common.h"
#include "finddbusaddress.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QProcessEnvironment>
#include <QDebug>

using namespace std;

/************************************************

 ************************************************/
BoomagaDbus::BoomagaDbus(const QString &serviceName, const QString &dbusPath):
    QObject()
{
    QDBusConnection::sessionBus().registerService(serviceName);
    QDBusConnection::sessionBus().registerObject(dbusPath, this, QDBusConnection::ExportAllSlots);
}


/************************************************

 ************************************************/
BoomagaDbus::~BoomagaDbus()
{
}


/************************************************

 ************************************************/
#if MAJOR_VERSION > 3
    #warning "Remove legacy DBUS support!!!"
#endif
void BoomagaDbus::add(const QString &file, const QString&, bool, const QString &, uint)
{
    add(file);
}


/************************************************
 *
 ************************************************/
void BoomagaDbus::add(const QString &file)
{
    QMetaObject::invokeMethod(this, "doAdd", Qt::QueuedConnection, Q_ARG(QString, file));
}


/************************************************

 ************************************************/
void BoomagaDbus::doAdd(const QString &file)
{
    project->load(file);
}


/************************************************
 *
 ************************************************/
static bool doRunBoomaga(const QString &dbusAddress, const QList<QVariant> &args)
{
    Log::debug("Try to start boomaga via DBus %s", dbusAddress.toLocal8Bit().data());
    QDBusConnection dbus = QDBusConnection::connectToBus(dbusAddress, "boomaga");
    if (!dbus.isConnected())
    {

        Log::debug("Can't connect to org.boomaga DBus %s", dbusAddress.toLocal8Bit().data());
        return false;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(
                "org.boomaga",
                "/boomaga",
                "org.boomaga",
                "add");

    msg.setArguments(args);

    QDBusMessage reply = dbus.call(msg);
    if (reply.type() != QDBusMessage::ReplyMessage)
    {
        Log::warn("Can't start boomaga  via DBus %s: %s",
             reply.errorName().toLocal8Bit().constData(),
             reply.errorMessage().toLocal8Bit().constData());

        return false;
    }

    Log::debug("The boomaga successfully started via DBus %s", dbusAddress.toLocal8Bit().data());
    return true;
}


/************************************************
 *
 ************************************************/
bool BoomagaDbus::runBoomaga(const QString &file)
{
    QList<QVariant> args;
    args << file;

    {
        string s;
        foreach (auto a, args)
            s +=  " " + a.toString().toStdString();
        Log::debug("Start boomaga: %s", s.c_str());
    }

    {
        QString addr = QProcessEnvironment::systemEnvironment().value("DBUS_SESSION_BUS_ADDRESS");
        if (!addr.isEmpty() && doRunBoomaga(addr, args))
            return true;
    }

    foreach (auto &addr, FindDbusAddress::fromSessionFiles())
    {
        if (doRunBoomaga(addr, args))
            return true;
    }

#ifdef Q_OS_LINUX
    foreach (auto &addr, FindDbusAddress::fromProcFiles())
    {
        if (doRunBoomaga(addr, args))
            return true;
    }
#endif

#ifdef Q_OS_FREEBSD
    foreach (auto &addr, FindDbusAddress::fromProcStat())
    {
        if (doRunBoomaga(addr, args))
            return true;
    }
#endif

    Log::error("Can't start boomaga gui.");
    return false;
}
