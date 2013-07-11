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

#ifndef SIGNONDISPOSABLE_H_
#define SIGNONDISPOSABLE_H_

#include "signond-common.h"

#include <QtCore>
#include <time.h>

namespace SignonDaemonNS {

/*!
 * @class SignonDisposable
 *
 * Base class for server objects that can be automatically destroyed after
 * a certain period of inactivity.
 */
class SignonDisposable: public QObject
{
    Q_OBJECT

protected:
    virtual ~SignonDisposable();

public:
    /*!
     * Construct an object that can be automatically destroyed after
     * having being unused for @maxInactivity seconds.
     *
     * @param maxInactivity the number of seconds of inactivity.
     * @param parent the parent object.
     */
    SignonDisposable(int maxInactivity, QObject *parent);

    /*!
     * Performs any predestruction operations and the destruction itself.
     * Reimplement this for smoother control.
     */
    virtual void destroy() { deleteLater(); }

    /*!
     * Mark the object as used. Calling this method causes the inactivity
     * timer to be reset.
     */
    void keepInUse() const;

    /*!
     * Mark the object as used. Calling this method enables/disables
     * autodestruction.
     * @param value enable/disable autodestruction
     */
    void setAutoDestruct(bool value = true) const;

    /*!
     * Invoke the specified method on @object when there are no
     * disposable objects for more than @maxInactivity seconds.
     *
     * To keep the implementation simpler, this function can be called only
     * once, and the @member variable must still be accessible when the method
     * will be invoked (use a static string).
     */
    static void invokeOnIdle(int maxInactivity,
                             QObject *object, const char *member);

public Q_SLOTS:
    /*!
     * Deletes all disposable object for which the inactivity time has
     * elapsed.
     */
    static void destroyUnused();

private:
    int maxInactivity;
    mutable time_t lastActivity;
    mutable bool autoDestruct;
}; //class SignonDaemon

} //namespace SignonDaemonNS

#endif /* SIGNONDISPOSABLE_H_ */
