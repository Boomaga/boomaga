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


#include "dbuslegacy.h"
#include "logging.h"

#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <cstring>
#include <unordered_set>

using namespace std;


#ifdef __linux__

static const string PROC_DIR = "/proc/";

/************************************************
 *
 ************************************************/
static string readEnvironFile(const string &fileName)
{
    ifstream strm(fileName, ios::binary);

    string line;

    while (getline(strm, line, '\0'))
    {
        if (line.find("DBUS_SESSION_BUS_ADDRESS=") != 0)
            continue;

        int s = strlen("DBUS_SESSION_BUS_ADDRESS=");
        s = line.find_first_not_of(" \t'\"", s);

        int e = line.find_last_not_of(" \t'\"") + 1;
        //debug("Found DBUS address \"%s\"", line.substr(s, e-s).c_str());
        return line.substr(s, e-s);
    }
    return "";
}


/************************************************
 * Looking for DBUS addresses in /proc/ * /environ files
 ************************************************/
vector<string> DBusLegacy::findDbusAddress(__uid_t uid)
{
    DIR *dir;
    if ((dir = opendir("/proc")) == NULL)
    {
        warn("Can't read /proc direcory");
        return vector<string>();
    }

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

        string addr = readEnvironFile(dir + "/environ");
        if (!addr.empty())
            res.insert(addr);
    }
    closedir (dir);


    vector<string> ret;
    ret.assign(res.begin(), res.end());
    return ret;
}
#endif
