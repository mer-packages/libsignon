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
#include "signond-common.h"
#include "signonidentityinfo.h"

#include <QBuffer>
#include <QDBusArgument>
#include <QDataStream>
#include <QDebug>

namespace SignonDaemonNS {

SignonIdentityInfo::SignonIdentityInfo()
{
}

SignonIdentityInfo::SignonIdentityInfo(const QVariantMap &info)
{
    /* We need to expand any QDBusArguments which might be present, since
     * the map is likely to be coming from QDBus. */
    QVariantMap::const_iterator i;
    for (i = info.constBegin(); i != info.constEnd(); i++) {
        if (qstrcmp(i.value().typeName(), "QDBusArgument") == 0) {
            QDBusArgument container = i.value().value<QDBusArgument>();

            if (i.key() == SIGNOND_IDENTITY_INFO_AUTHMETHODS) {
                MethodMap methodMap = qdbus_cast<MethodMap>(container);
                setMethods(methodMap);
            } else {
                BLAME() << "Found unsupported QDBusArgument in key" << i.key();
            }
        } else {
            insert(i.key(), i.value());
        }
    }
}

const QVariantMap SignonIdentityInfo::toMap() const
{
    return *this;
}

bool SignonIdentityInfo::checkMethodAndMechanism(const QString &method,
                                                 const QString &mechanism,
                                                 QString &allowedMechanism)
{
    MethodMap methodMap = methods();

    // If no methods have been specified for an identity assume anything goes
    if (methodMap.isEmpty())
        return true;

    if (!methodMap.contains(method))
        return false;

    MechanismsList mechs = methodMap[method];
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

} //namespace SignonDaemonNS
