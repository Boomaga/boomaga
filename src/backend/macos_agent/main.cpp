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

#include <iostream>
#include <glob.h>
#include <libgen.h>
#include <string>

#include <errno.h>
#include <glob.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

#include "../../common.h"

using namespace std;

static const char* PATTERN = "*.cboo." AUTOREMOVE_EXT;


/************************************************
 *
 ************************************************/
static void usage()
{
    cerr << "Usage: agent SOURCE_DIRECTORY" << endl;
    cerr << "  example: agent /var/spool/io.github.Boomaga " << endl;
}


/************************************************
 *
 ************************************************/
static string expandSrcPath(const string &path)
{
    string res = path;
    if (res[0] == '~')
    {
        const char *home = getenv("HOME");
        if (!home)
        {
            Log::error("can't get HOME environment variable");
            return nullptr;
        }

        res.replace(0, 1, home);
    }

    const char *user = getenv("USER");
    if (!user)
    {
        Log::error("can't get USER environment variable");
        return "";
    }

    return res + "/" + user;
}


/************************************************
 *
 ************************************************/
void startApplication(const char *appID, const string &file)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        throw string("can't fork: ") + strerror(errno);
    }

    if (pid)
    {
        Log::debug("forked, chaild PID is %d", pid);
        // wait for the child to exit
        int status;
        waitpid(pid, &status, 0);

        if (status != 0)
            Log::error("open command finished with %d exit code.", status);
        else
            Log::debug("open command successfully finished.");

        return;
    }

    else
    {
        Log::debug("exec open %s %s '%s' ",
              "-b", appID,
              file.c_str());

        execlp("open",
               "open",
               "-b", appID,
               file.c_str(),
               nullptr);
    }
}


/************************************************

 ************************************************/
int main(int argc, char *argv[])
{
    Log::setPrefix("Boomaga agent");
    Log::setWriteTime(true);

    if (argc < 2 || !strlen(argv[1]))
    {
        Log::error("Missing source directory operand");
        usage();
        return 1;
    }

    Log::debug("start '%s'", argv[1]);

    try
    {
        string srcDir  = expandSrcPath(argv[1]);

        Log::debug("Source dir: '%s'", srcDir.c_str());
        Log::debug("Search '%s'", (srcDir + "/" + PATTERN).c_str());

        int cnt = 100;
        while (cnt > 0) // If new files appear in the directory during this cycle, we process them again.
        {
            --cnt;

            glob_t files;
            int ret = glob((srcDir + "/" + PATTERN).c_str(), GLOB_TILDE, nullptr, &files);

            if (ret != 0 && ret != GLOB_NOMATCH)
            {
                Log::error("Glob error: %s", strerror(errno));
                return 500;
            }


            if (ret == GLOB_NOMATCH)
            {
                globfree(&files);
                Log::debug("No more files, exit");
                return 0;
            }


            for (size_t i=0; i<files.gl_pathc; ++i)
            {
                Log::debug("Found file '%s'", files.gl_pathv[i]);
                string file = files.gl_pathv[i];

                Log::debug("start application '%s' with file '%s'", MAC_APP_ID, file.c_str());
                startApplication(MAC_APP_ID, file);
            }

            globfree(&files);
            break;
        }

    }
    catch (string &e)
    {
        fflush(stdout);
        Log::error(e.c_str());
        return 500;
    }

    return CUPS_BACKEND_OK;
}
