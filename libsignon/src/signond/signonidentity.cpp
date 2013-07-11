/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2011 Intel Corporation.
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

SignonIdentity::SignonIdentity(quint32 id, int timeout,
                               SignonDaemon *parent):
    SignonDisposable(timeout, parent),
    m_pInfo(NULL),
    m_pSignonDaemon(parent),
    m_registered(false)
{
    m_id = id;

    /*
     * creation of unique name for the given identity
     * */
    static quint32 incr = 0;
    QString objectName = SIGNOND_DAEMON_OBJECTPATH + QLatin1String("/Identity_")
                         + QString::number(incr++, 16);
    setObjectName(objectName);

    m_signonui = new SignonUiAdaptor(
                                    SIGNON_UI_SERVICE,
                                    SIGNON_UI_DAEMON_OBJECTPATH,
                                    SIGNOND_BUS,
                                    this);
}

SignonIdentity::~SignonIdentity()
{
    if (m_registered)
    {
        emit unregistered();
        QDBusConnection connection = SIGNOND_BUS;
        connection.unregisterObject(objectName());
    }

    if (credentialsStored())
        m_pSignonDaemon->m_storedIdentities.remove(m_id);
    else
        m_pSignonDaemon->m_unstoredIdentities.remove(objectName());

    delete m_signonui;
}

bool SignonIdentity::init()
{
    QDBusConnection connection = SIGNOND_BUS;

    if (!connection.isConnected()) {
        QDBusError err = connection.lastError();
        TRACE() << "Connection cannot be established:" <<
            err.errorString(err.type()) ;
        return false;
    }

    QDBusConnection::RegisterOptions registerOptions =
        QDBusConnection::ExportAllContents;

    (void)new SignonIdentityAdaptor(this);
    registerOptions = QDBusConnection::ExportAdaptors;

    if (!connection.registerObject(objectName(), this, registerOptions)) {
        TRACE() << "Object cannot be registered: " << objectName();
        return false;
    }

    return (m_registered = true);
}

SignonIdentity *SignonIdentity::createIdentity(quint32 id, SignonDaemon *parent)
{
    SignonIdentity *identity =
        new SignonIdentity(id, parent->identityTimeout(), parent);

    if (!identity->init()) {
        TRACE() << "The created identity is invalid and will be deleted.\n";
        delete identity;
        return NULL;
    }

    return identity;
}

void SignonIdentity::destroy()
{
    if (m_registered)
    {
        emit unregistered();
        QDBusConnection connection = SIGNOND_BUS;
        connection.unregisterObject(objectName());
        m_registered = false;
    }

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
    QString appId =
        AccessControlManagerHelper::instance()->appIdOfPeer(
                                 (static_cast<QDBusContext>(*this)).message());
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
    QString appId =
        AccessControlManagerHelper::instance()->appIdOfPeer(
                                  (static_cast<QDBusContext>(*this)).message());
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
    m_message = message();

    //create ui request to ask password
    QVariantMap uiRequest;
    uiRequest.insert(SSOUI_KEY_QUERYPASSWORD, true);
    uiRequest.insert(SSOUI_KEY_USERNAME, info.userName());
    uiRequest.insert(SSOUI_KEY_MESSAGE, displayMessage);
    uiRequest.insert(SSOUI_KEY_CAPTION, info.caption());

    TRACE() << "Waiting for reply from signon-ui";
    QDBusPendingCallWatcher *watcher =
        new QDBusPendingCallWatcher(m_signonui->queryDialog(uiRequest), this);
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
    return info.toMap();
}

void SignonIdentity::queryUserPassword(const QVariantMap &params) {
    TRACE() << "Waiting for reply from signon-ui";
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(
            m_signonui->queryDialog(params), this);
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
    m_message = message();

    //create ui request to ask password
    QVariantMap uiRequest;
    uiRequest.unite(params);
    uiRequest.insert(SSOUI_KEY_QUERYPASSWORD, true);
    uiRequest.insert(SSOUI_KEY_USERNAME, info.userName());
    uiRequest.insert(SSOUI_KEY_CAPTION, info.caption());

    queryUserPassword(uiRequest);
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
    emit infoUpdated((int)SignOn::IdentityRemoved);
    keepInUse();
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

        emit infoUpdated((int)SignOn::IdentitySignedOut);
    }
    keepInUse();
    return true;
}

