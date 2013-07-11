/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
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
#ifndef SIGNONAUTHSESSIONADAPTOR_H_
#define SIGNONAUTHSESSIONADAPTOR_H_

#include <QtCore>
#include <QtDBus>

#include "signond-common.h"
#include "signonauthsession.h"

namespace SignonDaemonNS {

class SignonAuthSessionAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface",
                "com.google.code.AccountsSSO.SingleSignOn.AuthSession")

public:
    SignonAuthSessionAdaptor(SignonAuthSession *parent);
    virtual ~SignonAuthSessionAdaptor();

    inline SignonAuthSession *parent() const {
        return static_cast<SignonAuthSession *>(QObject::parent());
    }

private:
    void errorReply(const QString &name, const QString &message);

public Q_SLOTS:
    QStringList queryAvailableMechanisms(const QStringList &wantedMechanisms);
    QVariantMap process(const QVariantMap &sessionDataVa,
                        const QString &mechanism);

    Q_NOREPLY void cancel();
    Q_NOREPLY void setId(quint32 id);
    Q_NOREPLY void objectUnref();

Q_SIGNALS:
    void stateChanged(int state, const QString &message);
    void unregistered();
};

} //namespace SignonDaemonNS

#endif /* SIGNONAUTHSESSIONADAPTOR_H_ */
