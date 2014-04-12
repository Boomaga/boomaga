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


#ifndef BOOMAGAPOPPLER_H
#define BOOMAGAPOPPLER_H

#include <QString>
#include <poppler/PDFDoc.h>
class QString;

class BoomagaPDFDoc: public PDFDoc
{
public:
    BoomagaPDFDoc(const QString &fileName, qint64 startPos, qint64 endPos);

    bool isValid() const { return mValid; }
    QString errorString() const { return mErrorString; }

    QString getMetaInfo(const char *tag);

private:
    FILE *f;
    bool mValid;
    QString mErrorString;
};

QString getPDFDocMetaInfo(PDFDoc *doc, const char *tag);





#endif // BOOMAGAPOPPLER_H
