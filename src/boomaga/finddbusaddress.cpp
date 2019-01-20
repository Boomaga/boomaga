/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2018 Boomaga team https://github.com/Boomaga
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


#include "finddbusaddress.h"
#include "../common.h"

#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <unordered_set>
#include <unistd.h>
#include <assert.h>
#include <QDir>
#include <QProcess>


using namespace std;


/************************************************
 * Extract DBUS_SESSION_BUS_ADDRESS variable from file
 ************************************************/
static string readFile(const string &fileName, char delim)
{
    ifstream file(fileName, ios::binary);
    string line;
    while (getline(file, line, delim))
    {
        if (line.find("DBUS_SESSION_BUS_ADDRESS=") != 0)
            continue;

        static int s = strlen("DBUS_SESSION_BUS_ADDRESS=");
        s = line.find_first_not_of(" \t'\"", s);

        int e = line.find_last_not_of(" \t'\"") + 1;
        return line.substr(s, e-s);
    }

    return "";
}


/************************************************
 * Looking for DBUS addresses in files in the
 * directory ~/.dbus/session-bus
 ************************************************/
QStringList FindDbusAddress::fromSessionFiles()
{
    string sessionsDir = QDir::homePath().toStdString() + "/.dbus/session-bus";

    struct FileInfo {
        string name;
        long   ctime;
    };


    // Scan ~/.dbus/session-bus
    vector<FileInfo> files;
    DIR *dir;
    if ((dir = opendir(sessionsDir.c_str())) == NULL)
    {
        Log::warn("Can't read DBUS session direcory %s", sessionsDir.c_str());
        return QStringList();
    }

    dirent *ent;
    while ((ent = readdir(dir)) != NULL)
    {
        if (!strcmp(ent->d_name, ".") ||!strcmp(ent->d_name, ".."))
            continue;

        string f = sessionsDir + "/" + ent->d_name;

        struct stat st;
        stat(f.c_str(), &st);

        files.push_back(FileInfo{f, st.st_ctime});
    }
    closedir (dir);


    // Latest files are more priority, so sort by creation time
    std::sort(files.begin(), files.end(), [](const FileInfo &a, const FileInfo &b) -> bool {
       return a.ctime > b.ctime;
    });


    QStringList res;
    for (const auto & p : files)
    {
        string s = readFile(p.name, '\n');
        if (!s.empty())
            res << QString::fromStdString(s);
        else
            Log::warn("Don't found DBUS address in %s", p.name.c_str());
    }

    return res;
}


#ifdef Q_OS_LINUX
/************************************************
 *
 ************************************************/
QStringList FindDbusAddress::fromProcFiles()
{
    static const string PROC_DIR = "/proc/";

    DIR *dir;
    if ((dir = opendir(PROC_DIR.c_str())) == NULL)
    {
        Log::warn("Can't read /proc direcory");
        return QStringList();
    }

    uid_t uid = getuid();
    unordered_set<string> res;
    dirent *ent;
    struct stat fstat;
    while ((ent = readdir(dir)) != NULL)
    {
        if (!isdigit(ent->d_name[0]))
            continue;

        string dir(PROC_DIR + ent->d_name);
        if (stat(dir.c_str(), &fstat) != 0 || fstat.st_uid != uid)
            continue;

        string addr = readFile(dir + "/environ", '\0');
        if (!addr.empty())
            res.insert(addr);
    }
    closedir (dir);

    QStringList ret;
    for (const auto &s: res)
    {
        ret << QString::fromStdString(s);
    }
    return ret;
}
#endif


#ifdef Q_OS_FREEBSD
/************************************************
 *
 ************************************************/
static QString getValue(const QString &data, const QString &key)
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
QStringList FindDbusAddress::fromProcStat()
{
    QProcess proc;
    proc.setProcessChannelMode(QProcess::SeparateChannels);
    proc.start("procstat -ash");
    proc.waitForFinished();

    if (proc.exitStatus() != 0 || proc.exitCode() != 0)
    {
        QString msg = QString::fromLocal8Bit(proc.readAllStandardError());
        Log::warn("'procstat -ash' exit with code %d: %s", proc.exitCode(), msg.toLocal8Bit().data());
        return QStringList();
    }

    QString uid = QString::number(getuid());

    QHash<QString, bool> pids;
    proc.setReadChannel(QProcess::StandardOutput);
    while (!proc.atEnd())
    {
        QString line = QString::fromLocal8Bit(proc.readLine().simplified());
        QString p = line.section(' ', 0, 0);
        QString u = line.section(' ', 2, 2);
        if (u == uid)
            pids.insert(p, true);
    }


    proc.setProcessChannelMode(QProcess::SeparateChannels);
    proc.start("procstat -aeh");
    proc.waitForFinished();

    if (proc.exitStatus() != 0 || proc.exitCode() != 0)
    {
        QString msg = QString::fromLocal8Bit(proc.readAllStandardError());
        Log::warn("'procstat -aeh' exit with code %d: %s", proc.exitCode(), msg.toLocal8Bit().data());
        return QStringList();
    }

    QStringList res;
    proc.setReadChannel(QProcess::StandardOutput);
    while (!proc.atEnd())
    {
        QString line = QString::fromLocal8Bit(proc.readLine().simplified());
        if (!pids.contains(line.section(' ', 0, 0)))
            continue;

        QString env = line.section(' ', 2);
        QString addr = getValue(env, "DBUS_SESSION_BUS_ADDRESS");
        if (!addr.isEmpty())
            res << addr;
    }

    return res;
}
#endif
