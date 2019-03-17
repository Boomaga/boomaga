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

#if MAC_UPDATER

#include "updater.h"

#include <AppKit/AppKit.h>
#include <Cocoa/Cocoa.h>
#include <Sparkle.h>
#include <QDate>
#include <QDebug>


class Updater::Private {
public:
    NSAutoreleasePool* mAutoReleasePool;
    SUUpdater* mUpdater;

};


/************************************************
 *
 ************************************************/
Updater &Updater::sharedUpdater()
{
    static Updater *res = nullptr;

    if (!res)
        res = new Updater();

    return *res;
}


/************************************************
 *
 ************************************************/
Updater::Updater():
    QObject(),
    d(new Updater::Private())
{
    NSApplicationLoad();
    d->mAutoReleasePool = [[NSAutoreleasePool alloc] init];

    d->mUpdater = [SUUpdater sharedUpdater];
    [d->mUpdater retain];
}


/************************************************
 *
 ************************************************/
Updater::~Updater()
{
    [d->mUpdater release];
    [d->mAutoReleasePool release];
    delete d;
}


/************************************************
 *
 ************************************************/
void Updater::checkForUpdatesInBackground()
{
    [d->mUpdater checkForUpdatesInBackground];
}


/************************************************
 *
 ************************************************/
QUrl Updater::feedURL() const
{

    NSURL *nsUrl = [d->mUpdater feedURL];
    if (nsUrl)
        return QUrl::fromNSURL(nsUrl);

    return QUrl();
}


/************************************************
 *
 ************************************************/
void Updater::setFeedURL(const QUrl &url)
{
    [d->mUpdater setFeedURL: url.toNSURL()];
}


/************************************************
 *
 ************************************************/
void Updater::setFeedURL(const char *url)
{
    setFeedURL(QString::fromLocal8Bit(url));
}


/************************************************
 *
 ************************************************/
bool Updater::automaticallyChecksForUpdates() const
{
    return [d->mUpdater automaticallyChecksForUpdates];
}

/************************************************
 *
 ************************************************/
void Updater::setAutomaticallyChecksForUpdates(bool enable)
{
    [d->mUpdater setAutomaticallyChecksForUpdates: enable];
}


/************************************************
 *
 ************************************************/
QDateTime Updater::lastUpdateCheckDate() const
{
    NSDate *date = [d->mUpdater lastUpdateCheckDate];
    if (date)
        return QDateTime::fromNSDate(date);

    return QDateTime();
}

#endif
