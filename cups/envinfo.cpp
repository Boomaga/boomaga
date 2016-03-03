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


#include "envinfo.h"
#include "common.h"
#include <QStringList>
#include <unistd.h>
#include <sys/types.h>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QProcess>

#define PREFERED_APPS \
    "kdeinit4" << \
    "firefox" << \
    "chromium-browser" << \
    "razor-session" << \
    "gnome-session" << \
    "lxsession" << \
    "Xfce4-session"

#define ENV_VARS \
    "DISPLAY" << \
    "DBUS_SESSION_BUS_ADDRESS" << \
    "XDG_CACHE_HOME" << \
    "KDE_FULL_SESSION" << \
    "QT_PLUGIN_PATH" << \
    "GTK_RC_FILES" << \
    "GTK2_RC_FILES"


#ifdef Q_OS_FREEBSD
/************************************************
 *
 ************************************************/
QString getValue(const QString &data, const QString &key)
{
    int start = data.indexOf(key);
    if (start < 0)
        return "";
    start += key.length() + 1;

    int end = data.indexOf(' ', start);
    if (end > -1 )
        return data.mid(start, end - start -1);
    else
        return data.right(data.length() - start);
}


/************************************************
 * FreeBSD
 * read information from procstat
 ************************************************/
EnvInfo::EnvInfo(const QString &procDir)
{
    QProcess proc;
    QStringList args;
    args << "-eh";
    args << QFileInfo(procDir).baseName();

    proc.setProcessChannelMode(QProcess::SeparateChannels);
    proc.start("procstat", args);
    proc.waitForFinished();
    if (proc.exitStatus() != 0 || proc.exitCode() != 0)
    {
        QString msg = QString::fromLocal8Bit(proc.readAllStandardError());
        warning(QString("procstat exit with code %1: %2")
              .arg(proc.exitCode())
              .arg(msg));
        return;
    }

    QString line = QString::fromLocal8Bit(proc.readAllStandardOutput()).simplified();
    mExeName = line.section(' ', 1, 1);
    QString envData = line.section(' ', 2);

    QStringList extEnv;
    extEnv <<  ENV_VARS;

    foreach (QString name, extEnv)
    {
        setEnv(name, getValue(envData, name));
    }
}
#endif // FreeBSD


#ifdef Q_OS_LINUX

/************************************************
 *
 ************************************************/
QString getValue(const QByteArray &data, const QString &key)
{
    int b = data.indexOf(key + "=");
    if (b < 0)
        return "";

    b += key.length() + 1;
    int e = data.indexOf('\x0', b);
    if (e < 0)
        return QString::fromLocal8Bit(data.right(e-b));
    else
        return QString::fromLocal8Bit(data.mid(b, e-b));

}


/************************************************
 *
 ************************************************/
QString readCommFile(const QString &procDir)
{
    QFile file(procDir + "/comm");
    file.open(QFile::ReadOnly);
    QByteArray data  = file.readAll();
    file.close();
    return QString::fromLocal8Bit(data.left(data.length() - 1));
}


/************************************************
 * Linux
 * read all information from /proc files
 ************************************************/
EnvInfo::EnvInfo(const QString &procDir)
{
    mExeName = readCommFile(procDir);

    QFile file(procDir + "/environ");
    file.open(QFile::ReadOnly);
    QByteArray environData  = file.readAll();
    setEnv("BOOMAGA_PROC_FILE", file.fileName());

    QStringList extEnv;
    extEnv << ENV_VARS;

    foreach (QString name, extEnv)
    {
        setEnv(name, getValue(environData, name));
    }

    file.close();

}
#endif

/************************************************
 *
 ************************************************/
EnvInfo::EnvInfo(const EnvInfo &other)
{
    mExeName = other.mExeName;
    mData = other.mData;
}


/************************************************
 *
 ************************************************/
void EnvInfo::setEnv(const QString &key, const QString &value)
{
    mData.insert(key, value);
}


/************************************************
 *
 ************************************************/
EnvInfo EnvInfo::find(const QString &xDisplay)
{
    QStringList preferredAps;
    preferredAps << PREFERED_APPS;

    EnvInfo res;
    uid_t uid = getuid();
    foreach(QFileInfo dir, QDir("/proc").entryInfoList())
    {
        // Don't process directory
        bool ok;
        dir.fileName().toInt(&ok);
        if (!ok)
            continue;

        // Don't my process
        if (dir.ownerId() != uid)
            continue;

        EnvInfo envInfo(dir.filePath());


        if (envInfo.dbusAddr().isEmpty())
            continue;

        if (envInfo.xDisplay() == "")
            continue;

        if (!xDisplay.isEmpty() && envInfo.xDisplay() != xDisplay)
            continue;

        res = envInfo;

        if (preferredAps.contains(envInfo.exeName()))
            break;
    }

    return res;
}

/************************************************
 *
 ************************************************/
bool EnvInfo::save(const QString &fileName) const
{

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        return false;

    QHashIterator<QString, QString> it(mData);
    while (it.hasNext())
    {
        it.next();
        file.write(QString("%1=%2\n").arg(it.key(), it.value()).toLocal8Bit());
    }
    file.close();

    return true;
}
