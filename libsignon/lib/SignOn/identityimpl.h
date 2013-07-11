/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
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
#ifndef IDENTITYIMPL_H
#define IDENTITYIMPL_H

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMetaMethod>
#include <QQueue>
#include <QDBusObjectPath>

#include "libsignoncommon.h"
#include "identity.h"
#include "dbusinterface.h"
#include "dbusoperationqueuehandler.h"

namespace SignOn {

class IdentityInfo;

/*!
 * @class IdentityImpl
 * Identity class implementation.
 * @sa Identity
 */
class IdentityImpl: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(IdentityImpl)

    friend class Identity;

    enum State {
        PendingRegistration = 0, /* Ongoing registration */
        NeedsRegistration,       /* Remote object not created or destroyed */
        NeedsUpdate,             /* Remote data changed */
        Removed,                 /* Removed from database */
        Ready                    /* Ready for querying locally cached data, or
                                    any remote operation */
    };

public:
    IdentityImpl(Identity *parent,
                 const quint32 id = 0);
    ~IdentityImpl();

    quint32 id() const;
    AuthSession *createSession(const QString &methodName, QObject *parent = 0);
    void destroySession(AuthSession *session);

public Q_SLOTS:
    void errorReply(const QDBusError &err);
    void storeCredentialsReply(const quint32 id);
    void removeReply();
    void addReferenceReply();
    void removeReferenceReply();
    void getInfoReply(const QVariantMap &infoData);
    void verifyUserReply(const bool valid);
    void verifySecretReply(const bool valid);
    void signOutReply();
    void infoUpdated(int);
    void removeObjectDestroyed();

private Q_SLOTS:
    void queryAvailableMethods();
    void requestCredentialsUpdate(const QString &message = QString());
    void storeCredentials(const IdentityInfo &info);
    void remove();
    void addReference(const QString &reference = QString());
    void removeReference(const QString &reference = QString());
    void queryInfo();
    void verifyUser(const QString &message = QString());
    void verifyUser(const QVariantMap &params);
    void verifySecret(const QString &secret);
    void signOut();
    void authSessionCancelReply(const SignOn::Error &err);
    void registerReply(const QDBusObjectPath &objectPath,
                       const QVariantMap &infoData);
    void registerReply(const QDBusObjectPath &objectPath);

private:
    void copyInfo(const IdentityInfo &info);
    bool sendRequest(const char *remoteMethod, const QList<QVariant> &args,
                     const char *replySlot, int timeout = -1);
    void updateState(State state);
    void checkConnection();

    bool sendRegisterRequest();
    void updateContents();
    void updateCachedData(const QVariantMap &infoData);
    void clearAuthSessionsCache();

private:
    Identity *m_parent;
    IdentityInfo *m_identityInfo;
    DBusOperationQueueHandler m_operationQueueHandler;

    /* Cache info in the storing case, so that if the storing succeeds, server
     * side does not have to send succesfully stored data over IPC channel.
     */
    IdentityInfo *m_tmpIdentityInfo;
    DBusInterface *m_DBusInterface;
    State m_state;
    QList<AuthSession *> m_authSessions;
    /* This flag allows the queryInfo() reply slot not to reply with the
     * 'info()' signal, but with the 'methodsAvailable()' signal, for the case
     * when the cached info is updated at a queryAvaialbleMethods() request.
     */
    bool m_infoQueried;
    /* Marks this Identity as the one which requested the sign out */
    bool m_signOutRequestedByThisIdentity;
};

}  // namespace SignOn

#endif /* IDENTITYIMPL_H */
