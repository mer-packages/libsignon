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

#ifndef SIGNONDAEMONADAPTER_H_
#define SIGNONDAEMONADAPTER_H_

#include <QtCore>
#include <QtDBus>

#include "signond-common.h"
#include "signondaemon.h"


namespace SignonDaemonNS {

typedef QList<QVariantMap> MapList;

class SignonDaemonAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface",
                "com.google.code.AccountsSSO.SingleSignOn.AuthService")

public:
    SignonDaemonAdaptor(SignonDaemon *parent);
    virtual ~SignonDaemonAdaptor();

    inline const QDBusContext &parentDBusContext() const
        { return *static_cast<QDBusContext *>(m_parent); }

public Q_SLOTS:
    void registerNewIdentity(QDBusObjectPath &objectPath);
    void getIdentity(const quint32 id, QDBusObjectPath &objectPath,
                     QVariantMap &identityData);
    QString getAuthSessionObjectPath(const quint32 id, const QString &type);

    QStringList queryMethods();
    QStringList queryMechanisms(const QString &method);
    void queryIdentities(const QVariantMap &filter);
    bool clear();

private:
    void securityErrorReply();
    void securityErrorReply(const QDBusConnection &connection,
                            const QDBusMessage &message);
    bool handleLastError(const QDBusConnection &connection,
                         const QDBusMessage &message);
    QDBusObjectPath registerObject(const QDBusConnection &connection,
                                   QObject *object);

private Q_SLOTS:
    void onIdentityAccessReplyFinished();
    void onAuthSessionAccessReplyFinished();

private:
    SignonDaemon *m_parent;
}; //class SignonDaemonAdaptor

} //namespace SignonDaemonNS

Q_DECLARE_METATYPE(SignonDaemonNS::MapList)

#endif /* SIGNONDAEMONADAPTER_H_ */
