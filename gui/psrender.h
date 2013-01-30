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


#ifndef PSRENDER_H
#define PSRENDER_H

#include <QThread>
#include <QTextStream>
#include <QStringList>
#include <QImage>

class PsProject;
class PsSheet;
class PsRenderPrivate;

class PsRender : public QObject
{
    Q_OBJECT
public:
    explicit PsRender(PsProject *project, QObject *parent = 0);
    ~PsRender();

    int sheetCount() const;
    const PsSheet *sheet(int index) const;
    QImage image(int index);

public slots:
    void refresh();

signals:
    void changed(int sheetIndex);
    void started();
    void finished();

private:
    PsRenderPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(PsRender)



};

#endif // PSRENDER_H
