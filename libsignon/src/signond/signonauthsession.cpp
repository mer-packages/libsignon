/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2013 Canonical Ltd.
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
    m_ownerPid(ownerPid)
{
    TRACE();

    (void)new SignonAuthSessionAdaptor(this);

    static quint32 incr = 0;
    QString objectName = SIGNOND_DAEMON_OBJECTPATH +
        QLatin1String("/AuthSession_") + QString::number(incr++, 16);
    TRACE() << objectName;

    setObjectName(objectName);
}

SignonAuthSession::~SignonAuthSession()
{
    Q_EMIT unregistered();
    TRACE();
}

SignonAuthSession *SignonAuthSession::createAuthSession(const quint32 id,
                                                        const QString &method,
                                                        SignonDaemon *parent,
                                                        pid_t ownerPid)
{
    TRACE();
    SignonAuthSession *sas = new SignonAuthSession(id, method, ownerPid);

    SignonSessionCore *core = SignonSessionCore::sessionCore(id, method, parent);
    if (!core) {
        TRACE() << "Cannot retrieve proper tasks queue";
        delete sas;
        return NULL;
    }

    sas->setParent(core);

    connect(core, SIGNAL(stateChanged(const QString&, int, const QString&)),
            sas, SLOT(stateChangedSlot(const QString&, int, const QString&)));

    TRACE() << "SignonAuthSession created successfully:" << sas->objectName();
    return sas;
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
