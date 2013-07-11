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
#include "signonidentityinfo.h"

#include <QBuffer>
#include <QDataStream>
#include <QDebug>

namespace SignonDaemonNS {

SignonIdentityInfo::SignonIdentityInfo():
    m_id(0),
    m_userName(QString()),
    m_password(QString()),
    m_storePassword(false),
    m_caption(QString()),
    m_methods(QMap<QString, QStringList>()),
    m_realms(QStringList()),
    m_accessControlList(QStringList()),
    m_ownerList(QStringList()),
    m_type(0),
    m_refCount(0),
    m_validated(false),
    m_isUserNameSecret(false)
{
}

SignonIdentityInfo::SignonIdentityInfo(const QVariantMap &info):
    m_id(0),
    m_userName(QString()),
    m_password(QString()),
    m_storePassword(false),
    m_caption(QString()),
    m_methods(QMap<QString, QStringList>()),
    m_realms(QStringList()),
    m_accessControlList(QStringList()),
    m_ownerList(QStringList()),
    m_type(0),
    m_refCount(0),
    m_validated(false),
    m_isUserNameSecret(false)
{
    m_id = info.value(SIGNOND_IDENTITY_INFO_ID).toInt();
    m_userName = info.value(SIGNOND_IDENTITY_INFO_USERNAME).toString();
    m_password = info.value(SIGNOND_IDENTITY_INFO_SECRET).toString();
    m_storePassword = info.value(SIGNOND_IDENTITY_INFO_STORESECRET).toBool();
    m_caption = info.value(SIGNOND_IDENTITY_INFO_CAPTION).toString();
    m_methods =
        info.value(SIGNOND_IDENTITY_INFO_AUTHMETHODS).value<MethodMap>();

    m_realms = info.value(SIGNOND_IDENTITY_INFO_REALMS).toStringList();
    m_accessControlList = info.value(SIGNOND_IDENTITY_INFO_ACL).toStringList();
    m_ownerList = info.value(SIGNOND_IDENTITY_INFO_OWNER).toStringList();
    m_type = info.value(SIGNOND_IDENTITY_INFO_TYPE).toInt();
    m_refCount = info.value(SIGNOND_IDENTITY_INFO_REFCOUNT).toInt();
    m_validated = info.value(SIGNOND_IDENTITY_INFO_VALIDATED).toBool();
}

SignonIdentityInfo::SignonIdentityInfo(const quint32 id,
                                       const QString &userName,
                                       const QString &password,
                                       const bool storePassword,
                                       const QString &caption,
                                       const MethodMap &methods,
                                       const QStringList &realms,
                                       const QStringList &accessControlList,
                                       const QStringList &ownerList,
                                       int type,
                                       int refCount,
                                       bool validated):
    m_id(id),
    m_userName(userName),
    m_password(password),
    m_storePassword(storePassword),
    m_caption(caption),
    m_methods(methods),
    m_realms(realms),
    m_accessControlList(accessControlList),
    m_ownerList(ownerList),
    m_type(type),
    m_refCount(refCount),
    m_validated(validated),
    m_isUserNameSecret(false)
{
}

const QList<QVariant> SignonIdentityInfo::toVariantList()
{
    QList<QVariant> list;
    list << m_id
         << m_userName
         << m_password
         << m_caption
         << m_realms
         << QVariant::fromValue(m_methods)
         << m_accessControlList
         << m_type
         << m_refCount
         << m_validated
         << m_isUserNameSecret;

    return list;
}

const QVariantMap SignonIdentityInfo::toMap() const
{
    QVariantMap values;
    values.insert(SIGNOND_IDENTITY_INFO_ID, m_id);
    values.insert(SIGNOND_IDENTITY_INFO_USERNAME, m_userName);
    values.insert(SIGNOND_IDENTITY_INFO_SECRET, m_password);
    values.insert(SIGNOND_IDENTITY_INFO_CAPTION, m_caption);
    values.insert(SIGNOND_IDENTITY_INFO_REALMS, m_realms);
    values.insert(SIGNOND_IDENTITY_INFO_AUTHMETHODS,
                  QVariant::fromValue(m_methods));
    values.insert(SIGNOND_IDENTITY_INFO_ACL, m_accessControlList);
    values.insert(SIGNOND_IDENTITY_INFO_TYPE, m_type);
    values.insert(SIGNOND_IDENTITY_INFO_REFCOUNT, m_refCount);
    values.insert(SIGNOND_IDENTITY_INFO_VALIDATED, m_validated);
    values.insert(SIGNOND_IDENTITY_INFO_USERNAME_IS_SECRET,
                  m_isUserNameSecret);
    return values;
}

bool SignonIdentityInfo::operator==(const SignonIdentityInfo &other) const
{
    //do not care about list element order
    SignonIdentityInfo me = *this;
    SignonIdentityInfo you = other;
    me.m_realms.sort();
    you.m_realms.sort();
    me.m_accessControlList.sort();
    you.m_accessControlList.sort();
    QMapIterator<QString, QStringList> it(me.m_methods);
    while (it.hasNext()) {
        it.next();
        QStringList list1 = it.value();
        QStringList list2 = you.m_methods.value(it.key());
        list1.sort();
        list2.sort();
        if (list1 != list2) return false;
    }

    return (m_id == other.m_id)
            && (m_userName == other.m_userName)
            && (m_password == other.m_password)
            && (m_caption == other.m_caption)
            && (me.m_realms ==you.m_realms)
            && (me.m_accessControlList == you.m_accessControlList)
            && (m_type == other.m_type)
            && (m_validated == other.m_validated);
}

bool SignonIdentityInfo::checkMethodAndMechanism(const QString &method,
                                                 const QString &mechanism,
                                                 QString &allowedMechanism)
{
    // If no methods have been specified for an identity assume anything goes
    if (m_methods.isEmpty())
        return true;

    if (!m_methods.contains(method))
        return false;

    MechanismsList mechs = m_methods[method];
    // If no mechanisms have been specified for a method, assume anything goes
    if (mechs.isEmpty())
        return true;

    if (mechs.contains(mechanism)) {
        allowedMechanism = mechanism;
        return true;
    }

    /* in the case of SASL authentication (and possibly others),
     * mechanism can be a list of strings, separated by a space;
     * therefore, let's split the list first, and see if any of the
     * mechanisms is allowed.
     */
    QStringList mechanisms =
        mechanism.split(QLatin1Char(' '), QString::SkipEmptyParts);

    /* if the list is empty of it has only one element, then we already know
     * that it didn't pass the previous checks */
    if (mechanisms.size() <= 1)
        return false;

    QStringList allowedMechanisms;
    foreach (const QString &mech, mechanisms) {
        if (mechs.contains(mech))
            allowedMechanisms.append(mech);
    }
    if (allowedMechanisms.isEmpty())
        return false;

    allowedMechanism = allowedMechanisms.join(QLatin1String(" "));
    return true;
}

SignonIdentityInfo &
SignonIdentityInfo::operator=(const SignonIdentityInfo &other)
{

    m_id = other.m_id;
    m_userName = other.m_userName;
    m_password = other.m_password ;
    m_storePassword = other.m_storePassword;
    m_caption = other.m_caption;
    m_realms = other.m_realms;
    m_accessControlList = other.m_accessControlList;
    m_ownerList = other.m_ownerList;
    m_type = other.m_type;
    m_refCount = other.m_refCount;
    m_validated = other.m_validated;
    m_methods = other.m_methods;
    return *this;
}

} //namespace SignonDaemonNS