quint32 SignonIdentity::store(const QVariantMap &info)
{
    keepInUse();
    SIGNON_RETURN_IF_CAM_UNAVAILABLE(SIGNOND_NEW_IDENTITY);

    QString secret = info.value(SIGNOND_IDENTITY_INFO_SECRET).toString();
    QString appId =
        AccessControlManagerHelper::instance()->appIdOfPeer(
                                 (static_cast<QDBusContext>(*this)).message());

    bool storeSecret = info.value(SIGNOND_IDENTITY_INFO_STORESECRET).toBool();
    QVariant container = info.value(SIGNOND_IDENTITY_INFO_AUTHMETHODS);
    MethodMap methods =
        qdbus_cast<MethodMap>(container.value<QDBusArgument>());

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
        QString userName =
            info.value(SIGNOND_IDENTITY_INFO_USERNAME).toString();
        QString caption =
            info.value(SIGNOND_IDENTITY_INFO_CAPTION).toString();
        QStringList realms =
            info.value(SIGNOND_IDENTITY_INFO_REALMS).toStringList();
        QStringList accessControlList =
            info.value(SIGNOND_IDENTITY_INFO_ACL).toStringList();
        int type = info.value(SIGNOND_IDENTITY_INFO_TYPE).toInt();

        m_pInfo->setUserName(userName);
        m_pInfo->setCaption(caption);
        m_pInfo->setMethods(methods);
        m_pInfo->setRealms(realms);
        m_pInfo->setAccessControlList(accessControlList);
        m_pInfo->setOwnerList(ownerList);
        m_pInfo->setType(type);
    }

    m_pInfo->setPassword(secret);
    m_pInfo->setStorePassword(storeSecret);
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
        m_pSignonDaemon->identityStored(this);

        TRACE() << "FRESH, JUST STORED CREDENTIALS ID:" << m_id;
        emit infoUpdated((int)SignOn::IdentityDataUpdated);
    }
    return m_id;
}

void SignonIdentity::queryUiSlot(QDBusPendingCallWatcher *call)
{
    TRACE();
    setAutoDestruct(true);

    QDBusMessage errReply;
    QDBusPendingReply<QVariantMap> reply;
    if (call != NULL) {
        reply = *call;
        call->deleteLater();
    }
    QVariantMap resultParameters;
    if (!reply.isError() && reply.count()) {
        resultParameters = reply.argumentAt<0>();
    } else {
        errReply =
            m_message.createErrorReply(
                                  SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_NAME,
                                  SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_STR);
        SIGNOND_BUS.send(errReply);
        return;
    }

    if (!resultParameters.contains(SSOUI_KEY_ERROR)) {
        //no reply code
        errReply = m_message.createErrorReply(SIGNOND_INTERNAL_SERVER_ERR_NAME,
                SIGNOND_INTERNAL_SERVER_ERR_STR);
        SIGNOND_BUS.send(errReply);
        return;
    }

    int errorCode = resultParameters.value(SSOUI_KEY_ERROR).toInt();
    TRACE() << "error: " << errorCode;
    if (errorCode != QUERY_ERROR_NONE) {
        if (errorCode == QUERY_ERROR_CANCELED)
            errReply =
                m_message.createErrorReply(
                                  SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_NAME,
                                  SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_STR);
        else
            errReply =
                m_message.createErrorReply(SIGNOND_INTERNAL_SERVER_ERR_NAME,
                    QString(QLatin1String("signon-ui call returned error %1")).
                    arg(errorCode));

        SIGNOND_BUS.send(errReply);
        return;
    }

    if (resultParameters.contains(SSOUI_KEY_PASSWORD)) {
        CredentialsDB *db =
            CredentialsAccessManager::instance()->credentialsDB();
        if (db == NULL) {
            BLAME() << "NULL database handler object.";
            errReply = m_message.createErrorReply(SIGNOND_STORE_FAILED_ERR_NAME,
                    SIGNOND_STORE_FAILED_ERR_STR);
            SIGNOND_BUS.send(errReply);
            return;
        }

        //store new password
        if (m_pInfo) {
            m_pInfo->setPassword(resultParameters[SSOUI_KEY_PASSWORD].toString());

            quint32 ret = db->updateCredentials(*m_pInfo);
            delete m_pInfo;
            m_pInfo = NULL;
            if (ret != SIGNOND_NEW_IDENTITY) {
                QDBusMessage dbusreply = m_message.createReply();
                dbusreply << quint32(m_id);
                SIGNOND_BUS.send(dbusreply);
                return;
            } else{
                BLAME() << "Error during update";
            }
        }
    }

    //this should not happen, return error
    errReply = m_message.createErrorReply(SIGNOND_INTERNAL_SERVER_ERR_NAME,
            SIGNOND_INTERNAL_SERVER_ERR_STR);
    SIGNOND_BUS.send(errReply);
    return;
}

