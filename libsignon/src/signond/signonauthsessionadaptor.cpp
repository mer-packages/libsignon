/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2011 Intel Corporation.
 * Copyright (C) 2013 Canonical Ltd.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
 * Contact: Jussi Laako <jussi.laako@linux.intel.com>
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

#include "signonauthsessionadaptor.h"
#include "accesscontrolmanagerhelper.h"
#include "credentialsaccessmanager.h"
#include "credentialsdb.h"

namespace SignonDaemonNS {

SignonAuthSessionAdaptor::SignonAuthSessionAdaptor(SignonAuthSession *parent):
    QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

SignonAuthSessionAdaptor::~SignonAuthSessionAdaptor()
{
}

void SignonAuthSessionAdaptor::errorReply(const QString &name,
                                          const QString &message)
{
    const QDBusContext &context = *static_cast<QDBusContext *>(parent());
    QDBusMessage errReply = context.message().createErrorReply(name, message);
    context.connection().send(errReply);
}

QStringList
SignonAuthSessionAdaptor::queryAvailableMechanisms(
                                           const QStringList &wantedMechanisms)
{
    TRACE();

    QDBusContext &dbusContext = *static_cast<QDBusContext *>(parent());
    if (AccessControlManagerHelper::pidOfPeer(dbusContext) !=
        parent()->ownerPid()) {
        TRACE() << "queryAvailableMechanisms called from peer that doesn't "
            "own the AuthSession object\n";
        QString errMsg;
        QTextStream(&errMsg) << SIGNOND_PERMISSION_DENIED_ERR_STR
                             << " Authentication session owned by other "
                             "process.";
        errorReply(SIGNOND_PERMISSION_DENIED_ERR_NAME, errMsg);
        return QStringList();
    }

    return parent()->queryAvailableMechanisms(wantedMechanisms);
}

QVariantMap SignonAuthSessionAdaptor::process(const QVariantMap &sessionDataVa,
                                              const QString &mechanism)
{
    TRACE() << mechanism;

    QString allowedMechanism(mechanism);

    if (parent()->id() != SIGNOND_NEW_IDENTITY) {
        CredentialsDB *db =
            CredentialsAccessManager::instance()->credentialsDB();
        if (db) {
            SignonIdentityInfo identityInfo = db->credentials(parent()->id(),
                                                              false);
            if (!identityInfo.checkMethodAndMechanism(parent()->method(),
                                                      mechanism,
                                                      allowedMechanism)) {
                QString errMsg;
                QTextStream(&errMsg) << SIGNOND_METHOD_OR_MECHANISM_NOT_ALLOWED_ERR_STR
                                     << " Method:"
                                     << parent()->method()
                                     << ", mechanism:"
                                     << mechanism
                                     << ", allowed:"
                                     << allowedMechanism;
                errorReply(SIGNOND_METHOD_OR_MECHANISM_NOT_ALLOWED_ERR_NAME,
                           errMsg);
                return QVariantMap();
            }
        } else {
            BLAME() << "Null database handler object.";
        }
    }

    QDBusContext &dbusContext = *static_cast<QDBusContext *>(parent());
    if (AccessControlManagerHelper::pidOfPeer(dbusContext) !=
        parent()->ownerPid()) {
        TRACE() << "process called from peer that doesn't own the AuthSession "
            "object";
        QString errMsg;
        QTextStream(&errMsg) << SIGNOND_PERMISSION_DENIED_ERR_STR
                             << " Authentication session owned by other "
                             "process.";
        errorReply(SIGNOND_PERMISSION_DENIED_ERR_NAME, errMsg);
        return QVariantMap();
    }

    return parent()->process(sessionDataVa, allowedMechanism);
}

void SignonAuthSessionAdaptor::cancel()
{
    TRACE();

    QDBusContext &dbusContext = *static_cast<QDBusContext *>(parent());
    if (AccessControlManagerHelper::pidOfPeer(dbusContext) != parent()->ownerPid()) {
        TRACE() << "cancel called from peer that doesn't own the AuthSession "
            "object";
        return;
    }

    parent()->cancel();
}

void SignonAuthSessionAdaptor::setId(quint32 id)
{
    TRACE();

    QDBusContext &dbusContext = *static_cast<QDBusContext *>(parent());
    if (AccessControlManagerHelper::pidOfPeer(dbusContext) !=
        parent()->ownerPid()) {
        TRACE() << "setId called from peer that doesn't own the AuthSession "
            "object";
        return;
    }
    if (!AccessControlManagerHelper::instance()->isPeerAllowedToUseIdentity(
                                    dbusContext.connection(),
                                    dbusContext.message(),
                                    id)) {
        TRACE() << "setId called with an identifier the peer is not allowed "
            "to use";
        return;
    }

    parent()->setId(id);
}

void SignonAuthSessionAdaptor::objectUnref()
{
    TRACE();

    QDBusContext &dbusContext = *static_cast<QDBusContext *>(parent());
    if (AccessControlManagerHelper::pidOfPeer(dbusContext) !=
        parent()->ownerPid()) {
        TRACE() << "objectUnref called from peer that doesn't own the "
            "AuthSession object";
        return;
    }

    parent()->objectUnref();
}

} //namespace SignonDaemonNS
