/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2011 Intel Corporation.
 * Copyright (C) 2013 Canonical Ltd.
 *
 * Contact: Aurel Popirtac <ext-aurel.popirtac@nokia.com>
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
 * Contact: Jussi Laako <jussi.laako@linux.intel.com>
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

#include <iostream>
#include <QVariantMap>

#include "signond-common.h"
#include "signonidentity.h"
#include "signonui_interface.h"
#include "SignOn/uisessiondata.h"
#include "SignOn/uisessiondata_priv.h"
#include "signoncommon.h"

#include "accesscontrolmanagerhelper.h"
#include "signonidentityadaptor.h"

#define SIGNON_RETURN_IF_CAM_UNAVAILABLE(_ret_arg_) do {                          \
        if (!(CredentialsAccessManager::instance()->credentialsSystemOpened())) { \
            sendErrorReply(internalServerErrName, \
                           internalServerErrStr + \
                           QLatin1String("Could not access Signon Database."));\
            return _ret_arg_;           \
        }                               \
    } while(0)

namespace SignonDaemonNS {

const QString internalServerErrName = SIGNOND_INTERNAL_SERVER_ERR_NAME;
const QString internalServerErrStr = SIGNOND_INTERNAL_SERVER_ERR_STR;

class PendingCallWatcherWithContext: public QDBusPendingCallWatcher
{
    Q_OBJECT

public:
    PendingCallWatcherWithContext(const QDBusPendingCall &call,
                                  SignonIdentity *parent):
        QDBusPendingCallWatcher(call, parent),
        m_connection(parent->connection()),
        m_message(parent->message())
    {
    }

    PendingCallWatcherWithContext(const QDBusPendingCall &call,
                                  const QDBusConnection &connection,
                                  const QDBusMessage &message,
                                  SignonIdentity *parent):
        QDBusPendingCallWatcher(call, parent),
        m_connection(connection),
        m_message(message)
    {
    }

