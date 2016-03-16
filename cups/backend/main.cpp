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

#include <iostream>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


using namespace std;

#define CUPS_BACKEND_OK 0
#define CUPS_BACKEND_FAILED 1

/************************************************
 *
 ************************************************/
int main(int argc, char *argv[])
{
    if (argc < 6)
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
        return 0;
    }


    char *jobID   = argv[1];
    char *user    = argv[2];
    char *title   = argv[3];
    char *count   = argv[4];
    char *options = argv[5];

    passwd *pwd = getpwnam(user);
    if (!pwd)
    {
        cout << "ERROR: [Boomaga root] Can't get uid for user " << user << endl;
        return CUPS_BACKEND_FAILED;
    }

    cout << "DEBUG: [Boomaga root] run boomagabackend as UID:" << pwd->pw_uid << " GID: " << pwd->pw_gid << endl;

    setenv("HOME", pwd->pw_dir, 1);
    if (setgid(pwd->pw_gid) != 0)
    {
        cout << "ERROR: [Boomaga root] Can't change GID to " << pwd->pw_gid << endl;
        return CUPS_BACKEND_FAILED;
    }


    if (setuid(pwd->pw_uid) != 0)
    {
        cout << "ERROR: [Boomaga root] Can't change uid to " << pwd->pw_uid << endl;
        return CUPS_BACKEND_FAILED;
    }


    if (argc > 6)
    {
        // Read PDF from file
        stdin = freopen(argv[6], "r", stdin);
    }

    execl(NONGUI_DIR "/boomagabackend", NONGUI_DIR "/boomagabackend", jobID, title, count, options, NULL);
    perror("ERROR: [Boomaga root] run boomagabackend error");
    return CUPS_BACKEND_FAILED;
}
