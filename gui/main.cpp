/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2013 Alexander Sokoloff
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


#include "kernel/psproject.h"
#include "mainwindow.h"
#include "dbus.h"

#include <QApplication>
#include <QTextStream>
#include <QLocale>
#include <QTranslator>
#include <QDBusConnection>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QDebug>


void printHelp()
{
    QTextStream out(stdout);
    out << "Usage: booklet [options] file" << endl;
    out << endl;

    out << "Print poscript file as booklet" << endl;
    out << endl;

    out << "Options:" << endl;
    //out << "  --debug               Keep output results from scripts" << endl;
    out << "  -h, --help              Show help about options" << endl;
    out << "  -t, --title <title>     The job name/title" << endl;
    out << "  -n, --num <copies>      Sets the number of copies to print" << endl;
    out << endl;

    out << "Arguments:" << endl;
    out << "  file                    Postscript file" << endl;


}

int printError(const QString &msg)
{
    QTextStream out(stdout);
    out << msg << endl << endl;
    out << "Use --help to get a list of available command line options." << endl;
    return 1;
}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    application.installTranslator(&qtTranslator);


    QTranslator translator;
    translator.load(QString("%1/booklet_%2.qm").arg(TRANSLATIONS_DIR, QLocale::system().name()));
    application.installTranslator(&translator);


    QFileInfo file;
    QString jobTitle;
    int copiesCount=0;

    QStringList args = application.arguments();
    for (int i=1; i < args.count(); ++i)
    {
        QString arg = args.at(i);

        //*************************************************
        if (arg == "--help" || arg == "-h")
        {
            printHelp();
            return 0;
        }

        //*************************************************
        if (arg == "-t" || arg == "--title")
        {
            if (i+1 < args.count())
            {
                jobTitle = args.at(i+1);
                i++;
                continue;
            }
            else
            {
                return printError("'title' is missing.");
            }
        }

        //*************************************************
        if (arg == "-n" || arg == "--num")
        {
            if (i+1 < args.count())
            {
                bool ok;
                copiesCount = args.at(i+1).toInt(&ok);
                if (ok)
                {
                    i++;
                    continue;
                }
                else
                {
                    return printError("'number of copies' is invalid.");
                }
            }
            else
            {
                return printError(QString("expected copies after \"%1\" option.").arg(arg));
            }
        }

        //*************************************************
        file.setFile(args.at(i));
    }

    if (file.filePath().isEmpty())
        return printError("Postscript file missing.");

    if (!file.exists())
        return printError(QString("Cannot open file \"%1\" (No such file or directory)")
                          .arg(file.filePath()));

    if (!file.isReadable())
        return printError(QString("Cannot open file \"%1\" (Access denied)")
                          .arg(file.filePath()));

    // We try to open file in the another instanse (if it running).
    if (DBusProjectAdaptor::openFileInExisting(file.absoluteFilePath()))
        return 0;


    PsProject project;
    DBusProjectAdaptor dbus(&project);
    QDBusConnection::sessionBus().registerService("org.bprint");
    QDBusConnection::sessionBus().registerObject("/Project", &project);

    project.addFile(file.absoluteFilePath());

    MainWindow mainWindow(&project);
    mainWindow.show();
    return application.exec();
}