    const QDBusConnection &connection() const { return m_connection; }
    const QDBusMessage &message() const { return m_message; }

private:
    QDBusConnection m_connection;
    QDBusMessage m_message;
};

SignonIdentity::SignonIdentity(quint32 id, int timeout,
                               SignonDaemon *parent):
    SignonDisposable(timeout, parent),
    m_pInfo(NULL)
{
    m_id = id;

    (void)new SignonIdentityAdaptor(this);

    /*
     * creation of unique name for the given identity
     * */
    static quint32 incr = 0;
    QString objectName = SIGNOND_DAEMON_OBJECTPATH + QLatin1String("/Identity_")
                         + QString::number(incr++, 16);
    setObjectName(objectName);

    m_signonui = new SignonUiAdaptor(SIGNON_UI_SERVICE,
                                     SIGNON_UI_DAEMON_OBJECTPATH,
                                     QDBusConnection::sessionBus(),
                                     this);

    /* Watch for credential updates happening outside of this object (this can
     * happen on request of authentication plugins) */
    CredentialsDB *db = CredentialsAccessManager::instance()->credentialsDB();
    QObject::connect(db, SIGNAL(credentialsUpdated(quint32)),
                     this, SLOT(onCredentialsUpdated(quint32)));
}

SignonIdentity::~SignonIdentity()
{
    emit unregistered();

    delete m_signonui;
    delete m_pInfo;
}

SignonIdentity *SignonIdentity::createIdentity(quint32 id, SignonDaemon *parent)
{
    return new SignonIdentity(id, parent->identityTimeout(), parent);
}

void SignonIdentity::destroy()
{
    /* Emitting the destroyed signal makes QDBusConnection unregister the
     * object */
    Q_EMIT destroyed();
    deleteLater();
}

SignonIdentityInfo SignonIdentity::queryInfo(bool &ok, bool queryPassword)
{
    ok = true;

    bool needLoadFromDB = true;
    if (m_pInfo) {
        needLoadFromDB = false;
        if (queryPassword && m_pInfo->password().isEmpty()) {
            needLoadFromDB = true;
        }
    }

    if (needLoadFromDB) {
        if (m_pInfo != 0) {
            delete m_pInfo;
        }

        CredentialsDB *db =
            CredentialsAccessManager::instance()->credentialsDB();
        m_pInfo = new SignonIdentityInfo(db->credentials(m_id, queryPassword));

        if (db->lastError().isValid()) {
            ok = false;
            delete m_pInfo;
            m_pInfo = NULL;
            return SignonIdentityInfo();
        }
    }

    /* Make sure that we clear the password, if the caller doesn't need it */
    SignonIdentityInfo info = *m_pInfo;
    if (!queryPassword) {
        info.setPassword(QString());
    }
    return info;
}

bool SignonIdentity::addReference(const QString &reference)
{
    TRACE() << "addReference: " << reference;

    SIGNON_RETURN_IF_CAM_UNAVAILABLE(false);

    CredentialsDB *db = CredentialsAccessManager::instance()->credentialsDB();
    if (db == NULL) {
        BLAME() << "NULL database handler object.";
        return false;
    }
    const QDBusContext &context = static_cast<QDBusContext>(*this);
    QString appId =
        AccessControlManagerHelper::instance()->appIdOfPeer(
                                                   context.connection(),
                                                   context.message());
    keepInUse();
    return db->addReference(m_id, appId, reference);
}

bool SignonIdentity::removeReference(const QString &reference)
{
    TRACE() << "removeReference: " << reference;

    SIGNON_RETURN_IF_CAM_UNAVAILABLE(false);

    CredentialsDB *db = CredentialsAccessManager::instance()->credentialsDB();
    if (db == NULL) {
        BLAME() << "NULL database handler object.";
        return false;
    }
    const QDBusContext &context = static_cast<QDBusContext>(*this);
    QString appId =
        AccessControlManagerHelper::instance()->appIdOfPeer(
                                                   context.connection(),
                                                   context.message());
    keepInUse();
    return db->removeReference(m_id, appId, reference);
}

quint32 SignonIdentity::requestCredentialsUpdate(const QString &displayMessage)
{
    SIGNON_RETURN_IF_CAM_UNAVAILABLE(SIGNOND_NEW_IDENTITY);

    bool ok;
    SignonIdentityInfo info = queryInfo(ok, false);

    if (!ok) {
        BLAME() << "Identity not found.";
        sendErrorReply(SIGNOND_IDENTITY_NOT_FOUND_ERR_NAME,
                       SIGNOND_IDENTITY_NOT_FOUND_ERR_STR);
        return SIGNOND_NEW_IDENTITY;
    }
    if (!info.storePassword()) {
        BLAME() << "Password cannot be stored.";
        sendErrorReply(SIGNOND_STORE_FAILED_ERR_NAME,
                       SIGNOND_STORE_FAILED_ERR_STR);
        return SIGNOND_NEW_IDENTITY;
    }

    //delay dbus reply, ui interaction might take long time to complete
    setDelayedReply(true);

    //create ui request to ask password
    QVariantMap uiRequest;
    uiRequest.insert(SSOUI_KEY_QUERYPASSWORD, true);
    uiRequest.insert(SSOUI_KEY_USERNAME, info.userName());
    uiRequest.insert(SSOUI_KEY_MESSAGE, displayMessage);
    uiRequest.insert(SSOUI_KEY_CAPTION, info.caption());

    TRACE() << "Waiting for reply from signon-ui";
    PendingCallWatcherWithContext *watcher =
        new PendingCallWatcherWithContext(m_signonui->queryDialog(uiRequest),
                                          this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(queryUiSlot(QDBusPendingCallWatcher*)));

    setAutoDestruct(false);
    return 0;
}

QVariantMap SignonIdentity::getInfo()
{
    TRACE() << "QUERYING INFO";

    SIGNON_RETURN_IF_CAM_UNAVAILABLE(QVariantMap());

    bool ok;
    SignonIdentityInfo info = queryInfo(ok, false);

    if (!ok) {
        TRACE();
        sendErrorReply(SIGNOND_CREDENTIALS_NOT_AVAILABLE_ERR_NAME,
                       SIGNOND_CREDENTIALS_NOT_AVAILABLE_ERR_STR +
                       QLatin1String("Database querying error occurred."));
        return QVariantMap();
    }

    if (info.isNew()) {
        TRACE();
        sendErrorReply(SIGNOND_IDENTITY_NOT_FOUND_ERR_NAME,
                       SIGNOND_IDENTITY_NOT_FOUND_ERR_STR);
        return QVariantMap();
    }

    keepInUse();
    info.removeSecrets();
    return info.toMap();
}

void SignonIdentity::queryUserPassword(const QVariantMap &params,
                                       const QDBusConnection &connection,
                                       const QDBusMessage &message)
{
    TRACE() << "Waiting for reply from signon-ui";
    PendingCallWatcherWithContext *watcher =
        new PendingCallWatcherWithContext(m_signonui->queryDialog(params),
                                          connection, message, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this,
            SLOT(verifyUiSlot(QDBusPendingCallWatcher*)));

    setAutoDestruct(false);
}

bool SignonIdentity::verifyUser(const QVariantMap &params)
{
    SIGNON_RETURN_IF_CAM_UNAVAILABLE(false);

    bool ok;
    SignonIdentityInfo info = queryInfo(ok, true);

    if (!ok) {
        BLAME() << "Identity not found.";
        sendErrorReply(SIGNOND_IDENTITY_NOT_FOUND_ERR_NAME,
                       SIGNOND_IDENTITY_NOT_FOUND_ERR_STR);
        return false;
    }
    if (!info.storePassword() || info.password().isEmpty()) {
        BLAME() << "Password is not stored.";
        sendErrorReply(SIGNOND_CREDENTIALS_NOT_AVAILABLE_ERR_NAME,
                       SIGNOND_CREDENTIALS_NOT_AVAILABLE_ERR_STR);
        return false;
    }

    //delay dbus reply, ui interaction might take long time to complete
    setDelayedReply(true);

    //create ui request to ask password
    QVariantMap uiRequest;
    uiRequest.unite(params);
    uiRequest.insert(SSOUI_KEY_QUERYPASSWORD, true);
    uiRequest.insert(SSOUI_KEY_USERNAME, info.userName());
    uiRequest.insert(SSOUI_KEY_CAPTION, info.caption());

    queryUserPassword(uiRequest, connection(), message());
    return false;
}

bool SignonIdentity::verifySecret(const QString &secret)
{
    SIGNON_RETURN_IF_CAM_UNAVAILABLE(false);

    bool ok;
    queryInfo(ok);
    if (!ok) {
        TRACE();
        sendErrorReply(SIGNOND_CREDENTIALS_NOT_AVAILABLE_ERR_NAME,
                       SIGNOND_CREDENTIALS_NOT_AVAILABLE_ERR_STR +
                       QLatin1String("Database querying error occurred."));
        return false;
    }

    CredentialsDB *db = CredentialsAccessManager::instance()->credentialsDB();
    bool ret = db->checkPassword(m_pInfo->id(), m_pInfo->userName(), secret);

    keepInUse();
    return ret;
}

void SignonIdentity::remove()
{
    SIGNON_RETURN_IF_CAM_UNAVAILABLE();

    CredentialsDB *db = CredentialsAccessManager::instance()->credentialsDB();
    if ((db == 0) || !db->removeCredentials(m_id)) {
        TRACE() << "Error occurred while inserting/updating credentials.";
        sendErrorReply(SIGNOND_REMOVE_FAILED_ERR_NAME,
                       SIGNOND_REMOVE_FAILED_ERR_STR +
                       QLatin1String("Database error occurred."));
        return;
    }
    setDelayedReply(true);
    setAutoDestruct(false);
    PendingCallWatcherWithContext *watcher =
        new PendingCallWatcherWithContext(m_signonui->removeIdentityData(m_id),
                                          this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(removeCompleted(QDBusPendingCallWatcher*)));
    keepInUse();
}

void SignonIdentity::removeCompleted(QDBusPendingCallWatcher *call)
{
    Q_ASSERT(call != NULL);

    setAutoDestruct(true);
    call->deleteLater();

    PendingCallWatcherWithContext *context =
        qobject_cast<PendingCallWatcherWithContext*>(call);
    QDBusPendingReply<> signOnUiReply = *call;
    bool ok = !signOnUiReply.isError();
    TRACE() << (ok ? "removeIdentityData succeeded" : "removeIdentityData failed");

    emit infoUpdated((int)SignOn::IdentityRemoved);

    QDBusMessage reply = context->message().createReply();
    context->connection().send(reply);
}

bool SignonIdentity::signOut()
{
    TRACE() << "Signout request. Identity ID: " << id();
    /*
     * - If the identity is stored (thus registered here)
     * signal 'sign out' to all identities subsribed to this object,
     * otherwise the only identity subscribed to this is the newly
     * created client side identity, which called this method.
     * - This is just a safety check, as the client identity - if it is a new
     * one - should not inform server side to sign out.
     */
    if (id() != SIGNOND_NEW_IDENTITY) {
        //clear stored sessiondata
        CredentialsDB *db =
            CredentialsAccessManager::instance()->credentialsDB();
        if ((db == 0) || !db->removeData(m_id)) {
            TRACE() << "clear data failed";
        }

        setDelayedReply(true);
        setAutoDestruct(false);
        PendingCallWatcherWithContext *watcher =
            new PendingCallWatcherWithContext(m_signonui->removeIdentityData(m_id),
                                              this);
        connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                this, SLOT(signOutCompleted(QDBusPendingCallWatcher*)));
    }
    keepInUse();
    return true;
}

