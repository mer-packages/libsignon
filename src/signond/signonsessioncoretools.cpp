/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2011 Nokia Corporation.
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

#include "signonsessioncoretools.h"

#include <QDebug>
#include "signond-common.h"

using namespace SignonDaemonNS;

QVariantMap SignonDaemonNS::mergeVariantMaps(const QVariantMap &map1,
                                             const QVariantMap &map2)
{
    if (map1.isEmpty()) return map2;
    if (map2.isEmpty()) return map1;

    QVariantMap map = map1;
    //map2 values will overwrite map1 values for the same keys.
    QMapIterator<QString, QVariant> it(map2);
    while (it.hasNext()) {
        it.next();
        if (map.contains(it.key()))
            map.remove(it.key());
    }
    return map.unite(map2);
}

/* --------------------- StoreOperation ---------------------- */

StoreOperation::StoreOperation(const StoreType type):
    m_storeType(type)
{
}

StoreOperation::StoreOperation(const StoreOperation &src):
    m_storeType(src.m_storeType),
    m_info(src.m_info),
    m_authMethod(src.m_authMethod),
    m_blobData(src.m_blobData)
{
}

StoreOperation::~StoreOperation()
{
}

/* --------------------- RequestData ---------------------- */

RequestData::RequestData(const QDBusConnection &conn,
                         const QDBusMessage &msg,
                         const QVariantMap &params,
                         const QString &mechanism,
                         const QString &cancelKey):
    m_conn(conn),
    m_msg(msg),
    m_params(params),
    m_mechanism(mechanism),
    m_cancelKey(cancelKey)
{
}

RequestData::RequestData(const RequestData &other):
    m_conn(other.m_conn),
    m_msg(other.m_msg),
    m_params(other.m_params),
    m_mechanism(other.m_mechanism),
    m_cancelKey(other.m_cancelKey)
{
}

RequestData::~RequestData()
{
}
