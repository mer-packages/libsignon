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
#ifndef SIGNONIDENTITYINFO_H
#define SIGNONIDENTITYINFO_H

#include <QMap>
#include <QStringList>
#include <QVariant>

#include "signond/signoncommon.h"

namespace SignonDaemonNS {

typedef QString MethodName;
typedef QStringList MechanismsList;
typedef QMap<MethodName, MechanismsList> MethodMap;

/*!
 * @struct SignonIdentityInfo
 * Daemon side representation of identity information.
 * @todo description.
 */
struct SignonIdentityInfo
{
    SignonIdentityInfo();
    SignonIdentityInfo(const QVariantMap &info);
    SignonIdentityInfo(const quint32 id,
                       const QString &userName,
                       const QString &password,
                       const bool storePassword,
                       const QString &caption,
                       const MethodMap &methods,
                       const QStringList &realms = QStringList(),
                       const QStringList &accessControlList = QStringList(),
                       const QStringList &ownerList = QStringList(),
                       int type = 0,
                       int refCount = 0,
                       bool validated = false);

    const QList<QVariant> toVariantList();
    const QVariantMap toMap() const;

    bool operator== (const SignonIdentityInfo &other) const;
    SignonIdentityInfo &operator=(const SignonIdentityInfo &other);

    void setNew() { m_id = SIGNOND_NEW_IDENTITY; }
    bool isNew() const { return m_id == SIGNOND_NEW_IDENTITY; }
    void setId(quint32 id) { m_id = id; }
    quint32 id() const { return m_id; }

    void setUserName(const QString &userName) { m_userName = userName; }
    QString userName() const { return m_userName; }
    void setUserNameSecret(bool secret) { m_isUserNameSecret = secret; }
    bool isUserNameSecret() const { return m_isUserNameSecret; }

    void setPassword(const QString &password) { m_password = password; }
    QString password() const { return m_password; }
    void setStorePassword(bool storePassword) {
        m_storePassword = storePassword;
    }
    bool storePassword() const { return m_storePassword; }

    void setCaption(const QString &caption) { m_caption = caption; }
    QString caption() const { return m_caption; }

    void setRealms(const QStringList &realms) { m_realms = realms; }
    QStringList realms() const { return m_realms; }

    void setMethods(const MethodMap &methods)
        { m_methods = methods; }
    MethodMap methods() const { return m_methods; }

    void setAccessControlList(const QStringList &acl)
        { m_accessControlList = acl; }
    QStringList accessControlList() const { return m_accessControlList; }

    void setValidated(bool validated) { m_validated = validated; }
    bool validated() const { return m_validated; }

    void setType(const int type) { m_type = type; }
    int type() const { return m_type; }

    void setOwnerList(const QStringList &owner) { m_ownerList = owner; }
    QStringList ownerList() const { return m_ownerList; }

    bool checkMethodAndMechanism(const QString &method,
                                 const QString &mechanism,
                                 QString &allowedMechanism);

private:
    quint32 m_id;
    QString m_userName;
    QString m_password;
    bool m_storePassword;
    QString m_caption;
    MethodMap m_methods;
    QStringList m_realms;
    QStringList m_accessControlList;
    QStringList m_ownerList;
    int m_type;
    int m_refCount;
    bool m_validated;
    bool m_isUserNameSecret;
}; //struct SignonIdentityInfo

} //namespace SignonDaemonNS

Q_DECLARE_METATYPE(SignonDaemonNS::MethodMap)

#endif // SIGNONIDENTITYINFO_H