void SignonIdentity::signOutCompleted(QDBusPendingCallWatcher *call)
{
    Q_ASSERT(call != NULL);

    setAutoDestruct(true);
    call->deleteLater();

    PendingCallWatcherWithContext *context =
        qobject_cast<PendingCallWatcherWithContext*>(call);
    QDBusPendingReply<> signOnUiReply = *call;
    bool ok = !signOnUiReply.isError();
    TRACE() << (ok ? "removeIdentityData succeeded" : "removeIdentityData failed");

    emit infoUpdated((int)SignOn::IdentitySignedOut);

    QDBusMessage reply = context->message().createReply();
    reply << ok;
    context->connection().send(reply);
}

void SignonIdentity::onCredentialsUpdated(quint32 id)
{
    if (id != m_id) return;

    TRACE() << m_id;

    /* Clear the cached information about the identity; some of it might not be
     * valid anymore */
    if (m_pInfo) {
        delete m_pInfo;
        m_pInfo = NULL;
    }

    emit infoUpdated((int)SignOn::IdentityDataUpdated);
}

quint32 SignonIdentity::store(const QVariantMap &info)
{
    keepInUse();
    SIGNON_RETURN_IF_CAM_UNAVAILABLE(SIGNOND_NEW_IDENTITY);

    const QDBusContext &context = static_cast<QDBusContext>(*this);
    QString appId =
        AccessControlManagerHelper::instance()->appIdOfPeer(
                                                   context.connection(),
                                                   context.message());

    const QVariant container = info.value(SIGNOND_IDENTITY_INFO_AUTHMETHODS);
    MethodMap methods = container.isValid() ?
        qdbus_cast<MethodMap>(container.value<QDBusArgument>()) : MethodMap();

    //Add creator to owner list if it has AID
    QStringList ownerList =
        info.value(SIGNOND_IDENTITY_INFO_OWNER).toStringList();
    if (!appId.isNull())
        ownerList.append(appId);

    if (m_pInfo == 0) {
        m_pInfo = new SignonIdentityInfo(info);
        m_pInfo->setMethods(methods);
        m_pInfo->setOwnerList(ownerList);
    } else {
        if (info.contains(SIGNOND_IDENTITY_INFO_SECRET)) {
            QString secret = info.value(SIGNOND_IDENTITY_INFO_SECRET).toString();
            m_pInfo->setPassword(secret);
        }
        bool storeSecret =
            info.value(SIGNOND_IDENTITY_INFO_STORESECRET).toBool();
        QString userName =
            info.value(SIGNOND_IDENTITY_INFO_USERNAME).toString();
        QString caption =
            info.value(SIGNOND_IDENTITY_INFO_CAPTION).toString();
        QStringList realms =
            info.value(SIGNOND_IDENTITY_INFO_REALMS).toStringList();
        QStringList accessControlList =
            info.value(SIGNOND_IDENTITY_INFO_ACL).toStringList();
        int type = info.value(SIGNOND_IDENTITY_INFO_TYPE).toInt();

        m_pInfo->setStorePassword(storeSecret);
        m_pInfo->setUserName(userName);
        m_pInfo->setCaption(caption);
        m_pInfo->setMethods(methods);
        m_pInfo->setRealms(realms);
        m_pInfo->setAccessControlList(accessControlList);
        m_pInfo->setOwnerList(ownerList);
        m_pInfo->setType(type);
    }

    m_id = storeCredentials(*m_pInfo);

    if (m_id == SIGNOND_NEW_IDENTITY) {
        sendErrorReply(SIGNOND_STORE_FAILED_ERR_NAME,
                       SIGNOND_STORE_FAILED_ERR_STR);
    }

    return m_id;
}

