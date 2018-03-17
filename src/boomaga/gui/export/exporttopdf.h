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


#ifndef EXPORTTOPDF_H
#define EXPORTTOPDF_H

#include <QDialog>
#include <QString>
#include "kernel/project.h"

namespace Ui {
class ExportToPdf;
}

class ExportToPdf : public QDialog
{
    Q_OBJECT
    
public:
    explicit ExportToPdf(QWidget *parent = 0);
    ~ExportToPdf();
    
    QString outFileName() const;
    void setOutFileName(const QString &value);

    MetaData metaInfo() const;
    void setMetaInfo(const MetaData &value);

public slots:
    void accept();

private slots:
    void selectFile();
    void outFileNameChanged();

private:
    Ui::ExportToPdf *ui;
};

#endif // EXPORTTOPDF_H
