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


#ifndef COMMON_H
#define COMMON_H

#include <QString>

void debug(const QString &message);
void info(const QString &message);
void warning(const QString &message);
void error(const QString &message);

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
void messageOutput(QtMsgType type, const char *message);
#else
void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &message);
#endif

#define CUPS_BACKEND_OK 0
#define CUPS_BACKEND_FAILED 1

#endif // COMMON_H
