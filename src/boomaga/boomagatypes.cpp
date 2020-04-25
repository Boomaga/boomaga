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

#include "boomagatypes.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QUuid>
#include <QTemporaryDir>

/************************************************

 ************************************************/
QString flipTypeToStr(FlipType value)
{
    switch (value)
    {
    case FlipType::LongEdge:    return "LongEdge";
    case FlipType::ShortEdge:   return "ShortEdge";
    }
    return "";
}


/************************************************

 ************************************************/
FlipType strToFlipType(const QString &str)
{
    QString s = str.toUpper();
    if (s.contains("SHORT"))    return FlipType::ShortEdge;
    return FlipType::LongEdge;
}


/************************************************

 ************************************************/
QString duplexTypeToStr(DuplexType value)
{
    switch (value)
    {
    case DuplexAuto:          return "Auto";
    case DuplexManual:        return "Manual";
    case DuplexManualReverse: return "ManualReverse";
    }
    return "";
}


/************************************************

 ************************************************/
DuplexType strToDuplexType(const QString &str)
{
    QString s = str.toUpper();
    if (s == "AUTO")        return DuplexAuto;
    if (s == "MANUAL")      return DuplexManual;
    return DuplexManualReverse;
}


/************************************************

 ************************************************/
QString colorModeToStr(ColorMode value)
{
    switch (value)
    {
    case ColorModeAuto:         return "Auto";
    case ColorModeGrayscale:    return "Grayscale";
    case ColorModeColor:        return "Color";
    }
    return "";
}


/************************************************

 ************************************************/
ColorMode strToColorMode(const QString &str)
{
    QString s = str.toUpper();
    if (s == "GRAYSCALE")  return ColorModeGrayscale;
    if (s == "GRAY")       return ColorModeGrayscale;
    if (s == "COLOR")      return ColorModeColor;
    return ColorModeAuto;
}


/************************************************

 ************************************************/
QString safeFileName(const QString &str)
{
    QString res = str;
    res.replace('|', "-");
    res.replace('/', "-");
    res.replace('\\', "-");
    res.replace(':', "-");
    res.replace('*', "-");
    res.replace('?', "-");
    return res;
}


/************************************************

 ************************************************/
QString expandHomeDir(const QString &fileName)
{
    QString res = fileName;

    if (res.startsWith("~"))
        res.replace("~", QDir::homePath());

    return res;
}


/************************************************

 ************************************************/
QString shrinkHomeDir(const QString &fileName)
{
    QString res = fileName;

    if (res.startsWith(QDir::homePath()))
        res.replace(QDir::homePath(), "~");

    return res;

}


/************************************************
 *
 ************************************************/
static QString chacheDir()
{
    QString res = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
    res = expandHomeDir(res);
    return res;
}


/************************************************
 *
 ************************************************/
QString tmpDir()
{
    if (const char* dir = std::getenv("BOOMAGA_TMP_DIR")) {
        return dir;
    }

    static QTemporaryDir res(chacheDir() + QDir::separator() + "boomaga-XXXXXX");
    return res.path();
}


/************************************************
 *
 ************************************************/
void mustOpenFile(const QString &fileName, QFile *file)
{
    QFileInfo fi(fileName);

    if (fi.filePath().isEmpty())
        throw BoomagaError(QObject::tr("I can't open file \"%1\" (Empty file name)",
                                       "Error message. %1 is a file name")
                           .arg(fi.filePath()));

    if (!fi.exists())
        throw BoomagaError(QObject::tr("I can't open file \"%1\" (No such file or directory)",
                                       "Error message. %1 is a file name")
                           .arg(fi.filePath()));

    if (!fi.isReadable())
        throw BoomagaError(QObject::tr("I can't open file \"%1\" (Access denied)",
                                       "Error message. %1 is a file name")
                           .arg(fi.filePath()));


    file->setFileName(fileName);
    if(!file->open(QFile::ReadOnly))
        throw BoomagaError(QObject::tr("I can't open file \"%1\"",
                                       "Error message. %1 is a file name")
                           .arg(fi.filePath()) +
                           "\n" + file->errorString());
}


/************************************************
 *
 ************************************************/
QString appUUID()
{
    static QString res = "boomaga-" + QUuid::createUuid().toString().mid(1, 36);
    return res;
}


/************************************************
 *
 ************************************************/
QString genTmpFileName()
{
    static QAtomicInteger<quint64> cnt(1);
    quint64 id = cnt.fetchAndAddRelaxed(1);

    return tmpDir() + QDir::separator() + QString("tmp-%1.pdf").arg(id, 3, 10, QChar('0'));
}


/************************************************
 *
 ************************************************/
QString genInputFileName()
{
    static QAtomicInteger<quint64> cnt(1);
    quint64 id = cnt.fetchAndAddRelaxed(1);

    return tmpDir() + QDir::separator() + QString("in-%1").arg(id, 3, 10, QChar('0'));
}


/************************************************
 *
 ************************************************/
QString genJobFileName()
{
    static QAtomicInteger<quint64> cnt(1);
    quint64 id = cnt.fetchAndAddRelaxed(1);

    return tmpDir() + QDir::separator() + QString("job-%1.pdf").arg(id, 3, 10, QChar('0'));
}
