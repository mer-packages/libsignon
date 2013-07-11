/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2011 Intel Corporation.
 *
 * Contact: Aurel Popirtac <ext-aurel.popirtac@nokia.com>
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
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

#include <QBuffer>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include "accesscontrolmanagerhelper.h"
#include "signond-common.h"
#include "credentialsaccessmanager.h"
#include "signonidentity.h"

using namespace SignonDaemonNS;

AccessControlManagerHelper *AccessControlManagerHelper::m_pInstance = NULL;

AccessControlManagerHelper *AccessControlManagerHelper::instance()
{
    return m_pInstance;
}

AccessControlManagerHelper::AccessControlManagerHelper(
                                SignOn::AbstractAccessControlManager *acManager)
{
    if (!m_pInstance) {
        m_pInstance = this;
        m_acManager = acManager;
    } else {
        BLAME() << "Creating a second instance of the CAM";
    }
}

AccessControlManagerHelper::~AccessControlManagerHelper()
{
    m_acManager = NULL;
    m_pInstance = NULL;
}


bool
AccessControlManagerHelper::isPeerAllowedToUseIdentity(
                                               const QDBusMessage &peerMessage,
                                               const quint32 identityId)
{
    // TODO - improve this, the error handling and more precise behaviour

    CredentialsDB *db = CredentialsAccessManager::instance()->credentialsDB();
    if (db == 0) {
        TRACE() << "NULL db pointer, secure storage might be unavailable,";
        return false;
    }
    QStringList acl = db->accessControlList(identityId);

    TRACE() << QString(QLatin1String("Access control list of identity: "
                                 "%1: [%2].Tokens count: %3\t"))
                                .arg(identityId)
                                .arg(acl.join(QLatin1String(", ")))
                                .arg(acl.size());

    if (db->errorOccurred())
        return false;

    if (acl.isEmpty())
        return true;

    return peerHasOneOfAccesses(peerMessage, acl);
}

AccessControlManagerHelper::IdentityOwnership
AccessControlManagerHelper::isPeerOwnerOfIdentity(
                                               const QDBusMessage &peerMessage,
                                               const quint32 identityId)
{
    CredentialsDB *db = CredentialsAccessManager::instance()->credentialsDB();
    if (db == 0) {
        TRACE() << "NULL db pointer, secure storage might be unavailable,";
        return ApplicationIsNotOwner;
    }
    QStringList ownerSecContexts = db->ownerList(identityId);

    if (db->errorOccurred())
        return ApplicationIsNotOwner;

    if (ownerSecContexts.isEmpty())
        return IdentityDoesNotHaveOwner;

    return peerHasOneOfAccesses(peerMessage, ownerSecContexts) ?
        ApplicationIsOwner : ApplicationIsNotOwner;
}

bool
AccessControlManagerHelper::isPeerKeychainWidget(const QDBusMessage &peerMessage)
{
    static QString keychainWidgetAppId = m_acManager->keychainWidgetAppId();
    QString peerAppId = m_acManager->appIdOfPeer(peerMessage);
    return (peerAppId == keychainWidgetAppId);
}

QString AccessControlManagerHelper::appIdOfPeer(const QDBusMessage &peerMessage)
{
    TRACE() << m_acManager->appIdOfPeer(peerMessage);
    return m_acManager->appIdOfPeer(peerMessage);
}

bool
AccessControlManagerHelper::peerHasOneOfAccesses(const QDBusMessage &peerMessage,
                                                 const QStringList secContexts)
{
    foreach(QString securityContext, secContexts)
    {
        TRACE() << securityContext;
        if (m_acManager->isPeerAllowedToAccess(peerMessage, securityContext))
            return true;
    }

    BLAME() << "given peer does not have needed permissions";
    return false;
}

bool
AccessControlManagerHelper::isPeerAllowedToAccess(
                                               const QDBusMessage &peerMessage,
                                               const QString securityContext)
{
    TRACE() << securityContext;
    return m_acManager->isPeerAllowedToAccess(peerMessage, securityContext);
}

pid_t AccessControlManagerHelper::pidOfPeer(const QDBusContext &peerContext)
{
    QString service = peerContext.message().service();
    return peerContext.connection().interface()->servicePid(service).value();
}

