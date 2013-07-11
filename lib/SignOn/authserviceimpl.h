/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
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
#ifndef AUTHSERVICEIMPL_H
#define AUTHSERVICEIMPL_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QQueue>

#include "authservice.h"
#include "dbusinterface.h"

namespace SignOn {

class IdentityInfo;

typedef QList<QVariantMap> MapList;

/*!
 * @class AuthServiceImpl
 * AuthService class implementation
 * @sa AuthService
 */
class AuthServiceImpl: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AuthServiceImpl)

    friend class Identity;
    friend class AuthService;

public:
    AuthServiceImpl(AuthService *parent);
    virtual ~AuthServiceImpl();

    void queryMethods();
    void queryMechanisms(const QString &method);
    void queryIdentities(const AuthService::IdentityFilter &filter);
    void clear();

public Q_SLOTS:
    void errorReply(const QDBusError &err);
    void queryMechanismsReply(const QStringList &mechs);
    void queryIdentitiesReply(const QDBusMessage &msg);
    void queryMethodsReply(const QStringList &methods);
    void clearReply();

private:
    bool sendRequest(const QString &operation,
                     const char *replySlot,
                     const QList<QVariant> &args = QList<QVariant>(),
                     int timeout = -1);

private:
    AuthService *m_parent;
    DBusInterface *m_DBusInterface;
    QQueue<QString> m_methodsForWhichMechsWereQueried;
};

} // namespace SignOn

Q_DECLARE_METATYPE(SignOn::MapList)

#endif /* AUTHSERVICEIMPL_H */
