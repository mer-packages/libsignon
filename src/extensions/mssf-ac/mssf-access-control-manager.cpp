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

#include <DBusContextAccessManager>
#include <QStringList>

#include "mssf-access-control-manager.h"
#include "debug.h"

#define SSO_AEGIS_PACKAGE_ID_TOKEN_PREFIX QLatin1String("AID::")

static const char keychainAppId[] = "signond::keychain-access";

MSSFAccessControlManager::MSSFAccessControlManager(QObject *parent):
    SignOn::AbstractAccessControlManager(parent)
{
}

MSSFAccessControlManager::~MSSFAccessControlManager()
{
}

QString MSSFAccessControlManager::keychainWidgetAppId()
{
    return QLatin1String(keychainAppId);
}

bool MSSFAccessControlManager::isPeerAllowedToAccess(
                                       const QDBusMessage &peerMessage,
                                       const QString &securityContext)
{
    bool hasAccess = false;
    QStringList Credlist =
        MssfQt::DBusContextAccessManager::peerCredentials(peerMessage, NULL);
    foreach(QString cred, Credlist) {
        if (cred.compare(securityContext) == 0) {
            hasAccess = true;
            break;
        }
    }
    TRACE() << "Process ACCESS:" << (hasAccess ? "TRUE" : "FALSE");
    return hasAccess;
}

QString MSSFAccessControlManager::appIdOfPeer(const QDBusMessage &peerMessage)
{
    QStringList Credlist =
        MssfQt::DBusContextAccessManager::peerCredentials(peerMessage, NULL);
    foreach(QString cred, Credlist) {
        if (cred.startsWith(SSO_AEGIS_PACKAGE_ID_TOKEN_PREFIX))
            return cred;
    }

    return QString();
}

