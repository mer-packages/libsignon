/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2012-2013 Canonical Ltd.
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

#ifndef SIGNONAUTHSESSION_H_
#define SIGNONAUTHSESSION_H_

#include <QtCore>
#include <QtDBus>

/*
 * TODO: remove invocation of plugin operations into the main signond process
 */
#include "signond-common.h"
#include "signonsessioncore.h"

using namespace SignOn;

namespace SignonDaemonNS {

/*!
 * @class SignonAuthSession
 * Daemon side representation of authentication session.
 * @todo description.
 */
class SignonAuthSession: public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    inline SignonSessionCore *parent() const
    {
        return static_cast<SignonSessionCore *>(QObject::parent());
    }

    friend class SignonAuthSessionAdaptor;

    static SignonAuthSession *createAuthSession(const quint32 id,
                                                const QString &method,
                                                SignonDaemon *parent,
                                                pid_t ownerPid);
    static void stopAllAuthSessions();
    quint32 id() const;
    QString method() const;
    pid_t ownerPid() const;

public Q_SLOTS:
    QStringList queryAvailableMechanisms(const QStringList &wantedMechanisms);
    QVariantMap process(const QVariantMap &sessionDataVa,
                        const QString &mechanism);
    void cancel();
    void setId(quint32 id);
    void objectUnref();

Q_SIGNALS:
    void stateChanged(int state, const QString &message);
    void unregistered();

private Q_SLOTS:
    void stateChangedSlot(const QString &sessionKey,
                          int state,
                          const QString &message);

protected:
    SignonAuthSession(quint32 id, const QString &method, pid_t ownerPid);
    virtual ~SignonAuthSession();

private:
    quint32 m_id;
    QString m_method;
    pid_t m_ownerPid;

    Q_DISABLE_COPY(SignonAuthSession)
}; //class SignonDaemon

} //namespace SignonDaemonNS

#endif //SIGNONAUTHSESSION_H_
