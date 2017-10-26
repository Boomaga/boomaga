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


#include "common.h"
#include <QTextStream>


/************************************************
 *
 ************************************************/
void print(const QString &prefix, const QString &message)
{
    QString msg = message.trimmed();
    msg.replace('\n', "\n" + prefix + " [Boomaga] ");

    QTextStream out(stderr);
    out << prefix + " [Boomaga] " << msg << endl;
}



/************************************************
 *
 ************************************************/
void debug(const QString &message)
{
    print("DEBUG:", message);
}


/************************************************
 *
 ************************************************/
void warning(const QString &message)
{
    print("WARNING:", message);
}


/************************************************
 *
 ************************************************/
void info(const QString &message)
{
    print("INFO:", message);
}


/************************************************
 *
 ************************************************/
void error(const QString &message)
{
    print("ERROR:", message);
    exit(CUPS_BACKEND_FAILED);
}


#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
/************************************************
 *
 ************************************************/
void messageOutput(QtMsgType type, const char *message)
{
    switch (type) {
    case QtDebugMsg:
        debug(QString::fromLocal8Bit(message));
        break;

    case QtWarningMsg:
        warning(QString::fromLocal8Bit(message));
        break;

    case QtCriticalMsg:
        error(QString::fromLocal8Bit(message));
        break;

    case QtFatalMsg:
        error(QString::fromLocal8Bit(message));
        break;
    }
}

#else
/************************************************
 *
 ************************************************/
void messageOutput(QtMsgType type, const QMessageLogContext&, const QString &message)
{
    switch (type) {
    case QtDebugMsg:
        debug(message);
        break;

    case QtWarningMsg:
        warning(message);
        break;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    case QtInfoMsg:
        info(message);
        break;
#endif

    case QtCriticalMsg:
        error(message);
        break;

    case QtFatalMsg:
        error(message);
        break;
    }
}
#endif
