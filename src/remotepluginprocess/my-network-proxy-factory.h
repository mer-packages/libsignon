/*
 * This file is part of signon
 *
 * Copyright (C) 2012 Canonical Ltd.
 *
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

#ifndef SIGNON_MY_NETWORK_PROXY_FACTORY_H
#define SIGNON_MY_NETWORK_PROXY_FACTORY_H

#include <QNetworkProxyFactory>

class MyNetworkProxyFactory: public QNetworkProxyFactory
{
public:
    // reimplemented virtual methods
    QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery &query =
                                    QNetworkProxyQuery());
};

#endif // SIGNON_MY_NETWORK_PROXY_FACTORY_H
