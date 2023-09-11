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
#ifdef MAC_UPDATER
#include "updater/updater.h"
#endif

using namespace std;

typedef vector<string> stringList;

struct Args
{
    Args(int argc, char *argv[]);

    bool startedFromCups;
    stringList files;
};


/************************************************

 ************************************************/
void printHelp()
{
    QTextStream out(stdout);
    out << "Usage: boomaga [options] [files...]" << Qt::endl;
    out <<Qt::endl;

    out << "Boomaga provides a virtual printer for CUPS. This can be used" <<Qt::endl;
    out << "for print preview or for print booklets." <<Qt::endl;
    out <<Qt::endl;

    out << "Options:" <<Qt::endl;
    out << "  -h, --help              Show help about options" <<Qt::endl;
    out << "  -V, --version           Print program version" <<Qt::endl;
    out <<Qt::endl;

    out << "Arguments:" <<Qt::endl;
    out << "  files                    One or more PDF files" <<Qt::endl;


}


/************************************************

 ************************************************/
void printVersion()
{
    QTextStream out(stdout);
    out << "boomaga " << FULL_VERSION <<Qt::endl;
}


/************************************************

 ************************************************/
int printError(const QString &msg)
{
    QTextStream out(stdout);
    out << msg << Qt::endl;
    out << "Use --help to get a list of available command line options." << Qt::endl;
    return 1;
}


/************************************************
 *
 ************************************************/
Args::Args(int argc, char *argv[]):
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
 *
 ************************************************/
void cleanup()
{
    // Remove temporary files ..............
    QDir dir(boomagaChacheDir());
    QStringList filters;
    filters << (appUUID() + "*");
    QStringList files = dir.entryList(filters, QDir::NoDotAndDotDot|QDir::Files);
    foreach (QString f, files)
    {
        dir.remove(f);
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
        for (auto file: args.files)
        {
            Log::info("Start boomaga '%s'", file.c_str());
            BoomagaDbus::runBoomaga(QString::fromStdString(file));
        }
        return 0;
    }

    // Start GUI ................................
    readEnvFile();

    QApplication application(argc, argv);
    QObject::connect(&application, &QCoreApplication::aboutToQuit,
                     &cleanup);


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

#ifdef MAC_UPDATER
    Updater &updater = Updater::sharedUpdater();
    if (updater.automaticallyChecksForUpdates())
        updater.checkForUpdatesInBackground();
#endif

    QStringList files;
    for (auto &f: args.files)
    {
        files << QString::fromStdString(f);
    }

    project->load(files);
    return application.exec();
}