quint32 SignonIdentity::storeCredentials(const SignonIdentityInfo &info)
{
    CredentialsDB *db = CredentialsAccessManager::instance()->credentialsDB();
    if (db == NULL) {
        BLAME() << "NULL database handler object.";
        return SIGNOND_NEW_IDENTITY;
    }

    bool newIdentity = info.isNew();

    if (newIdentity)
        m_id = db->insertCredentials(info);
    else
        db->updateCredentials(info);

    if (db->errorOccurred()) {
        if (newIdentity)
            m_id = SIGNOND_NEW_IDENTITY;

        TRACE() << "Error occurred while inserting/updating credentials.";
    } else {
        if (m_pInfo) {
            delete m_pInfo;
            m_pInfo = NULL;
        }
        Q_EMIT stored(this);

        TRACE() << "FRESH, JUST STORED CREDENTIALS ID:" << m_id;
        emit infoUpdated((int)SignOn::IdentityDataUpdated);
    }
    return m_id;
}

void SignonIdentity::queryUiSlot(QDBusPendingCallWatcher *call)
{
    TRACE();
    Q_ASSERT(call != NULL);

    setAutoDestruct(true);

    PendingCallWatcherWithContext *context =
        qobject_cast<PendingCallWatcherWithContext*>(call);
    const QDBusMessage &message = context->message();
    const QDBusConnection &connection = context->connection();

    QDBusMessage errReply;
    QDBusPendingReply<QVariantMap> reply = *call;
    call->deleteLater();

    QVariantMap resultParameters;
    if (!reply.isError() && reply.count()) {
        resultParameters = reply.argumentAt<0>();
    } else {
        errReply = message.createErrorReply(
                                  SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_NAME,
                                  SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_STR);
        connection.send(errReply);
        return;
    }

    if (!resultParameters.contains(SSOUI_KEY_ERROR)) {
        //no reply code
        errReply = message.createErrorReply(SIGNOND_INTERNAL_SERVER_ERR_NAME,
                                            SIGNOND_INTERNAL_SERVER_ERR_STR);
        connection.send(errReply);
        return;
    }

    int errorCode = resultParameters.value(SSOUI_KEY_ERROR).toInt();
    TRACE() << "error: " << errorCode;
    if (errorCode != QUERY_ERROR_NONE) {
        if (errorCode == QUERY_ERROR_CANCELED)
            errReply =
                message.createErrorReply(
                                  SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_NAME,
                                  SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_STR);
        else
            errReply =
                message.createErrorReply(SIGNOND_INTERNAL_SERVER_ERR_NAME,
                    QString(QLatin1String("signon-ui call returned error %1")).
                    arg(errorCode));

        connection.send(errReply);
        return;
    }

    if (resultParameters.contains(SSOUI_KEY_PASSWORD)) {
        CredentialsDB *db =
            CredentialsAccessManager::instance()->credentialsDB();
        if (db == NULL) {
            BLAME() << "NULL database handler object.";
            errReply = message.createErrorReply(SIGNOND_STORE_FAILED_ERR_NAME,
                    SIGNOND_STORE_FAILED_ERR_STR);
            connection.send(errReply);
            return;
        }

        //store new password
        if (m_pInfo) {
            m_pInfo->setPassword(resultParameters[SSOUI_KEY_PASSWORD].toString());

            quint32 ret = db->updateCredentials(*m_pInfo);
            delete m_pInfo;
            m_pInfo = NULL;
            if (ret != SIGNOND_NEW_IDENTITY) {
                QDBusMessage dbusreply = message.createReply();
                dbusreply << quint32(m_id);
                connection.send(dbusreply);
                return;
            } else{
                BLAME() << "Error during update";
            }
        }
    }

    //this should not happen, return error
    errReply = message.createErrorReply(SIGNOND_INTERNAL_SERVER_ERR_NAME,
            SIGNOND_INTERNAL_SERVER_ERR_STR);
    connection.send(errReply);
    return;
}

