/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2012 Canonical Ltd.
 *
 * Contact: Aurel Popirtac <ext-aurel.popirtac@nokia.com>
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

#ifndef SIGNONIDENTITYADAPTOR_H
#define SIGNONIDENTITYADAPTOR_H

#include <QDBusAbstractAdaptor>
#include <QDBusContext>

#include "signond-common.h"
#include "signonidentity.h"

namespace SignonDaemonNS {

class SignonIdentityAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface",
                "com.google.code.AccountsSSO.SingleSignOn.Identity")

public:
    SignonIdentityAdaptor(SignonIdentity *parent);
    virtual ~SignonIdentityAdaptor();

    inline const QDBusContext &parentDBusContext() const
        { return *static_cast<QDBusContext *>(m_parent); }

public Q_SLOTS:
    quint32 requestCredentialsUpdate(const QString &message);
    QVariantMap getInfo();
    void addReference(const QString &reference);
    void removeReference(const QString &reference);

    bool verifyUser(const QVariantMap &params);
    bool verifySecret(const QString &secret);
    void remove();
    bool signOut();
    quint32 store(const QVariantMap &info);

Q_SIGNALS:
    void unregistered();
    void infoUpdated(int);

private:
    void securityErrorReply(const char *failedMethodName);
    void errorReply(const QString &name, const QString &message);

private:
    SignonIdentity *m_parent;
};

} //namespace SignonDaemonNS

#endif // SIGNONIDENTITYADAPTOR_H
