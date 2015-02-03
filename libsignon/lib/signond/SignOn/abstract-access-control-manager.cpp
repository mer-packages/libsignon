/*
 * This file is part of signon
 *
 * Copyright (C) 2011 Intel Corporation.
 * Copyright (C) 2013 Canonical Ltd.
 *
 * Contact: Elena Reshetova <elena.reshetova@intel.com>
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

#include "abstract-access-control-manager.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QSharedData>

using namespace SignOn;

namespace SignOn {

class AccessRequestData: public QSharedData
{
public:
    AccessRequestData();
    AccessRequestData(const AccessRequestData &other);
    ~AccessRequestData() {};

    QDBusConnection m_connection;
    QDBusMessage m_message;
    quint32 m_identity;
};

} // namespace

AccessRequestData::AccessRequestData():
    m_connection(QLatin1String("invalid")),
    m_message(),
    m_identity(0)
{
}

AccessRequestData::AccessRequestData(const AccessRequestData &other):
    QSharedData(other),
    m_connection(other.m_connection),
    m_message(other.m_message),
    m_identity(other.m_identity)
{
}

AccessRequest::AccessRequest():
    d(new AccessRequestData)
{
}

AccessRequest::AccessRequest(const AccessRequest &other):
    d(other.d)
{
}

AccessRequest::~AccessRequest()
{
}

void AccessRequest::setPeer(const QDBusConnection &connection,
                            const QDBusMessage &message)
{
    d->m_connection = connection;
    d->m_message = message;
}

const QDBusConnection &AccessRequest::peerConnection() const
{
    return d->m_connection;
}

const QDBusMessage &AccessRequest::peerMessage() const
{
    return d->m_message;
}

void AccessRequest::setIdentity(quint32 id)
{
    d->m_identity = id;
}

quint32 AccessRequest::identity() const
{
    return d->m_identity;
}

namespace SignOn {

class AccessReplyPrivate
{
public:
    inline AccessReplyPrivate(const AccessRequest &request);

private:
    friend class AccessReply;
    AccessRequest m_request;
    bool m_isAccepted;
};

} // namespace

AccessReplyPrivate::AccessReplyPrivate(const AccessRequest &request):
    m_request(request),
    m_isAccepted(false)
{
}

AccessReply::AccessReply(const AccessRequest &request, QObject *parent):
    QObject(parent),
    d_ptr(new AccessReplyPrivate(request))
{
}

AccessReply::~AccessReply()
{
    delete d_ptr;
}

const AccessRequest &AccessReply::request() const
{
    Q_D(const AccessReply);
    return d->m_request;
}

bool AccessReply::isAccepted() const
{
    Q_D(const AccessReply);
    return d->m_isAccepted;
}

void AccessReply::accept()
{
    Q_D(AccessReply);
    d->m_isAccepted = true;
    Q_EMIT finished();
}

void AccessReply::decline()
{
    Q_D(AccessReply);
    d->m_isAccepted = false;
    Q_EMIT finished();
}

AbstractAccessControlManager::AbstractAccessControlManager(QObject *parent):
    QObject(parent)
{
}

AbstractAccessControlManager::~AbstractAccessControlManager()
{
}

QString AbstractAccessControlManager::keychainWidgetAppId()
{
    return QString();
}

bool AbstractAccessControlManager::isPeerAllowedToAccess(
                                       const QDBusConnection &peerConnection,
                                       const QDBusMessage &peerMessage,
                                       const QString &securityContext)
{
    Q_UNUSED(peerConnection);
    Q_UNUSED(peerMessage);
    Q_UNUSED(securityContext);
    return true;
}

QString AbstractAccessControlManager::appIdOfPeer(
                                       const QDBusConnection &peerConnection,
                                       const QDBusMessage &peerMessage)
{
    Q_UNUSED(peerConnection);
    Q_UNUSED(peerMessage);
    return QString();
}

AccessReply *
AbstractAccessControlManager::handleRequest(const AccessRequest &request)
{
    /* The default implementation creates a reply and accepts it */
    AccessReply *reply = new AccessReply(request, this);
    QMetaObject::invokeMethod(reply, "accept", Qt::QueuedConnection);
    return reply;
}
