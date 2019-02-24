/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2019 Boomaga team https://github.com/Boomaga
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


#ifndef POSTSCRIPTFILE_H
#define POSTSCRIPTFILE_H

#include "infile.h"

class QFile;

class PostScriptFile : public InFile
{
    Q_OBJECT
public:
    explicit PostScriptFile(QObject *parent = 0);
    Type type() const override final { return Type::PostScript; }

protected:
    void read() override final;

private:
    void convertToPdf(QFile &psFile, const QString &pdfFile);
};

#endif // POSTSCRIPTFILE_H
