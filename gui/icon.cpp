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


#include "icon.h"

#include <QHash>
#include <QDebug>

/************************************************
 *
 * ***********************************************/
Icon::Icon()
{
}


/************************************************
 *
 * ***********************************************/
void Icon::iconDefs(Icon::IconName iconName, QStringList *theme, QStringList *fallBack)
{
    switch (iconName)
    {
    case ApplicationIcon:
        *theme << "boomaga";
        *fallBack << ":/images/icon-16x16"          // 16
                  << ""                             // 22
                  << ":/images/icon-32x32"          // 32
                  << ""                             // 48
                  << ":/images/icon-64x64"          // 64
                  << ":/images/icon-128x128";       // 128
        break;

    case RotateLeft:
        *theme << "object-rotate-left";
        *fallBack << ":/images/rotate-left-16x16"   // 16
                  << ":/images/rotate-left-22x22"   // 22
                  << ":/images/rotate-left-32x32"   // 32
                  << ""                             // 48
                  << ""                             // 64
                  << "";                            // 128
        break;

    case RotateRight:
        *theme << "object-rotate-right";
        *fallBack << ":/images/rotate-right-16x16"  // 16
                  << ":/images/rotate-right-22x22"  // 22
                  << ":/images/rotate-right-32x32"  // 32
                  << ""                             // 48
                  << ""                             // 64
                  << "";                            // 128
        break;

    case Print:
        *theme << "document-print";
        *fallBack << ""                             // 16
                  << ""                             // 22
                  << ""                             // 32
                  << ":/images/print-48x48"         // 48
                  << ""                             // 64
                  << "";                            // 128
        break;


    case Previous:
        *theme << "go-previous-view";
        *fallBack << ""                             // 16
                  << ""                             // 22
                  << ":/images/previous"            // 32
                  << ""                             // 48
                  << ""                             // 64
                  << "";                            // 128
        break;

    case Next:
        *theme << "go-next-view";
        *fallBack << ""                             // 16
                  << ""                             // 22
                  << ":/images/next"                // 32
                  << ""                             // 48
                  << ""                             // 64
                  << "";                            // 128
        break;

    case Open:
        *theme << "document-open";
        *fallBack << ""                             // 16
                  << ""                             // 22
                  << ":/images/open"                // 32
                  << ""                             // 48
                  << ""                             // 64
                  << "";                            // 128
        break;

    case Save:
        *theme << "document-save";
        *fallBack << ""                             // 16
                  << ""                             // 22
                  << ":/images/save"                // 32
                  << ""                             // 48
                  << ""                             // 64
                  << "";                            // 128
        break;

    case SaveAs:
        *theme << "document-save-as";
        *fallBack << ""                             // 16
                  << ""                             // 22
                  << ":/images/save-as"             // 32
                  << ""                             // 48
                  << ""                             // 64
                  << "";                            // 128
        break;

    case Configure:
        *theme << "configure";
        *fallBack << ":/images/configure-16x16"     // 16
                  << ":/images/configure-22x22"     // 22
                  << ":/images/configure-32x32"     // 32
                  << ""                             // 48
                  << ":/images/configure-64x64"     // 64
                  << "";                            // 128
        break;

    }
}



/************************************************
 *
 * ***********************************************/
QIcon Icon::icon(Icon::IconName iconName)
{
    static QHash<IconName, QIcon> cache;


    if (cache.contains(iconName))
        return cache.value(iconName);

    QIcon icon = loadIcon(iconName);
    cache.insert(iconName, icon);
    return icon;
}



/************************************************
 *
 ************************************************/
QIcon Icon::loadIcon(Icon::IconName iconName)
{
    const int sizes[] = {16, 22, 32, 48, 64, 128};
    const int sizesLen = sizeof(sizes)/sizeof(*sizes);

    QStringList theme;
    QStringList fallback;
    iconDefs(iconName, &theme, &fallback);

    foreach (QString s, theme)
    {
        QIcon icon = QIcon::fromTheme(s);
        if (!icon.isNull())
        {
            QIcon res;
            int lastSize = 0;
            QList<QSize> availableSizes = icon.availableSizes();
            for (int i = sizesLen-1; i>=0; --i)
            {
                int sz = sizes[i];
                if (availableSizes.contains(QSize(sz, sz)))
                {
                    res.addPixmap(icon.pixmap(sz, sz));
                    lastSize = sz;
                }
                else
                {
                    if (lastSize)
                    {
                        res.addPixmap(icon.pixmap(lastSize, lastSize).scaled(sz, sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                    }
                }
            }
            return res;
        }


    }

    QIcon icon;

    for (int i=0; i<fallback.count(); ++i)
    {
        if (!fallback.at(i).isEmpty())
            icon.addFile(fallback.at(i), QSize(sizes[i], sizes[i]));
    }
    return icon;
}

