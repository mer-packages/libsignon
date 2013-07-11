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

#include "signon-ui.h"
#include "SignOn/uisessiondata.h"
#include "SignOn/uisessiondata_priv.h"

#include <QDBusArgument>
#include <QDebug>
#include <QTimer>

static const char serviceName[] = "com.nokia.singlesignonui";
static const char objectPath[] = "/SignonUi";

SignOnUI::SignOnUI(QDBusConnection connection, QObject *parent):
    QObject(parent),
    QDBusContext(),
    m_connection(connection),
    m_replyPassword("SSOUI default password"),
    m_delay(0)
{
    connection.registerObject(QLatin1String(objectPath), this,
                              QDBusConnection::ExportAllContents);
    connection.registerService(QLatin1String(serviceName));
}

SignOnUI::~SignOnUI()
{
    m_connection.unregisterService(QLatin1String(serviceName));
    m_connection.unregisterObject(QLatin1String(objectPath));
}

QVariantMap SignOnUI::clientData() const
{
    if (!m_parameters.contains(SSOUI_KEY_CLIENT_DATA)) {
        return QVariantMap();
    }

    QVariant variant = m_parameters[SSOUI_KEY_CLIENT_DATA];
    return (variant.type() == QVariant::Map) ?
        variant.toMap() :
        qdbus_cast<QVariantMap>(variant.value<QDBusArgument>());
}

QString SignOnUI::method() const
{
    return m_parameters.value(SSOUI_KEY_METHOD).toString();
}

QString SignOnUI::mechanism() const
{
    return m_parameters.value(SSOUI_KEY_MECHANISM).toString();
}

void SignOnUI::cancelUiRequest(const QString &requestId)
{
    qDebug() << Q_FUNC_INFO << requestId;
}

QVariantMap SignOnUI::queryDialog(const QVariantMap &parameters)
{
    qDebug() << Q_FUNC_INFO << parameters;

    m_parameters = parameters;

    QVariantMap result = parameters;
    if (parameters.value(SSOUI_KEY_QUERYPASSWORD, false).toBool()) {
        result.remove(SSOUI_KEY_QUERYPASSWORD);
        result.insert(SSOUI_KEY_PASSWORD, m_replyPassword);
    }
    result.insert(SSOUI_KEY_ERROR, SignOn::QUERY_ERROR_NONE);

    m_reply = message().createReply(result);

    setDelayedReply(true);

    QTimer::singleShot(m_delay, this, SLOT(processQueryDialog()));
    return QVariantMap();
}

QVariantMap SignOnUI::refreshDialog(const QVariantMap &parameters)
{
    qDebug() << Q_FUNC_INFO << parameters;
    return parameters;
}

void SignOnUI::processQueryDialog()
{
    m_connection.send(m_reply);
}
