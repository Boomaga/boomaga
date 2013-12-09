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
#include <QProcessEnvironment>
#include <QFile>
#include <QDir>

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
    out << "  -n, --num <copies>      Sets the number of copies to print" << endl;
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
void readEnvFile()
{
    QFile envFile(QDir::homePath() + "/.cache/boomaga.env");
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
    readEnvFile();

    QApplication application(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    application.installTranslator(&qtTranslator);


    QTranslator translator;
    translator.load(QString("%1/boomaga_%2.qm").arg(TRANSLATIONS_DIR, QLocale::system().name()));
    application.installTranslator(&translator);


    QFileInfoList files;
    QString jobTitle;
    int copiesCount = 0;
    bool autoRemove = false;

    QStringList args = application.arguments();
    for (int i=1; i < args.count(); ++i)
    {
        QString arg = args.at(i);

        //*************************************************
        if (arg == "-h" || arg == "--help")
        {
            printHelp();
            return 0;
        }

        //*************************************************
        if (arg == "-V" || arg == "--version")
        {
            printVersion();
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
        if (arg == "--autoremove")
        {
            autoRemove = true;
            continue;
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
        files << QFileInfo(args.at(i));
    }

    QList<Job> jobs;
    foreach (const QFileInfo &file, files)
    {
        if (!file.filePath().isEmpty())
        {
            if (!file.exists())
                return printError(QString("Cannot open file \"%1\" (No such file or directory)")
                                  .arg(file.filePath()));

            if (!file.isReadable())
                return printError(QString("Cannot open file \"%1\" (Access denied)")
                                  .arg(file.filePath()));
        }
        jobs << Job(file.absoluteFilePath(), "", autoRemove);
    }

#if 0
    QFile f(QString("/tmp/boomaga-%1.env").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd-hh.mm.ss")));
    f.open(QFile::WriteOnly | QFile::Text);
    foreach(QString s, QProcessEnvironment::systemEnvironment().toStringList())
    {
        f.write(s.toLocal8Bit());
        f.write("\n");
    }
    f.close();
#endif

    BoomagaDbus dbus("org.boomaga", "/boomaga");

    MainWindow mainWindow;
    mainWindow.show();
    application.processEvents();


    if (!jobs.isEmpty())
        project->addFiles(jobs);


    return application.exec();
}
