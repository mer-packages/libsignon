/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2012 Canonical Ltd.
 *
 * Contact: Aurel Popirtac <ext-aurel.popirtac@nokia.com>
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

#ifndef SIGNONIDENTITY_H_
#define SIGNONIDENTITY_H_

#include <QtCore>
#include <QtDBus>

#include "pluginproxy.h"

#include "signond-common.h"
#include "signondaemon.h"
#include "signondisposable.h"
#include "signonidentityinfo.h"
#include "credentialsaccessmanager.h"

#include "signonui_interface.h"

namespace SignonDaemonNS {

/*!
 * @class SignonIdentity
 * Daemon side representation of identity.
 * @todo description.
 */
class SignonIdentity: public SignonDisposable, protected QDBusContext
{
    Q_OBJECT

    friend class SignonIdentityAdaptor;

    virtual ~SignonIdentity();

public:
    void destroy();
    static SignonIdentity *createIdentity(quint32 id, SignonDaemon *parent);
    quint32 id() const { return m_id; }

    SignonIdentityInfo queryInfo(bool &ok, bool queryPassword = true);
    quint32 storeCredentials(const SignonIdentityInfo &info);

public Q_SLOTS:
    quint32 requestCredentialsUpdate(const QString &message);
    QVariantMap getInfo();
    bool addReference(const QString &reference);
    bool removeReference(const QString &reference);
    bool verifyUser(const QVariantMap &params);
    bool verifySecret(const QString &secret);
    void remove();
    bool signOut();
    quint32 store(const QVariantMap &info);
    void queryUiSlot(QDBusPendingCallWatcher *call);
    void verifyUiSlot(QDBusPendingCallWatcher *call);
Q_SIGNALS:
    void unregistered();
    //TODO - split this into the 3 separate signals(updated, removed, signed out)
    void infoUpdated(int);

private:
    SignonIdentity(quint32 id, int timeout, SignonDaemon *parent);
    bool init();
    bool credentialsStored() const { return m_id > 0 ? true : false; }
    void queryUserPassword(const QVariantMap &params);

private:
    quint32 m_id;
    SignonUiAdaptor *m_signonui;
    SignonIdentityInfo *m_pInfo;
    SignonDaemon *m_pSignonDaemon;
    bool m_registered;
    QDBusMessage m_message;

}; //class SignonDaemon

} //namespace SignonDaemonNS

#endif /* SIGNONIDENTITY_H_ */
