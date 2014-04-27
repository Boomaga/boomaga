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


#include "exporttopdfprinter.h"
#include "exporttopdf.h"
#include "kernel/project.h"
#include "settings.h"

#include <QDir>
#include <QDebug>


/************************************************

 ************************************************/
ExportToPDFPrinter::ExportToPDFPrinter()
{
    setDuplexType(DuplexAuto);
    mCanChangeDuplexType = false;
    mShowProgressDialog = false;
    mOutFileName = settings->value(Settings::ExportPDF_FileName).toString();
    readSettings();
}


/************************************************

 ************************************************/
QString ExportToPDFPrinter::printerName() const
{
    return QObject::tr("Print to file (PDF)");
}


/************************************************

 ************************************************/
bool ExportToPDFPrinter::print(const QList<Sheet *> &sheets, const QString &jobName, bool duplex, int numCopies) const
{
    ExportToPdf dialog;
    dialog.setOutFileName(mOutFileName);
    dialog.setMetaInfo(project->metaData());

    dialog.setModal(true);
    if (dialog.exec())
    {
        mOutFileName = dialog.outFileName();
        QString fileName = mOutFileName;
        settings->setValue(Settings::ExportPDF_FileName, mOutFileName);

        if (fileName.startsWith("~"))
            fileName.replace("~", QDir::homePath());

        project->setMetadata(dialog.metaInfo());
        project->writeDocument(sheets, fileName);
        return true;
    }
    return false;
}
