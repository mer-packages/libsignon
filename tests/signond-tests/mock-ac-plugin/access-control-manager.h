/*
 * This file is part of signon
 *
 * Copyright (C) 2013 Canonical Ltd.
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

#ifndef SIGNON_MOCK_ACCESS_CONTROL_MANAGER_H
#define SIGNON_MOCK_ACCESS_CONTROL_MANAGER_H

#include <SignOn/AbstractAccessControlManager>

class AccessControlManager: public SignOn::AbstractAccessControlManager
{
    Q_OBJECT

public:
    AccessControlManager(QObject *parent = 0);
    ~AccessControlManager();

    bool isPeerAllowedToAccess(const QDBusConnection &peerConnection,
                               const QDBusMessage &peerMessage,
                               const QString &securityContext);

    QString appIdOfPeer(const QDBusConnection &peerConnection,
                        const QDBusMessage &peerMessage);

    QString keychainWidgetAppId();

    SignOn::AccessReply *handleRequest(const SignOn::AccessRequest &request);

private:
    pid_t pidOfPeer(const QDBusConnection &peerConnection,
                    const QDBusMessage &peerMessage);
};

#endif // SIGNON_MOCK_ACCESS_CONTROL_MANAGER_H
