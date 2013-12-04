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


#include "../kernel/project.h"
#include "pdfmerger.h"
#include <QTextStream>
#include <QDebug>
#include <QCoreApplication>
#include <iostream>

/************************************************

 ************************************************/
int main(int argc, char *argv[])
{
    if (argc < 3)
        return 1;

    PdfMerger merger;
    QCoreApplication app(argc, argv);

    QStringList args = app.arguments();
    for (int i=1; i<args.count() - 1; ++i)
    {
        merger.addFile(args.at(i));
    }

    merger.run(argv[argc-1]);

    return 0;
}
