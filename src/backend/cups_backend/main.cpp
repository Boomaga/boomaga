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


#include "../../common.h"

#include <iostream>
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <fstream>
#include <string>


using namespace std;

struct Args
{
    string jobID;
    string user;
    string title;
    string options;
    string file;
    passwd *pwd;
    int    count;
};


/************************************************
 *
 ************************************************/
static int getmod(const string &file)
{
    struct stat fstat;
    if (stat(file.c_str(), &fstat) == 0)
    {
        return fstat.st_mode & 0xFFF;
    }

    return 0;
}


/************************************************
 *
 ************************************************/
static string mkUserDir(const string &baseDir, const string &user)
{
    Log::debug("Create user direcory %s", baseDir.c_str());

    passwd *pwd = getpwnam(user.c_str());
    if (!pwd)
    {
        Log::error("Can't get uid for user %s", user.c_str());
        return "";
    }


    // Base .....................................
    if (mkdir(baseDir.c_str(), 0775) != 0)
    {
        if (errno != EEXIST)
        {
            Log::error("Can't create directory %s: %s", baseDir.c_str(), std::strerror(errno));
            return "";
        }
    }


    if (getmod(baseDir) != 0775 && chmod(baseDir.c_str(), 0775) != 0)
    {
        Log::error("Can't change mode on directory %s: %s", baseDir.c_str(), std::strerror(errno));
        return "";
    }

    // User .....................................
    string dir = baseDir + "/" + user;
    if (mkdir(dir.c_str(), 0770) != 0)
    {
        if (errno != EEXIST)
        {
            Log::error("Can't create directory %s: %s", dir.c_str(), std::strerror(errno));
            return "";
        }
    }

    if (getmod(dir) != 0770 && chmod(dir.c_str(), 0770) != 0)
    {
        Log::error("Can't change mode on directory %s: %s", dir.c_str(), std::strerror(errno));
        return "";
    }

    if (chown(dir.c_str(), pwd->pw_uid, -1) != 0)
    {
        Log::error("Can't change owner on directory %s: %s", dir.c_str(), std::strerror(errno));
        return "";
    }

    Log::debug("User directory %s saccefully created", dir.c_str());
    return dir;
}


/************************************************
 *
 ************************************************/
static bool createJobFile(istream &src, const string &destFile, const Args &args)
{
    Log::debug("Create job file %s", destFile.c_str());

    ofstream dest(destFile, ios::binary | ios::trunc);
    dest << "\033CUPS_BOOMAGA\n";
    dest << "JOB="     << escapeString(args.jobID)   << "\n";
    dest << "USER="    << escapeString(args.user)    << "\n";
    dest << "TITLE="   << escapeString(args.title)   << "\n";
    dest << "COUNT="   << args.count                 << "\n";
    dest << "OPTIONS=" << escapeString(args.options) << "\n";
    dest << "CUPS_BOOMAGA_DATA\n";
    dest << src.rdbuf();
    dest.close();

    if (src.bad() || !dest.good())
    {
        Log::debug("Delete file %s", destFile.c_str());
        unlink(destFile.c_str());
        Log::error("Can't create job file: %s", strerror(errno));
        return false;
    }

    if (chown(destFile.c_str(), args.pwd->pw_uid, -1) != 0)
    {
        Log::error("Can't change owner on directory %s: %s", destFile.c_str(), std::strerror(errno));
        return false;
    }

    return true;
}


/************************************************
 *
 ************************************************/
string dirname(const string &path)
{
    string::size_type n = path.rfind('/', path.length() - 2);
    if (n != string::npos)
        return path.substr(0, n);

    return path;
}


/************************************************
 * http://www.cups.org/documentation.php/doc-1.6/api-filter.html
 ************************************************/
int main(int argc, char *argv[])
{
    Log::setPrefix("Boomaga backend");
    if (argc == 1)
    {
        // Output "device discovery" information on stdout:
        // http://www.cups.org/documentation.php/doc-1.6/man-backend.html
        // device-class device-uri "device-make-and-model" "device-info" "device-id"

        cout << "file "
                  << CUPS_BACKEND_URI << " "
                  << "\"" CUPS_BACKEND_MODEL "\" "
                  << "\"" CUPS_BACKEND_INFO  "\" "
                  << "\"" "MFG:" CUPS_BACKEND_MANUFACTURER ";CMD:PJL,PDF;MDL:"
                  << CUPS_BACKEND_MODEL ";CLS:PRINTER;DES:" CUPS_BACKEND_DESCRIPTION ";DRV:DPDF,R1,M0;" "\""
                  << endl;
        return CUPS_BACKEND_OK;
    }

    if (argc < 6)
    {
        cerr << "Usage: boomaga job-id user title copies options [file]" << endl;
        return CUPS_BACKEND_FAILED;
    }


    Args args;
    args.jobID   = argv[1];
    args.user    = argv[2];
    args.title   = argv[3];
    args.count   = max(1, atoi(argv[4]));
    args.options = argv[5];
    args.file    = (argc > 6) ? argv[6] : "";


    args.pwd = getpwnam(args.user.c_str());
    if (!args.pwd)
    {
        Log::error("Can't get uid for user %s.", args.user.c_str());
        return CUPS_BACKEND_FAILED;
    }

#ifdef __APPLE__
    const string baseDir = MAC_SPOOL_DIR;
#else
    char *cupsCacheDir = getenv("CUPS_CACHEDIR");
    const string baseDir = cupsCacheDir ?
                dirname(cupsCacheDir) + "/boomaga" :
                "/var/cache/boomaga";
#endif

    string dir = mkUserDir(baseDir, args.user);
    if (dir.empty())
        return CUPS_BACKEND_FAILED;

    string booFile = dir + "/in_" + args.jobID + ".cboo." + AUTOREMOVE_EXT;

    if (argc > 6)
    {
        ifstream src(argv[6]);
        if (!src.is_open())
            Log::fatalError("Can't write job file %s: %s", argv[6], strerror(errno));

        if (!createJobFile(src, booFile, args))
            return CUPS_BACKEND_FAILED;
    }
    else
    {
        if (!createJobFile(std::cin, booFile, args))
            return CUPS_BACKEND_FAILED;
    }

#ifdef __APPLE__
    // Start agent from Boomaga.app

    string startFile = baseDir + "/.start";
    ofstream dest(startFile, ios::binary | ios::trunc);
    dest << time(nullptr);
    dest.close();

    if (getmod(startFile) != 0664 && chmod(startFile.c_str(), 0664) != 0)
    {
        Log::error("Can't change mode on file %s: %s", startFile.c_str(), std::strerror(errno));
        return CUPS_BACKEND_FAILED;
    }

    return CUPS_BACKEND_OK;
#else
    if (setgid(args.pwd->pw_gid) != 0)
        Log::fatalError("Can't change GID to %d: %s.", args.pwd->pw_gid, strerror(errno));

    if (setuid(args.pwd->pw_uid) != 0)
        Log::fatalError("Can't change UID to %d: %s.", args.pwd->pw_uid, strerror(errno));


    string path = GUI_DIR;
    char *envPath = getenv("PATH");
    if (envPath != nullptr)
          path.append(":").append(envPath);
    setenv("PATH", path.c_str(), 1);

    execlp("boomaga",
           "boomaga",
           "--started-from-cups",
           booFile.c_str(),
           NULL);

    Log::error("run boomaga GUI error: %s", strerror(errno));
    return CUPS_BACKEND_FAILED;
#endif

}
