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


#include "logging.h"
#include "dbussessionbus.h"
#include "dbuslegacy.h"
#include "../pstream.h"

#include <iostream>
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <fstream>
#include <string>


static const std::string CACHE_DIR  = "boomaga";
static const int CUPS_BACKEND_OK      = 0;
static const int CUPS_BACKEND_FAILED  = 1;
const static int BUF_SIZE  = 4096;

using namespace std;

struct Args
{
    string jobID;
    string user;
    string title;
    int    count;
    string options;
    string file;
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
static string mkUserDir(const char *baseDir, const string &user)
{
    string dir(baseDir);

    int n = dir.rfind('/', dir.length() - 2);
    if (n>-1)
        dir.resize(n);

    dir.append("/" + CACHE_DIR);



    debug("Create user direcory '%s'", dir.c_str());

    passwd *pwd = getpwnam(user.c_str());
    if (!pwd)
    {
        error("Can't get uid for user %s", user.c_str());
        return "";
    }


    // Base .....................................
    if (mkdir(dir.c_str(), 0775) != 0)
    {
        if (errno != EEXIST)
        {
            error("Can't create directory %s: %s", dir.c_str(), std::strerror(errno));
            return "";
        }
    }


    if (getmod(dir) != 0775 && chmod(dir.c_str(), 0775) != 0)
    {
        error("Can't change mode on directory %s: %s", dir.c_str(), std::strerror(errno));
        return "";
    }

    // User .....................................
    dir.append("/" + user);
    if (mkdir(dir.c_str(), 0770) != 0)
    {
        if (errno != EEXIST)
        {
            error("Can't create directory %s: %s", dir.c_str(), std::strerror(errno));
            return "";
        }
    }

    if (getmod(dir) != 0770 && chmod(dir.c_str(), 0770) != 0)
    {
        error("Can't change mode on directory %s: %s", dir.c_str(), std::strerror(errno));
        return "";
    }

    if (chown(dir.c_str(), pwd->pw_uid, -1) != 0)
    {
        error("Can't change owner on directory %s: %s", dir.c_str(), std::strerror(errno));
        return "";
    }

    debug("User directory %s saccefully created", dir.c_str());
    return dir;
}


/************************************************
 *
 ************************************************/
static bool createBooFile(istream &src, const string &destFile, const Args args)
{
    debug("Create job file %s", destFile.c_str());
    string header;
    getline(src, header);


    ofstream dest(destFile, ios::binary | ios::trunc);

    dest << "\x1B%-12345X@PJL BOOMAGA_PROGECT\n";
    dest << "@PJL BOOMAGA META_TITLE=" << args.title << "\n";
    dest << "@PJL BOOMAGA JOB_TITLE=" << args.title << "\n";
    // @PJL SET COPIES = 4

    if (header.compare(0, 5, "%PDF-") == 0)
    {
        dest << "@PJL ENTER LANGUAGE=PDF\n";
    }
    else if (header.compare(0, 5, "%!PS-Adobe-") == 0)
    {
        dest << "@PJL ENTER LANGUAGE=POSTSCRIPT\n";
    }

    bool lf = true;
    dest << header << endl;
    char buf[BUF_SIZE];
    do {
        src.read(&buf[0], BUF_SIZE);
        if (src.gcount())
            lf = buf[src.gcount() - 1] == '\n';
        dest.write(&buf[0], src.gcount());
    } while (src.gcount() > 0);

    if (!lf)
        dest << "\n";

    dest << "\x1B%-12345X@PJL\n";
    dest << "@PJL EOJ\n";
    dest << "\x1B%-12345X";
    dest.close();

    if (src.bad() || !dest.good())
    {
        debug("Delete file %s", destFile.c_str());
        unlink(destFile.c_str());
        error("Can't create job file: %s", strerror(errno));
        return false;
    }

    return true;
}


/************************************************
 *
 ************************************************/
static bool dbusSend(const string& dbusAddress, const string &file, const Args &args)
{
    vector<string> argv;
    argv.push_back("dbus-send");
    argv.push_back("--session");
    argv.push_back("--type=method_call");
    argv.push_back("--print-reply");
    argv.push_back("--dest=org.boomaga");
    argv.push_back("/boomaga");
    argv.push_back("org.boomaga.add");
    argv.push_back("string:" + file);
    argv.push_back("string:" + args.title);
    argv.push_back("boolean:true");
    argv.push_back("string:" + args.options);
    argv.push_back("uint32:" + std::to_string(args.count));


    setenv("DBUS_SESSION_BUS_ADDRESS", dbusAddress.c_str(), 1);
    redi::ipstream proc;

    proc.open(argv[0], argv, redi::pstream::pstdout | redi::pstream::pstderr);
    string line;

    while (getline(proc.err(), line))
    {
        warn("dbus-send (%s) error: %s", dbusAddress.c_str(),  line.c_str());
    }
    proc.rdbuf()->close();

    if (proc.rdbuf()->status() != 0)
    {
        warn("dbus-send (%s) exit: %d", dbusAddress.c_str(), proc.rdbuf()->status());
        return false;
    }

    debug("dbus-send (%s) successfully finished.", dbusAddress.c_str());
    return true;
}


/************************************************
 * http://www.cups.org/documentation.php/doc-1.6/api-filter.html
 ************************************************/
int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        // Output "device discovery" information on stdout:
        // http://www.cups.org/documentation.php/doc-1.6/man-backend.html
        // device-class device-uri "device-make-and-model" "device-info" "device-id"

