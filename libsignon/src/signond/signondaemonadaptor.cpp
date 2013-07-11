/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2011 Intel Corporation.
 *
 * Contact: Aurel Popirtac <ext-aurel.popirtac@nokia.com>
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

#include "signondaemonadaptor.h"
#include "signondisposable.h"
#include "accesscontrolmanagerhelper.h"

namespace SignonDaemonNS {

SignonDaemonAdaptor::SignonDaemonAdaptor(SignonDaemon *parent):
    QDBusAbstractAdaptor(parent),
    m_parent(parent)
{
    setAutoRelaySignals(false);
}

SignonDaemonAdaptor::~SignonDaemonAdaptor()
{
}

void SignonDaemonAdaptor::registerNewIdentity(QDBusObjectPath &objectPath)
{
    m_parent->registerNewIdentity(objectPath);

    SignonDisposable::destroyUnused();
}

void SignonDaemonAdaptor::securityErrorReply(const char *failedMethodName)
{
    QString errMsg;
    QTextStream(&errMsg) << SIGNOND_PERMISSION_DENIED_ERR_STR
                         << "Method:"
                         << failedMethodName;

    QDBusMessage msg = parentDBusContext().message();
    msg.setDelayedReply(true);
    QDBusMessage errReply =
                msg.createErrorReply(SIGNOND_PERMISSION_DENIED_ERR_NAME,
                                     errMsg);
    SIGNOND_BUS.send(errReply);
    TRACE() << "Method FAILED Access Control check:" << failedMethodName;
}

void SignonDaemonAdaptor::getIdentity(const quint32 id,
                                      QDBusObjectPath &objectPath,
                                      QVariantMap &identityData)
{
    if (!AccessControlManagerHelper::instance()->isPeerAllowedToUseIdentity(
                                    parentDBusContext().message(), id)) {
        securityErrorReply(__func__);
        return;
    }

    m_parent->getIdentity(id, objectPath, identityData);

    SignonDisposable::destroyUnused();
}

QStringList SignonDaemonAdaptor::queryMethods()
{
    return m_parent->queryMethods();
}

QString SignonDaemonAdaptor::getAuthSessionObjectPath(const quint32 id,
                                                      const QString &type)
{
    SignonDisposable::destroyUnused();

    /* Access Control */
    if (id != SIGNOND_NEW_IDENTITY) {
        if (!AccessControlManagerHelper::instance()->isPeerAllowedToUseAuthSession(
                                        parentDBusContext().message(), id)) {
            securityErrorReply(__func__);
            return QString();
        }
    }

    TRACE() << "ACM passed, creating AuthSession object";
    return m_parent->getAuthSessionObjectPath(id, type);
}

QStringList SignonDaemonAdaptor::queryMechanisms(const QString &method)
{
    return m_parent->queryMechanisms(method);
}

void SignonDaemonAdaptor::queryIdentities(const QVariantMap &filter)
{
    /* Access Control */
    if (!AccessControlManagerHelper::instance()->isPeerKeychainWidget(
                                              parentDBusContext().message())) {
        securityErrorReply(__func__);
        return;
    }

    QDBusMessage msg = parentDBusContext().message();
    msg.setDelayedReply(true);
    MapList identities = m_parent->queryIdentities(filter);
    QDBusMessage reply = msg.createReply(QVariant::fromValue(identities));
    SIGNOND_BUS.send(reply);
}

bool SignonDaemonAdaptor::clear()
{
    /* Access Control */
    if (!AccessControlManagerHelper::instance()->isPeerKeychainWidget(
                                              parentDBusContext().message())) {
        securityErrorReply(__func__);
        return false;
    }

    return m_parent->clear();
}

} //namespace SignonDaemonNS