void SignonIdentity::verifyUiSlot(QDBusPendingCallWatcher *call)
{
    TRACE();
    Q_ASSERT(call != NULL);

    setAutoDestruct(true);

    PendingCallWatcherWithContext *context =
        qobject_cast<PendingCallWatcherWithContext*>(call);
    const QDBusMessage &message = context->message();
    const QDBusConnection &connection = context->connection();

    QDBusMessage errReply;
    QDBusPendingReply<QVariantMap> reply = *call;
    call->deleteLater();
    QVariantMap resultParameters;
    if (!reply.isError() && reply.count()) {
        resultParameters = reply.argumentAt<0>();
    } else {
        errReply =
            message.createErrorReply(
                                 SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_NAME,
                                 SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_STR);
        connection.send(errReply);
        return;
    }

    if (!resultParameters.contains(SSOUI_KEY_ERROR)) {
        //no reply code
        errReply = message.createErrorReply(SIGNOND_INTERNAL_SERVER_ERR_NAME,
                                            SIGNOND_INTERNAL_SERVER_ERR_STR);
        connection.send(errReply);
        return;
    }

    int errorCode = resultParameters.value(SSOUI_KEY_ERROR).toInt();
    TRACE() << "error: " << errorCode;
    if (errorCode != QUERY_ERROR_NONE) {
        if (errorCode == QUERY_ERROR_CANCELED)
            errReply = message.createErrorReply(
                                  SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_NAME,
                                  SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_STR);
        else if (errorCode == QUERY_ERROR_FORGOT_PASSWORD)
            errReply = message.createErrorReply(
                                  SIGNOND_FORGOT_PASSWORD_ERR_NAME,
                                  SIGNOND_FORGOT_PASSWORD_ERR_STR);
        else
            errReply = message.createErrorReply(
                                  SIGNOND_INTERNAL_SERVER_ERR_NAME,
                                  QString(QLatin1String("signon-ui call "
                                                        "returned error %1")).
                                  arg(errorCode));

        connection.send(errReply);
        return;
    }

    if (resultParameters.contains(SSOUI_KEY_PASSWORD)) {
        CredentialsDB *db =
            CredentialsAccessManager::instance()->credentialsDB();
        if (db == NULL) {
            BLAME() << "NULL database handler object.";
            errReply = message.createErrorReply(SIGNOND_STORE_FAILED_ERR_NAME,
                    SIGNOND_STORE_FAILED_ERR_STR);
            connection.send(errReply);
            return;
        }

        //compare passwords
        if (m_pInfo) {
            bool ret =
                m_pInfo->password() == resultParameters[SSOUI_KEY_PASSWORD].
                toString();

            if (!ret && resultParameters.contains(SSOUI_KEY_CONFIRMCOUNT)) {
                int count = resultParameters[SSOUI_KEY_CONFIRMCOUNT].toInt();
                TRACE() << "retry count:" << count;
                if (count > 0) { //retry
                    resultParameters[SSOUI_KEY_CONFIRMCOUNT] = (count-1);
                    resultParameters[SSOUI_KEY_MESSAGEID] =
                        QUERY_MESSAGE_NOT_AUTHORIZED;
                    queryUserPassword(resultParameters, connection, message);
                    return;
                } else {
                    //TODO show error note here if needed
                }
            }
            delete m_pInfo;
            m_pInfo = NULL;
            QDBusMessage dbusreply = message.createReply();
            dbusreply << ret;
            connection.send(dbusreply);
            return;
        }
    }
    //this should not happen, return error
    errReply = message.createErrorReply(SIGNOND_INTERNAL_SERVER_ERR_NAME,
            SIGNOND_INTERNAL_SERVER_ERR_STR);
    connection.send(errReply);
    return;
}

} //namespace SignonDaemonNS

#include "signonidentity.moc"
