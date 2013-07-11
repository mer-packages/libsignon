/*
 * This file is part of signon
 *
 * Copyright (C) 2010 Nokia Corporation.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include "signondisposable.h"

#include <QTimer>

namespace SignonDaemonNS {

static QList<SignonDisposable *> disposableObjects;
static QPointer<QTimer> notifyTimer = 0;
static QPointer<QTimer> disposeTimer = 0;

SignonDisposable::SignonDisposable(int maxInactivity, QObject *parent):
    QObject(parent),
    maxInactivity(maxInactivity),
    autoDestruct(true)
{
    disposableObjects.append(this);

    if (disposeTimer != 0) {
        int interval = (maxInactivity + 2) * 1000;
        if (interval > disposeTimer->interval())
            disposeTimer->setInterval(interval);
        QObject::connect(disposeTimer, SIGNAL(timeout()),
                         this, SLOT(destroyUnused()));
    }

    // mark as used
    keepInUse();
}

SignonDisposable::~SignonDisposable()
{
    disposableObjects.removeOne(this);
}

void SignonDisposable::keepInUse() const
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        qWarning("Couldn't get time from monotonic clock");
        return;
    }
    lastActivity = ts.tv_sec;

    if (notifyTimer != 0) {
        notifyTimer->stop();
    }
    if (disposeTimer != 0) {
        disposeTimer->start();
    }
}

void SignonDisposable::setAutoDestruct(bool value) const
{
    autoDestruct = value;
    keepInUse();
}

void SignonDisposable::invokeOnIdle(int maxInactivity,
                                    QObject *object, const char *member)
{
    notifyTimer = new QTimer(object);
    notifyTimer->setSingleShot(true);
    notifyTimer->setInterval(maxInactivity * 1000);
    QObject::connect(notifyTimer, SIGNAL(timeout()),
                     object, member);

    /* In addition to the notifyTimer, we create another timer to let
     * destroyUnused() to run when we expect that some SignonDisposable object
     * might be inactive: that is, a couple of seconds later than the maximum
     * inactivity interval. This timer is triggered by the keepInUse() method.
     */
    disposeTimer = new QTimer(object);
    disposeTimer->setSingleShot(true);
    int disposableMaxInactivity = 0;
    foreach (SignonDisposable *disposable, disposableObjects) {
        QObject::connect(disposeTimer, SIGNAL(timeout()),
                         disposable, SLOT(destroyUnused()));
        if (disposableMaxInactivity < disposable->maxInactivity)
            disposableMaxInactivity = disposable->maxInactivity;
    }

    // Add a couple of seconds, to run the check after the objects are inactive
    disposeTimer->setInterval((disposableMaxInactivity + 2) * 1000);
}

void SignonDisposable::destroyUnused()
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        qWarning("Couldn't get time from monotonic clock");
        return;
    }

    foreach (SignonDisposable *object, disposableObjects) {
        if (object->autoDestruct &&
            (ts.tv_sec - object->lastActivity > object->maxInactivity)) {
            TRACE() << "Object unused, deleting: " << object;
            object->destroy();
            disposableObjects.removeOne(object);
        }
    }

    if (disposableObjects.isEmpty() && notifyTimer != 0) {
        TRACE() << "No disposable objects, starting notification timer";
        notifyTimer->start();
    }
}

} //namespace SignonDaemonNS
