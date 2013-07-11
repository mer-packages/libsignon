/*
 * Copied from
 * https://codereview.qt-project.org/cat/29453%2C6%2Csrc/network/kernel/qnetworkproxy_libproxy.cpp%5E0
 *
 * With minor modifications by
 * Alberto Mardegan <alberto.mardegan@canonical.com>
 */

/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "my-network-proxy-factory.h"

#include <QNetworkProxy>

#include <QtCore/QByteArray>
#include <QtCore/QUrl>

#include <proxy.h>

class QLibProxyWrapper
{
public:
    QLibProxyWrapper()
        : factory(px_proxy_factory_new())
    {
    }

    ~QLibProxyWrapper()
    {
        px_proxy_factory_free(factory);
    }

    QList<QUrl> getProxies(const QUrl &url);

private:
    pxProxyFactory *factory;
};

Q_GLOBAL_STATIC(QLibProxyWrapper, libProxyWrapper);

/*
    Gets the list of proxies from libproxy, converted to QUrl list.
    Thread safe, according to libproxy documentation.
*/
QList<QUrl> QLibProxyWrapper::getProxies(const QUrl &url)
{
    QList<QUrl> ret;

    if (factory) {
        char **proxies = px_proxy_factory_get_proxies(factory, url.toEncoded());
        if (proxies) {
            for (int i = 0; proxies[i]; i++) {
                ret.append(QUrl::fromEncoded(proxies[i]));
                free(proxies[i]);
            }
            free(proxies);
        }
    }

    return ret;
}

QList<QNetworkProxy> MyNetworkProxyFactory::queryProxy(const QNetworkProxyQuery &query)
{
    QList<QNetworkProxy> proxyList;

    QUrl queryUrl;
    QNetworkProxy::Capabilities requiredCapabilities(0);
    switch (query.queryType()) {
    //URL requests are directly supported by libproxy
    case QNetworkProxyQuery::UrlRequest:
        queryUrl = query.url();
        break;
    // fake URLs to get libproxy to tell us the SOCKS proxy
    case QNetworkProxyQuery::TcpSocket:
        queryUrl.setScheme(QLatin1String("tcp"));
        queryUrl.setHost(query.peerHostName());
        queryUrl.setPort(query.peerPort());
        requiredCapabilities |= QNetworkProxy::TunnelingCapability;
        break;
    case QNetworkProxyQuery::UdpSocket:
        queryUrl.setScheme(QLatin1String("udp"));
        queryUrl.setHost(query.peerHostName());
        queryUrl.setPort(query.peerPort());
        requiredCapabilities |= QNetworkProxy::UdpTunnelingCapability;
        break;
    default:
        proxyList.append(QNetworkProxy(QNetworkProxy::NoProxy));
        return proxyList;
    }

    QList<QUrl> rawProxies = libProxyWrapper()->getProxies(queryUrl);

    bool haveDirectConnection = false;
    foreach (const QUrl& url, rawProxies) {
        QNetworkProxy::ProxyType type;
        if (url.scheme() == QLatin1String("http")) {
            type = QNetworkProxy::HttpProxy;
        } else if (url.scheme() == QLatin1String("socks")
              || url.scheme() == QLatin1String("socks5")) {
            type = QNetworkProxy::Socks5Proxy;
        } else if (url.scheme() == QLatin1String("ftp")) {
            type = QNetworkProxy::FtpCachingProxy;
        } else if (url.scheme() == QLatin1String("direct")) {
            type = QNetworkProxy::NoProxy;
            haveDirectConnection = true;
        } else {
            continue; //unsupported proxy type e.g. socks4
        }

        QNetworkProxy proxy(type,
            url.host(),
            url.port(),
            url.userName(),
            url.password());

        if ((proxy.capabilities() & requiredCapabilities) == requiredCapabilities)
            proxyList.append(proxy);
    }

    // fallback is direct connection
    if (proxyList.isEmpty() || !haveDirectConnection)
        proxyList.append(QNetworkProxy(QNetworkProxy::NoProxy));

    return proxyList;
}
