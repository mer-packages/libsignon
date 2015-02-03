/*
 * This file is part of signon
 *
 * Copyright (C) 2011 Intel Corporation.
 * Copyright (C) 2013 Canonical Ltd.
 *
 * Contact: Elena Reshetova <elena.reshetova@intel.com>
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
/*!
 * @file abstract-access-control-manager.h
 * Definition of the AbstractAccessControlManager object.
 * @ingroup Accounts_and_SSO_Framework
 */

#ifndef SIGNON_ABSTRACT_ACCESS_CONTROL_MANAGER_H
#define SIGNON_ABSTRACT_ACCESS_CONTROL_MANAGER_H

#include <SignOn/extension-interface.h>

#include <QSharedDataPointer>
#include <QString>

class QDBusConnection;
class QDBusMessage;

namespace SignOn {

class AbstractAccessControlManager;
class AccessRequestData;

class SIGNON_EXPORT AccessRequest
{
public:
    explicit AccessRequest();
    AccessRequest(const AccessRequest &other);
    ~AccessRequest();

    /*!
     * Identifies the client requesting the access.
     * @param peerConnection the connection over which the message was sent.
     * @param peerMessage, the request message sent over DBUS by the process.
     */
    void setPeer(const QDBusConnection &connection,
                 const QDBusMessage &message);
    const QDBusConnection &peerConnection() const;
    const QDBusMessage &peerMessage() const;

    /*!
     * Specifies the SignOn::Identity resource being accessed.
     */
    void setIdentity(quint32 id);
    quint32 identity() const;

private:
    QSharedDataPointer<AccessRequestData> d;
};

class AccessReplyPrivate;
class SIGNON_EXPORT AccessReply: public QObject
{
    Q_OBJECT

public:
    ~AccessReply();

    const AccessRequest &request() const;
    bool isAccepted() const;

Q_SIGNALS:
    void finished();

protected:
    explicit AccessReply(const AccessRequest &request, QObject *parent = 0);

protected Q_SLOTS:
    void accept();
    void decline();

private:
    friend class AbstractAccessControlManager;
    AccessReplyPrivate *d_ptr;
    Q_DECLARE_PRIVATE(AccessReply)
};

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
     * @param peerConnection the connection over which the message was sent.
     * @param peerMessage, the request message sent over DBUS by the process.
     * @param securityContext, the securityContext to be checked against.
     * @returns true, if the peer is allowed, false otherwise.
     */
    virtual bool isPeerAllowedToAccess(const QDBusConnection &peerConnection,
                                       const QDBusMessage &peerMessage,
                                       const QString &securityContext);

    /*!
     * Looks up for the application identifier of a specific client process.
     * @param peerConnection the connection over which the message was sent.
     * @param peerMessage, the request message sent over DBUS by the process.
     * @returns the application identifier of the process, or an empty string
     * if none found.
     */
    virtual QString appIdOfPeer(const QDBusConnection &peerConnection,
                                const QDBusMessage &peerMessage);

    /*!
     * @returns the application identifier of the keychain widget
     */
    virtual QString keychainWidgetAppId();

    /*!
     * Asynchronously handle an access request from a client.
     * @param request, the AccessRequest describing the requested access.
     * @returns an AccessReply object which can be used to obtain the
     * asynchronous reply.
     */
    virtual AccessReply *handleRequest(const AccessRequest &request);
};

} // namespace

#endif // SIGNON_ABSTRACT_ACCESS_CONTROL_MANAGER_H
