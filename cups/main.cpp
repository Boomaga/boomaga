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


#include <QString>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QStringList>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>

#include "common.h"
#include "envinfo.h"
#include "inputfile.h"
#include "systemd.h"
#include "consolekit.h"


/************************************************
 *
 ************************************************/
void runGUI(const QString &dbusAddr, const QStringList &files, const QString &title, const QString &options, uint count)
{
    QDBusConnection dbus = QDBusConnection::connectToBus(dbusAddr, "boomaga");
    if (!dbus.isConnected())
    {
        error("Can't connect to org.boomaga DBus");
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(
                "org.boomaga",
                "/boomaga",
                "org.boomaga",
                "add");

    foreach (QString file, files)
    {
        QList<QVariant> args;
        args << file;
        args << title;
        args << true;
        args << options;
        args << count;
        msg.setArguments(args);

        QStringList sl;
        foreach (const QVariant &arg, args)
        {
            sl << "'" + arg.toString() +"'";
        }

        debug(QString("Start boomaga: %1").arg(sl.join(" ")));

        QDBusMessage reply = dbus.call(msg);

        if (reply.type() != QDBusMessage::ReplyMessage)
        {
            error(reply.errorName() + " : " + reply.errorMessage());
        }
    }
}


/************************************************

 ************************************************/
int main(int argc, char *argv[])
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    qInstallMsgHandler(messageOutput);
#else
    qInstallMessageHandler(messageOutput);
#endif

    QCoreApplication a(argc, argv);

    if (argc < 4)
    {
        info("Usage: <jobId> <title> <count> <options> <user>.");
        info("boomaga backend takes the file from standard input.");
        error(QString("Not enough arguments, expected 4, got %1.").arg(argc));
    }


    QString jobId = QString::fromLocal8Bit(argv[1]);
    QString title = QString::fromLocal8Bit(argv[2]);
    bool ok;
    int count = QString::fromLocal8Bit(argv[3]).toInt(&ok);
    if (!ok)
        error("Invalid usage: job-id title copies options");

    QString options = QString::fromLocal8Bit(argv[4]);
    QString user = qgetenv("USER");

    info(QString("jobId:   %1").arg(jobId));
    info(QString("title:   %1").arg(title));
    info(QString("count:   %1").arg(count));
    info(QString("options: %1").arg(options));
    info(QString("user:    %1").arg(user));


    // Get Xdisplay .............................
    QString xDisplay;
#ifdef Q_OS_LINUX
    xDisplay = getActiveSessionDisplaySystemd();
    if (xDisplay.isEmpty())
        xDisplay = getActiveSessionDisplayConsoleKit();
#else
    xDisplay = getActiveSessionDisplayConsoleKit();
#endif

    if (xDisplay.isEmpty())
        warning(QString("Can't found active session for user '%1'.").arg(user));
    else
        debug(QString("xDisplay: %1").arg(xDisplay));

    // Find D-Bus address .......................
    EnvInfo envInfo = EnvInfo::find(xDisplay);
    if (envInfo.dbusAddr().isEmpty())
    {
        error(QString("Can't extract D-Bus bus address for user \"%1\" and session \"%2\"").arg(user, xDisplay));
    }
    else
    {
        debug(QString("exe name %1").arg(envInfo.exeName()));
        debug(QString("D-Bus address %1").arg(envInfo.dbusAddr()));
    }

    // Cahe dir .................................
    QString cacheDir = envInfo.getEnv("XDG_CACHE_HOME");
    if (cacheDir.isEmpty())
        cacheDir = QDir::homePath() + QLatin1String("/.cache");

    debug(QString("Cache dir: %1").arg(cacheDir));

    if (!QDir(cacheDir).mkpath("."))
        error(QString("Can't create chache directory %1").arg(cacheDir));

    // For the beauty of the GUI, certain environment variables must be set.
    // If the program is running through DBus variables isn't set. Therefore,
    // we copy them to boomaga.env file. GUI uses this file at startup.
    envInfo.save(QString("%1/boomaga.env").arg(cacheDir));

    // Prepare input files ......................
    QStringList files = createJobFiles(jobId, cacheDir);
    debug(QString("Job files: %1").arg(files.join(", ")));

    runGUI(envInfo.dbusAddr(), files, title, options, count);

    return CUPS_BACKEND_OK;
}
