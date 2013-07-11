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
/*!
 * @copyright Copyright (C) 2009-2011 Nokia Corporation.
 * @license LGPL
 */

#ifndef IDENTITYINFOIMPL_H
#define IDENTITYINFOIMPL_H

#include "QtCore/qglobal.h"
#include <QMap>
#include <QVariantMap>

#include "identityinfo.h"

namespace SignOn {

typedef QMap<MethodName, MechanismsList> MethodMap;

/*!
 * @class IdentityInfoImpl
 * IdentityInfo class implementation.
 * @sa IdentityInfo
 */
class IdentityInfoImpl
{
    friend class IdentityInfo;
    friend class IdentityImpl;

    Q_DISABLE_COPY(IdentityInfoImpl)

public:
    IdentityInfoImpl(IdentityInfo *identityInfo);
    ~IdentityInfoImpl();

    void addMethod(const MethodName &method,
                   const MechanismsList &mechanismsList);
    void updateMethod(const MethodName &method,
                      const MechanismsList &mechanismsList);
    void removeMethod(const MethodName &method);
    void setType(IdentityInfo::CredentialsType type);
    IdentityInfo::CredentialsType type() const;
    void setRefCount(qint32 refCount);
    qint32 refCount() const;

    bool isEmpty() const;
    bool hasMethod(const MethodName &method) const;
    void clear();
    QVariantMap toMap() const;
    void updateFromMap(const QVariantMap &map);

private:
    void copy(const IdentityInfoImpl &other);

private:
    IdentityInfo *m_identityInfo;

    quint32 m_id;
    QString m_userName;
    QString m_secret;
    bool m_storeSecret;
    QString m_caption;
    MethodMap m_authMethods;
    QStringList m_realms;
    QStringList m_accessControlList;
    QString m_owner;
    IdentityInfo::CredentialsType m_type;
    qint32 m_refCount;
    bool m_isEmpty;
};

} //namespace SignOn

Q_DECLARE_METATYPE(SignOn::MethodMap)

#endif // IDENTITYINFOIMPL_H
