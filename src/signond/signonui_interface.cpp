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

#include "signonui_interface.h"
#include "signond-common.h"

#include "SignOn/uisessiondata_priv.h"

/*
 * Implementation of interface class SignonUiAdaptor
 */
SignonUiAdaptor::SignonUiAdaptor(const QString &service,
                                 const QString &path,
                                 const QDBusConnection &connection,
                                 QObject *parent):
    QDBusAbstractInterface(service, path, staticInterfaceName(),
                           connection, parent)
{
}

SignonUiAdaptor::~SignonUiAdaptor()
{
}

/*
 * Open a new dialog
 * */

QDBusPendingCall SignonUiAdaptor::queryDialog(const QVariantMap &parameters)
{
    QList<QVariant> argumentList;
    argumentList << parameters;
    return callWithArgumentListAndBigTimeout(QLatin1String("queryDialog"),
                                             argumentList);
}


/*
 * update the existing dialog
 * */
QDBusPendingCall SignonUiAdaptor::refreshDialog(const QVariantMap &parameters)
{
    QList<QVariant> argumentList;
    argumentList << parameters;
    return callWithArgumentListAndBigTimeout(QLatin1String("refreshDialog"),
                                             argumentList);
}


/*
 * cancel dialog request
 * */
void SignonUiAdaptor::cancelUiRequest(const QString &requestId)
{
    QList<QVariant> argumentList;
    argumentList << requestId;
    callWithArgumentList(QDBus::NoBlock, QLatin1String("cancelUiRequest"),
                         argumentList);
}

/*
 * Remove any data associated with the given identity.
 * */
QDBusPendingCall SignonUiAdaptor::removeIdentityData(quint32 id)
{
    QList<QVariant> argumentList;
    argumentList << id;
    return asyncCallWithArgumentList(QLatin1String("removeIdentityData"),
                                     argumentList);
}

QDBusPendingCall
SignonUiAdaptor::callWithArgumentListAndBigTimeout(const QString &method,
                                                   const QList<QVariant> &args)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(service(),
                                                      path(),
                                                      interface(),
                                                      method);
    if (!args.isEmpty())
        msg.setArguments(args);
    return connection().asyncCall(msg, SIGNOND_MAX_TIMEOUT);
}
