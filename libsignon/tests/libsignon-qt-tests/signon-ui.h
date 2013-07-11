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

#ifndef SIGNON_UI_H
#define SIGNON_UI_H

#include "SignOn/signonerror.h"

#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusMessage>
#include <QObject>
#include <QVariantMap>

class SignOnUI: public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.nokia.singlesignonui")

public:
    SignOnUI(QDBusConnection connection, QObject *parent = 0);
    ~SignOnUI();

    /*!
     * Add an artificial delay to the queryDialog() replies.
     */
    void setDelay(int seconds) { m_delay = seconds * 1000; }

    /*!
     * Get the parameters sent with the last request.
     */
    QVariantMap parameters() const { return m_parameters; }

    /*!
     * Get the client data.
     */
    QVariantMap clientData() const;

    /*!
     * Get the authentication method.
     */
    QString method() const;

    /*!
     * Get the authentication mechanism.
     */
    QString mechanism() const;

    /*!
     * Set the password which will be returned to the next query.
     */
    void setPassword(const QString &password) { m_replyPassword = password; }

    /*!
     * Get the password used by SignOnUI.
     */
    QString password() const { return m_replyPassword; }

public Q_SLOTS:
    Q_NOREPLY void cancelUiRequest(const QString &requestId);
    QVariantMap queryDialog(const QVariantMap &parameters);
    QVariantMap refreshDialog(const QVariantMap &newParameters);

private Q_SLOTS:
    void processQueryDialog();

private:
    QDBusMessage m_reply;
    QDBusConnection m_connection;
    QVariantMap m_parameters;
    QString m_replyPassword;
    int m_delay;
};

#endif // SIGNON_UI_H
