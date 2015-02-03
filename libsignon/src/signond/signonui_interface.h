/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2013 Canonical Ltd.
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


#ifndef SIGNONUI_INTERFACE_H_
#define SIGNONUI_INTERFACE_H_

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include <QtDBus/QDBusPendingCall>

/*
 * Proxy class for interface com.nokia.singlesignonui
 */
class SignonUiAdaptor: public QDBusAbstractInterface
{
    Q_OBJECT

    friend class SignonSecureStorageUiAdaptor;

public:
    static inline const char *staticInterfaceName() {
        return "com.nokia.singlesignonui";
    }

public:

    SignonUiAdaptor(const QString &service,
                    const QString &path,
                    const QDBusConnection &connection,
                    QObject *parent = 0);
    ~SignonUiAdaptor();

public Q_SLOTS: // METHODS
    QDBusPendingCall queryDialog(const QVariantMap &parameters);
    QDBusPendingCall refreshDialog(const QVariantMap &parameters);

    void cancelUiRequest(const QString &requestId);
    QDBusPendingCall removeIdentityData(quint32 id);

protected:
    QDBusPendingCall callWithArgumentListAndBigTimeout(
        const QString &method,
        const QList<QVariant> &args = QList<QVariant>());
};

#endif
