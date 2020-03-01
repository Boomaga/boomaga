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
#include "application.h"

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

#define LOG_TO_FILE
#ifdef LOG_TO_FILE
QFile *openLog() {
    QFile *res = new QFile(QDir::homePath() + "/boomaga.log");
    res->open(QFile::WriteOnly);
    return res;
}


void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)
    static QFile *log = openLog();
    log->seek(log->size());

    QString s = QString("%1: %2")
            .arg(QDateTime::currentDateTime().toString("MM.dd hh:mm:ss"))
            .arg(msg);


    switch (type) {
    case QtDebugMsg:
        log->write((QString("Debug: ") + s).toLocal8Bit());
        break;
    case QtInfoMsg:
        log->write((QString("Info: ") + s).toLocal8Bit());
        break;
    case QtWarningMsg:
        log->write((QString("Warning: ") + s).toLocal8Bit());
        break;
    case QtCriticalMsg:
        log->write((QString("Critical: ") + s).toLocal8Bit());
        break;
    case QtFatalMsg:
        log->write((QString("Fatal: ") + s).toLocal8Bit());
        break;
    }
    log->write("\n");
    log->flush();
}

typedef vector<string> stringList;

struct Args
{
    Args(int argc, char *argv[]);

    bool startedFromCups;
    stringList files;
};
#endif

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
    out << "  -h, --help              Show help about options" << endl;
    out << "  -V, --version           Print program version" << endl;
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
 *
 ************************************************/
static QString moveAutoRemoveFile(const QString oldFile)
{
    static quint64 num = 0;
    ++num;
    QString newFile = QString("%1%2%3_in[%4].cboo")
            .arg(boomagaChacheDir())
            .arg(QDir::separator())
            .arg(appUUID())
            .arg(num);

    Log::debug("Move \"%s\" to \"%s\"",
               oldFile.toLocal8Bit().data(),
               newFile.toLocal8Bit().data());

    QFile f(oldFile);
    if (! f.rename(newFile)) {
        Log::error("Can't move \"%s\" to \"%s\": %s",
                   oldFile.toLocal8Bit().data(),
                   newFile.toLocal8Bit().data(),
                   f.errorString().toLocal8Bit().data());
        return "";
    }

    return newFile;
}


/************************************************
 *
 ************************************************/
static void openFiles(const QStringList &files)
{
    QStringList jobFiles;
    for (auto file: files) {
        if (file.endsWith(AUTOREMOVE_EXT))
        {
            QString newFile = moveAutoRemoveFile(file);
            if (!newFile.isEmpty())
                jobFiles << newFile;
        }
        else
        {
            jobFiles << file;
        }
    }
    project->load(jobFiles);
}


/************************************************
 *
 ************************************************/
static void openFile(const QString &file)
{
    project->load(QStringList() << file);
}


/************************************************

 ************************************************/
int main(int argc, char *argv[])
{
#ifdef LOG_TO_FILE
    qInstallMessageHandler(myMessageOutput);
    qDebug() << ".:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:";
#endif
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

    Application application(argc, argv);
    QObject::connect(&application, &QCoreApplication::aboutToQuit, &cleanup);
    Application::connect(&application, &Application::openFile, openFile);


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

    openFiles(files);
    return application.exec();
}
