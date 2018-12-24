/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2018 Boomaga team https://github.com/Boomaga
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


#include "dbussessionbus.h"
#include "logging.h"
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>

#include <iostream>
#include <algorithm>
#include <fstream>

using namespace std;


/************************************************
 * Read session-bus file
 ************************************************/
static string readFile(const string &fileName)
{
    debug("Read DBUS address from %s", fileName.c_str());
    ifstream file(fileName, ios::binary);
    string line;
    while (getline(file, line))
    {
        if (line.find("DBUS_SESSION_BUS_ADDRESS=") != 0)
            continue;

        int s = strlen("DBUS_SESSION_BUS_ADDRESS=");
        s = line.find_first_not_of(" \t'\"", s);

        int e = line.find_last_not_of(" \t'\"") + 1;
        debug("Found DBUS address \"%s\"", line.substr(s, e-s).c_str());
        return line.substr(s, e-s);
    }

    warn("Don't found DBUS address in %s", fileName.c_str());
    return "";
}


/************************************************
 * Looking for DBUS addresses in files in the
 * directory ~/.dbus/session-bus
 ************************************************/
vector<string> DBusSessionBus::findDbusAddress(const string &home)
{
    string sessionsDir = home + "/.dbus/session-bus";

    vector<string> res;

    struct FileInfo {
        string name;
        long   ctime;
    };


    // Scan ~/.dbus/session-bus
    vector<FileInfo> files;
    DIR *dir;
    if ((dir = opendir(sessionsDir.c_str())) == NULL)
    {
        warn("Can't read DBUS session direcory %s", sessionsDir.c_str());
        return res;
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

    for (const auto & p : files)
    {
        string s = readFile(p.name);
        if (!s.empty())
            res.push_back(s);
    }

    return res;
}
