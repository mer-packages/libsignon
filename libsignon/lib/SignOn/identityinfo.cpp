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

#include <QVariant>

#include "libsignoncommon.h"
#include "identityinfo.h"
#include "identityinfoimpl.h"
#include "identity.h"

namespace SignOn {

IdentityInfo::IdentityInfo():
    impl(new IdentityInfoImpl)
{
    qRegisterMetaType<IdentityInfo>("SignOn::IdentityInfo");

    if (qMetaTypeId<IdentityInfo>() < QMetaType::User)
        BLAME() << "IdentityInfo::IdentityInfo() - "
            "IdentityInfo meta type not registered.";
}

IdentityInfo::IdentityInfo(const IdentityInfo &other):
    impl(new IdentityInfoImpl)
{
    *impl = *other.impl;
}

IdentityInfo &IdentityInfo::operator=(const IdentityInfo &other)
{
    *impl = *other.impl;
    return *this;
}

IdentityInfo::IdentityInfo(const QString &caption,
                           const QString &userName,
                           const QMap<MethodName, MechanismsList> &methods):
    impl(new IdentityInfoImpl)
{
    impl->setCaption(caption);
    impl->setUserName(userName);
    impl->setMethods(methods);
}

IdentityInfo::~IdentityInfo()
{
    if (impl) delete impl;
    impl = 0;
}

void IdentityInfo::setId(const quint32 id)
{
    impl->setId(id);
}

quint32 IdentityInfo::id() const
{
    return impl->id();
}

void IdentityInfo::setUserName(const QString &userName)
{
    impl->setUserName(userName);
}

const QString IdentityInfo::userName() const
{
    return impl->userName();
}

void IdentityInfo::setCaption(const QString &caption)
{
    impl->setCaption(caption);
}

const QString IdentityInfo::caption() const
{
    return impl->caption();
}

void IdentityInfo::setRealms(const QStringList &realms)
{
    impl->setRealms(realms);
}

QStringList IdentityInfo::realms() const
{
    return impl->realms();
}

void IdentityInfo::setOwner(const QString &ownerToken)
{
    impl->setOwners(QStringList() << ownerToken);
}

QString IdentityInfo::owner() const
{
    return impl->owners().value(0);
}

void IdentityInfo::setAccessControlList(const QStringList &accessControlList)
{
    impl->setAccessControlList(accessControlList);
}

QStringList IdentityInfo::accessControlList() const
{
    return impl->accessControlList();
}

QString IdentityInfo::secret() const
{
    return impl->secret();
}

void IdentityInfo::setSecret(const QString &secret, const bool storeSecret)
{
    impl->setSecret(secret);
    impl->setStoreSecret(storeSecret);
}

bool IdentityInfo::isStoringSecret() const
{
    return impl->storeSecret();
}

void IdentityInfo::setStoreSecret(const bool storeSecret)
{
    impl->setStoreSecret(storeSecret);
}

void IdentityInfo::setMethod(const MethodName &method,
                             const MechanismsList &mechanismsList)
{
    impl->updateMethod(method, mechanismsList);
}

void IdentityInfo::removeMethod(const MethodName &method)
{
    impl->removeMethod(method);
}

void IdentityInfo::setType(IdentityInfo::CredentialsType type)
{
    impl->setType(type);
}

IdentityInfo::CredentialsType IdentityInfo::type() const
{
    return impl->type();
}

QList<MethodName> IdentityInfo::methods() const
{
    return impl->methods().keys();
}

MechanismsList IdentityInfo::mechanisms(const MethodName &method) const
{
    return impl->methods().value(method, QStringList());
}

void IdentityInfo::setRefCount(qint32 refCount)
{
    /* This method is a mistake, let's keep it for binary compatibility as a
     * no-op. */
    Q_UNUSED(refCount);
}

qint32 IdentityInfo::refCount() const
{
    return impl->refCount();
}

} //namespace SignOn
