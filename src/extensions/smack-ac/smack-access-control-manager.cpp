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

#include "smack-access-control-manager.h"
#include "debug.h"

#include <DBusSmackContext>
#include <SmackQt>

static const char keychainAppId[] = "SignondKeychain";

SmackAccessControlManager::SmackAccessControlManager(QObject *parent):
    SignOn::AbstractAccessControlManager(parent)
{
}

SmackAccessControlManager::~SmackAccessControlManager()
{
}

QString SmackAccessControlManager::keychainWidgetAppId()
{
    return QLatin1String(keychainAppId);
}

bool SmackAccessControlManager::isPeerAllowedToAccess(
                                               const QDBusMessage &peerMessage,
                                               const QString &securityContext)
{
    QString appId =
        SmackQt::DBusSmackContext::getCallerSmackContext(peerMessage);
    TRACE() << appId << ":" << securityContext;

    if (SmackQt::Smack::hasAccess(appId, securityContext, QLatin1String("r")) ||
        SmackQt::Smack::hasAccess(appId, securityContext, QLatin1String("x"))) {
            TRACE() << "Process ACCESS:TRUE";
            return true;
    } else {
            TRACE() << "Process ACCESS:FALSE";
            return false;
    }
}

QString SmackAccessControlManager::appIdOfPeer(const QDBusMessage &peerMessage)
{
    TRACE() << SmackQt::DBusSmackContext::getCallerSmackContext(peerMessage);
    return SmackQt::DBusSmackContext::getCallerSmackContext(peerMessage);
}

