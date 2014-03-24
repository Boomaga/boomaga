/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2014 Boomaga team https://github.com/Boomaga
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


#ifndef EXPORTTOPDFPRINTER_H
#define EXPORTTOPDFPRINTER_H

#include "../kernel/printer.h"

class ExportToPDFPrinter: public Printer
{
public:
    ExportToPDFPrinter();
    virtual QString printerName() const;
    bool print(const QList<Sheet*> &sheets, const QString &jobName, bool duplex, int numCopies = 1) const;
private:
    mutable QString mOutFileName ;
};

#endif // EXPORTTOPDFPRINTER_H
