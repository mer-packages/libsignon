/*
 * This file is part of signon
 *
 * Copyright (C) 2011 Intel Corporation.
 *
 * Contact: Elena Reshetova <elena.reshetova@intel.com>
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
/*!
 * @file abstract-access-control-manager.h
 * Definition of the AbstractAccessControlManager object.
 * @ingroup Accounts_and_SSO_Framework
 */

#ifndef SIGNON_ABSTRACT_ACCESS_CONTROL_MANAGER_H
#define SIGNON_ABSTRACT_ACCESS_CONTROL_MANAGER_H

#include <SignOn/extension-interface.h>

#include <QString>
#include <QDBusMessage>


namespace SignOn {

/*!
 * @class AbstractAccessControlManager
 * Helps filtering incoming Singnon Daemon requests,
 * based on security priviledges of the client processes.
 * @ingroup Accounts_and_SSO_Framework
 */
class SIGNON_EXPORT AbstractAccessControlManager: public QObject
{
    Q_OBJECT

public:
    /*!
     * Constructs a AbstractAccessControlManager object with the given parent.
     * @param parent
     */
    explicit AbstractAccessControlManager(QObject *parent = 0);

    /*!
     * Destructor.
     */
    virtual ~AbstractAccessControlManager();

    /*!
     * Checks if a client process is allowed to access objects with a certain
     * security context.
     * The access type to be checked depends on the concrete implementation of
     * this function.
     * @param peerMessage, the request message sent over DBUS by the process.
     * @param securityContext, the securityContext to be checked against.
     * @returns true, if the peer is allowed, false otherwise.
     */
    virtual bool isPeerAllowedToAccess(const QDBusMessage &peerMessage,
                                       const QString &securityContext);

    /*!
     * Looks up for the application identifier of a specific client process.
     * @param peerMessage, the request message sent over DBUS by the process.
     * @returns the application identifier of the process, or an empty string
     * if none found.
     */
    virtual QString appIdOfPeer(const QDBusMessage &peerMessage);

    /*!
     * @returns the application identifier of the keychain widget
     */
    virtual QString keychainWidgetAppId();
};

} // namespace

#endif // SIGNON_ABSTRACT_ACCESS_CONTROL_MANAGER_H
