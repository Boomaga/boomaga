/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2013 Boomaga team https://github.com/Boomaga
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


#include "kernel/project.h"
#include "gui/mainwindow.h"
#include "dbus.h"
#include "kernel/job.h"
#include "../common.h"

#include <QApplication>
#include <QTextStream>
#include <QLocale>
#include <QTranslator>
#include <QDBusConnection>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <iostream>
#include <cstdlib>


using namespace std;

typedef vector<string> stringList;

struct Args
{
    Args(int argc, char *argv[]);

    bool autoRemove;
    bool startedFromCups;
    stringList titles;
    stringList files;
};


/************************************************

 ************************************************/
void printHelp()
{
    QTextStream out(stdout);
    out << "Usage: boomaga [options] [files...]" << endl;
    out << endl;

    out << "Boomaga provides a virtual printer for CUPS. This can be used" << endl;
    out << "for print preview or for print booklets." << endl;
    out << endl;

    out << "Options:" << endl;
    //out << "  --debug               Keep output results from scripts" << endl;
    out << "  -h, --help              Show help about options" << endl;
    out << "  -t, --title <title>     The job name/title" << endl;
    out << "  -V, --version           Print program version" << endl;
    out << "      --autoremove        Automatically delete the input file" << endl;
    out << endl;

    out << "Arguments:" << endl;
    out << "  files                    One or more PDF files" << endl;


}


/************************************************

 ************************************************/
void printVersion()
{
    QTextStream out(stdout);
    out << "boomaga " << FULL_VERSION << endl;
}


/************************************************

 ************************************************/
int printError(const QString &msg)
{
    QTextStream out(stdout);
    out << msg << endl << endl;
    out << "Use --help to get a list of available command line options." << endl;
    return 1;
}


/************************************************
 *
 ************************************************/
Args::Args(int argc, char *argv[]):
    autoRemove(false),
    startedFromCups(false)
{
    for (int i = 1; i<argc; ++i)
    {
        const string arg = argv[i];

        //*************************************************
        if (arg == "-h" || arg == "--help")
        {
            printHelp();
            exit(EXIT_SUCCESS);
        }

        //*************************************************
        if (arg == "-V" || arg == "--version")
        {
            printVersion();
            exit(EXIT_SUCCESS);
        }

        //*************************************************
        if (arg == "-t" || arg == "--title")
        {
            if (i+1 < argc)
            {
                titles.push_back(argv[i+1]);
                i++;
                continue;
            }
            else
            {
                printError("'title' is missing.");
                exit(1);
            }
        }

        //*************************************************
        if (arg == "--autoremove")
        {
            autoRemove = true;
            continue;
        }

        //*************************************************
        if (arg == "--started-from-cups")
        {
            startedFromCups = true;
            continue;
        }

        //*************************************************
        files.push_back(argv[i]);
    }
}



/************************************************
 *
 ************************************************/
void readEnvFile()
{
    QString cacheDir = getenv("XDG_CACHE_HOME");
    if (cacheDir.isEmpty())
        cacheDir = QDir::homePath() + QLatin1String("/.cache");

    QFile envFile(cacheDir + "/boomaga.env");
    if (envFile.exists())
    {
        envFile.open(QFile::ReadOnly);
        while (!envFile.atEnd())
        {
            QString line = QString::fromLocal8Bit(envFile.readLine());
            QString name = line.section("=", 0, 0).trimmed();
            QString value = line.section("=", 1).trimmed();

            if ((value.startsWith('\'') && value.endsWith('\'')) ||
                (value.startsWith('"') && value.endsWith('"')))
            {
                value = value.mid(1, value.length() - 2);
            }

            qputenv(name.toLocal8Bit(), value.toLocal8Bit());
        }
        envFile.close();
    }

}


/************************************************

 ************************************************/
int main(int argc, char *argv[])
{
    Log::setPrefix("Boomaga GUI");

    Args args(argc, argv);

    // Start DBUS ...............................
    if (args.startedFromCups)
    {
        string title;
        auto it = args.titles.cbegin();
        for (auto file: args.files)
        {
            if (it != args.titles.cend())
            {
                title = *it;
                ++it;
            }
            else
                title.clear();

            Log::info("Start boomaga '%s' '%s' %s",
                      file.c_str(),
                      title.c_str(),
                      args.autoRemove ? "autoRemove" : "");
            BoomagaDbus::runBoomaga(QString::fromStdString(file), QString::fromStdString(title), args.autoRemove);
        }
        return 0;
    }

    // Start GUI ................................
    readEnvFile();

    QApplication application(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    application.installTranslator(&qtTranslator);


    QTranslator translator;
    translator.load(QString("%1/boomaga_%2.qm").arg(TRANSLATIONS_DIR, QLocale::system().name()));
    application.installTranslator(&translator);


    BoomagaDbus dbus("org.boomaga", "/boomaga");

    MainWindow mainWindow;
    mainWindow.show();
    application.processEvents();

    QStringList files;
    for (auto &f: args.files)
    {
        files << QString::fromStdString(f);
    }

    JobList jobs = project->load(files, "");
    for (int i=0; i<jobs.count(); ++i)
    {
        if (uint(i) < args.titles.size())
            jobs[i].setTitle(QString::fromStdString(args.titles[i]));

        jobs[i].setAutoRemove(args.autoRemove);
    }

    return application.exec();
}
