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


#include "dbus.h"
#include "kernel/psproject.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>

/************************************************

 ************************************************/
DBusProjectAdaptor::DBusProjectAdaptor(PsProject *project) :
    QDBusAbstractAdaptor(project),
    mProject(project)
{
}


/************************************************

 ************************************************/
bool DBusProjectAdaptor::openFileInExisting(const QString &fileName)
{
    QDBusInterface remote("org.boomaga", "/Project");
    if (!remote.isValid())
        return false;

    QDBusMessage res = remote.call("addFile", fileName);
    if (res.errorName().isEmpty())
        return true;

    qWarning() << res.errorMessage();
    return false;
}


/************************************************

 ************************************************/
void DBusProjectAdaptor::addFile(const QString &fileName)
{
    mProject->addFile(fileName);
}
