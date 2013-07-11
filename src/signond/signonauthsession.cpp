/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 *
 * Conta Alberto Mardegan <alberto.mardegan@canonical.com>
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
#include "signonauthsession.h"
#include "signonauthsessionadaptor.h"

using namespace SignonDaemonNS;

SignonAuthSession::SignonAuthSession(quint32 id,
                                     const QString &method,
                                     pid_t ownerPid):
    m_id(id),
    m_method(method),
    m_registered(false),
    m_ownerPid(ownerPid)
{
    TRACE();

    static quint32 incr = 0;
    QString objectName = SIGNOND_DAEMON_OBJECTPATH +
        QLatin1String("/AuthSession_") + QString::number(incr++, 16);
    TRACE() << objectName;

    setObjectName(objectName);
}

SignonAuthSession::~SignonAuthSession()
{
    TRACE();

    if (m_registered)
    {
        emit unregistered();
        QDBusConnection connection(SIGNOND_BUS);
        connection.unregisterObject(objectName());
    }
}

QString SignonAuthSession::getAuthSessionObjectPath(const quint32 id,
                                                    const QString &method,
                                                    SignonDaemon *parent,
                                                    bool &supportsAuthMethod,
                                                    pid_t ownerPid)
{
    TRACE();
    supportsAuthMethod = true;
    SignonAuthSession* sas = new SignonAuthSession(id, method, ownerPid);

    QDBusConnection connection(SIGNOND_BUS);
    if (!connection.isConnected()) {
        TRACE() << "Cannot get DBUS object connected";
        delete sas;
        return QString();
    }

    (void)new SignonAuthSessionAdaptor(sas);
    QString objectName = sas->objectName();
    if (!connection.registerObject(sas->objectName(), sas,
                                   QDBusConnection::ExportAdaptors)) {
        TRACE() << "Object cannot be registered: " << objectName;
        delete sas;
        return QString();
    }

    SignonSessionCore *core = SignonSessionCore::sessionCore(id, method, parent);
    if (!core) {
        TRACE() << "Cannot retrieve proper tasks queue";
        supportsAuthMethod = false;
        delete sas;
        return QString();
    }

    sas->objectRegistered();
    sas->setParent(core);

    connect(core, SIGNAL(stateChanged(const QString&, int, const QString&)),
            sas, SLOT(stateChangedSlot(const QString&, int, const QString&)));

    TRACE() << "SignonAuthSession is created successfully: " << objectName;
    return objectName;
}

void SignonAuthSession::stopAllAuthSessions()
{
    SignonSessionCore::stopAllAuthSessions();
}

quint32 SignonAuthSession::id() const
{
    return m_id;
}

QString SignonAuthSession::method() const
{
    return m_method;
}

pid_t SignonAuthSession::ownerPid() const
{
    return m_ownerPid;
}

QStringList
SignonAuthSession::queryAvailableMechanisms(const QStringList &wantedMechanisms)
{
    return parent()->queryAvailableMechanisms(wantedMechanisms);
}

QVariantMap SignonAuthSession::process(const QVariantMap &sessionDataVa,
                                       const QString &mechanism)
{
    setDelayedReply(true);
    parent()->process(connection(),
                      message(),
                      sessionDataVa,
                      mechanism,
                      objectName());
    return QVariantMap();
}

void SignonAuthSession::cancel()
{
    TRACE();
    parent()->cancel(objectName());
}

void SignonAuthSession::setId(quint32 id)
{
    m_id = id;
    parent()->setId(id);
}

void SignonAuthSession::objectUnref()
{
    //TODO - remove the `objectUnref` functionality from the DBus API
    TRACE();
    cancel();

    if (m_registered) {
        QDBusConnection connection(SIGNOND_BUS);
        connection.unregisterObject(objectName());
        m_registered = false;
    }

    deleteLater();
}

void SignonAuthSession::stateChangedSlot(const QString &sessionKey,
                                         int state,
                                         const QString &message)
{
    TRACE();

    if (sessionKey == objectName())
        emit stateChanged(state, message);
}

void SignonAuthSession::objectRegistered()
{
    m_registered = true;
}