        cout << "file "
                  << CUPS_BACKEND_URI << " "
                  << "\"" CUPS_BACKEND_MODEL "\" "
                  << "\"" CUPS_BACKEND_INFO  "\" "
                  << "\"" "MFG:" CUPS_BACKEND_MANUFACTURER ";CMD:PJL,PDF;MDL:" CUPS_BACKEND_MODEL ";CLS:PRINTER;DES:" CUPS_BACKEND_DESCRIPTION ";DRV:DPDF,R1,M0;" "\""
                  << endl;
        return CUPS_BACKEND_OK;
    }

    if (argc < 6)
    {
        cout << "Usage: boomaga job-id user title copies options [file]" << endl;
        return CUPS_BACKEND_FAILED;
    }

    Args args;
    args.jobID   = argv[1];
    args.user    = argv[2];
    args.title   = argv[3];
    args.count   = max(1, atoi(argv[4]));
    args.options = argv[5];
    args.file    = (argc > 6) ? argv[6] : "";

    char *cupsCacheDir = getenv("CUPS_CACHEDIR");
    string dir = mkUserDir(cupsCacheDir ? cupsCacheDir : "/var/cache/cups", args.user);
    if (dir.empty())
        return CUPS_BACKEND_FAILED;

    string booFile = dir + "/in_" + args.jobID + ".boo";

    bool ok;
    if (argc > 6)
    {

        ifstream src(argv[6]);
        if (!src.is_open())
        {
            fatalError("Can't write job file %s: %s", argv[6], strerror(errno));
            return CUPS_BACKEND_FAILED;
        }
        ok = createBooFile(src, booFile, args);
        src.close();
    }
    else
    {
        ok = createBooFile(std::cin, booFile, args);
    }

    if (!ok)
        return CUPS_BACKEND_FAILED;

    passwd *pwd = getpwnam(args.user.c_str());
    if (!pwd)
    {
        error("Can't get uid for user %s.", args.user.c_str());
        return CUPS_BACKEND_FAILED;
    }


    if (setgid(pwd->pw_gid) != 0)
    {
        error("Can't change GID to %d: %s.", pwd->pw_gid, strerror(errno));
        return CUPS_BACKEND_FAILED;
    }


    if (setuid(pwd->pw_uid) != 0)
    {
        error("Can't change UID to %d: %s.", pwd->pw_uid, strerror(errno));
        return CUPS_BACKEND_FAILED;
    }


    vector<string> dbusAddresses = DBusSessionBus::findDbusAddress(pwd->pw_dir);

    for (const string& addr: dbusAddresses)
    {
        bool ok = dbusSend(addr, booFile, args);
        if (ok)
            return CUPS_BACKEND_OK;
    }


    // Legacy method .................................
    dbusAddresses = DBusLegacy::findDbusAddress(pwd->pw_uid);

    for (const string& addr: dbusAddresses)
    {
       bool ok = dbusSend(addr, booFile, args);
       if (ok)
           return CUPS_BACKEND_OK;
    }

    error("Can't start boomaga GUI.");
    return CUPS_BACKEND_FAILED;
}