void SignonIdentity::verifyUiSlot(QDBusPendingCallWatcher *call)
{
    TRACE();
    setAutoDestruct(true);

    QDBusMessage errReply;
    QDBusPendingReply<QVariantMap> reply;
    if (call != NULL) {
        reply = *call;
        call->deleteLater();
    }
    QVariantMap resultParameters;
    if (!reply.isError() && reply.count()) {
        resultParameters = reply.argumentAt<0>();
    } else {
        errReply =
            m_message.createErrorReply(
                                 SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_NAME,
                                 SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_STR);
        SIGNOND_BUS.send(errReply);
        return;
    }

    if (!resultParameters.contains(SSOUI_KEY_ERROR)) {
        //no reply code
        errReply = m_message.createErrorReply(SIGNOND_INTERNAL_SERVER_ERR_NAME,
                SIGNOND_INTERNAL_SERVER_ERR_STR);
        SIGNOND_BUS.send(errReply);
        return;
    }

    int errorCode = resultParameters.value(SSOUI_KEY_ERROR).toInt();
    TRACE() << "error: " << errorCode;
    if (errorCode != QUERY_ERROR_NONE) {
        if (errorCode == QUERY_ERROR_CANCELED)
            errReply = m_message.createErrorReply(
                                  SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_NAME,
                                  SIGNOND_IDENTITY_OPERATION_CANCELED_ERR_STR);
        else if (errorCode == QUERY_ERROR_FORGOT_PASSWORD)
            errReply = m_message.createErrorReply(
                                  SIGNOND_FORGOT_PASSWORD_ERR_NAME,
                                  SIGNOND_FORGOT_PASSWORD_ERR_STR);
        else
            errReply = m_message.createErrorReply(
                                  SIGNOND_INTERNAL_SERVER_ERR_NAME,
                                  QString(QLatin1String("signon-ui call "
                                                        "returned error %1")).
                                  arg(errorCode));

        SIGNOND_BUS.send(errReply);
        return;
    }

    if (resultParameters.contains(SSOUI_KEY_PASSWORD)) {
        CredentialsDB *db =
            CredentialsAccessManager::instance()->credentialsDB();
        if (db == NULL) {
            BLAME() << "NULL database handler object.";
            errReply = m_message.createErrorReply(SIGNOND_STORE_FAILED_ERR_NAME,
                    SIGNOND_STORE_FAILED_ERR_STR);
            SIGNOND_BUS.send(errReply);
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
                    queryUserPassword(resultParameters);
                    return;
                } else {
                    //TODO show error note here if needed
                }
            }
            delete m_pInfo;
            m_pInfo = NULL;
            QDBusMessage dbusreply = m_message.createReply();
            dbusreply << ret;
            SIGNOND_BUS.send(dbusreply);
            return;
        }
    }
    //this should not happen, return error
    errReply = m_message.createErrorReply(SIGNOND_INTERNAL_SERVER_ERR_NAME,
            SIGNOND_INTERNAL_SERVER_ERR_STR);
    SIGNOND_BUS.send(errReply);
    return;
}

} //namespace SignonDaemonNS
