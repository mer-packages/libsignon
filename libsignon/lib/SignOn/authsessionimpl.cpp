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
#include "signond/signoncommon.h"

#include "authsessionimpl.h"
#include "libsignoncommon.h"


#define SIGNOND_AUTHSESSION_CONNECTION_PROBLEM \
    QLatin1String("Cannot create remote AuthSession object: " \
                  "check the signon daemon and authentication plugin.")

#define SIGNOND_SESSION_PROCESS_METHOD \
    SIGNOND_NORMALIZE_METHOD_SIGNATURE("process(const SessionData &, const QString &)")
#define SIGNOND_SESSION_SET_ID_METHOD \
    SIGNOND_NORMALIZE_METHOD_SIGNATURE("setId(quint32)")
#define SIGNOND_SESSION_QUERY_AVAILABLE_MECHANISMS_METHOD \
    SIGNOND_NORMALIZE_METHOD_SIGNATURE("queryAvailableMechanisms(const QStringList &)")

#ifndef SIGNOND_NEW_IDENTITY
    #define SIGNOND_NEW_IDENTITY 0
#endif

using namespace SignOn;

static QVariantMap sessionData2VariantMap(const SessionData &data)
{
    QVariantMap result;

    foreach(QString key, data.propertyNames()) {
        if (!data.getProperty(key).isNull() && data.getProperty(key).isValid())
            result[key] = data.getProperty(key);
    }

    return result;
}

AuthSessionImpl::AuthSessionImpl(AuthSession *parent,
                                 quint32 id,
                                 const QString &methodName):
    QObject(parent),
    m_parent(parent),
    m_operationQueueHandler(this),
    m_methodName(methodName)
{
    m_id = id;
    m_DBusInterface = 0;
    m_isValid = true;
    m_isAuthInProcessing = false;
    m_isBusy = false;

    initInterface();
}

AuthSessionImpl::~AuthSessionImpl()
{
    if (m_DBusInterface) {
        m_DBusInterface->call(QLatin1String("objectUnref"));
        delete m_DBusInterface;
    }
}

void AuthSessionImpl::send2interface(const QString &operation,
                                     const char *slot,
                                     const QVariantList &arguments)
{
    if (!m_DBusInterface || !m_DBusInterface->isValid()) {
        emit m_parent->error(Error(Error::InternalCommunication,
                                   SIGNOND_INTERNAL_COMMUNICATION_ERR_STR));
        return;
    }

    bool res = true;

    if (slot) {
        QDBusMessage msg =
            QDBusMessage::createMethodCall(m_DBusInterface->service(),
                                           m_DBusInterface->path(),
                                           m_DBusInterface->interface(),
                                           operation);
        if (!arguments.isEmpty())
            msg.setArguments(arguments);

        msg.setDelayedReply(true);

        res = m_DBusInterface->connection().
            callWithCallback(msg, this, slot,
                             SLOT(errorSlot(const QDBusError&)),
                             SIGNOND_MAX_TIMEOUT);
    } else {
        m_DBusInterface->callWithArgumentList(QDBus::NoBlock,
                                              operation, arguments);
    }

    if (!res) {
        emit m_parent->error(Error(Error::InternalCommunication,
                                   m_DBusInterface->lastError().message()));

    } else {
        emit m_parent->stateChanged(AuthSession::ProcessPending,
                                    QLatin1String("The request is added "
                                                  "to queue."));
    }
}

void AuthSessionImpl::setId(quint32 id)
{
    if (!m_isValid) {
        emit m_parent->error(
                Error(Error::InternalCommunication,
                      QString(QLatin1String("AuthSession(%1) cannot perform "
                                            "operation.")).arg(m_methodName)));
        return;
    }

    m_id = id;

    QLatin1String remoteFunctionName("setId");

    QVariantList arguments;
    arguments += id;

    if (m_DBusInterface)
        send2interface(remoteFunctionName, 0, arguments);
    else
        m_operationQueueHandler.enqueueOperation(
                                    SIGNOND_SESSION_SET_ID_METHOD,
                                    QList<QGenericArgument *>() <<
                                    (new Q_ARG(quint32, id)));
}

bool AuthSessionImpl::checkConnection()
{
    /*
     * if the AuthSession is unable to connect to SignonDaemon
     * or was refused to create a remote AuthSession then
     * there is no sense to try again (????)
     */
    if (!m_isValid)
        return false;

    if (m_isAuthInProcessing)
        return true;

    if (!m_DBusInterface || m_DBusInterface->lastError().isValid())
        return initInterface();

    TRACE();
    return true;
}

bool AuthSessionImpl::initInterface()
{
    TRACE();
    m_isAuthInProcessing = true;

    if (m_DBusInterface != NULL)
    {
        delete m_DBusInterface;
        m_DBusInterface = 0;
    }

    m_operationQueueHandler.stopOperationsProcessing();

    QLatin1String operation("getAuthSessionObjectPath");
    QDBusMessage msg = QDBusMessage::createMethodCall(SIGNOND_SERVICE,
                                                      SIGNOND_DAEMON_OBJECTPATH,
                                                      SIGNOND_DAEMON_INTERFACE,
                                                      operation);
    QVariantList arguments;
    arguments += m_id;
    arguments += m_methodName;

    msg.setArguments(arguments);
    msg.setDelayedReply(true);

    return SIGNOND_BUS.callWithCallback(
        msg, this,
        SLOT(authenticationSlot(const QString&)),
        SLOT(errorSlot(const QDBusError&)));
}

QString AuthSessionImpl::name()
{
    return m_methodName;
}

void
AuthSessionImpl::queryAvailableMechanisms(const QStringList &wantedMechanisms)
{
    if (!checkConnection()) {
        qCritical() << SIGNOND_AUTHSESSION_CONNECTION_PROBLEM;
        emit m_parent->error(Error(Error::InternalCommunication,
                                   SIGNOND_AUTHSESSION_CONNECTION_PROBLEM));
        return;
    }

    QLatin1String remoteFunctionName("queryAvailableMechanisms");

    QVariantList arguments;
    arguments += wantedMechanisms;

    if (m_DBusInterface)
        send2interface(remoteFunctionName,
                       SLOT(mechanismsAvailableSlot(const QStringList&)),
                       arguments);
    else
        m_operationQueueHandler.enqueueOperation(
                        SIGNOND_SESSION_QUERY_AVAILABLE_MECHANISMS_METHOD,
                        QList<QGenericArgument *>() <<
                        (new Q_ARG(QStringList, wantedMechanisms)));
}

void AuthSessionImpl::process(const SessionData &sessionData,
                              const QString &mechanism)
{
    if (!checkConnection()) {
        qCritical() << SIGNOND_AUTHSESSION_CONNECTION_PROBLEM;
        emit m_parent->error(
                Error(Error::InternalCommunication,
                      SIGNOND_AUTHSESSION_CONNECTION_PROBLEM));
        return;
    }

    if (m_isBusy) {
        qCritical() << "AuthSession: client is busy";

        emit m_parent->error(
                Error(Error::Unknown,
                      QString(QLatin1String("AuthSession(%1) is busy"))
                         .arg(m_methodName)));
        return;
    }

    QVariantMap sessionDataVa = sessionData2VariantMap(sessionData);
    QString remoteFunctionName;

    QVariantList arguments;
    arguments += sessionDataVa;
    arguments += mechanism;

    remoteFunctionName = QLatin1String("process");

    if (m_DBusInterface) {
        TRACE() << "sending to daemon";
        send2interface(remoteFunctionName,
                       SLOT(responseSlot(const QVariantMap&)), arguments);
        m_isBusy = true;
    } else {
        TRACE() << "sending to queue";
        QList<QGenericArgument *> args;
        args << (new Q_ARG(QVariantMap, sessionDataVa))
             << (new Q_ARG(QString, mechanism));

        m_operationQueueHandler.enqueueOperation(SIGNOND_SESSION_PROCESS_METHOD,
                                                 args);
    }
}

void AuthSessionImpl::cancel()
{
    if (!checkConnection()) {
        qCritical() << SIGNOND_AUTHSESSION_CONNECTION_PROBLEM;

        emit m_parent->error(Error(Error::InternalCommunication,
                                   SIGNOND_AUTHSESSION_CONNECTION_PROBLEM));
        return;
    }

    if (!m_isBusy &&
        !m_operationQueueHandler.queueContainsOperation(
                                              SIGNOND_SESSION_PROCESS_METHOD)) {
        qCritical() << "no requests to be canceled";
        return;
    }

    if (!m_DBusInterface) {
        m_operationQueueHandler.removeOperation(SIGNOND_SESSION_PROCESS_METHOD);

        emit m_parent->error(Error(Error::SessionCanceled,
                                   QLatin1String("Process is canceled.")));

    } else {
        TRACE() << "Sending cancel-request";
        m_DBusInterface->call(QDBus::NoBlock, QLatin1String("cancel"));
    }

    m_isBusy = false;
}

void AuthSessionImpl::errorSlot(const QDBusError &err)
{
    TRACE() << err;

    m_isBusy = false;
    int errCode = Error::Unknown;
    QString errMessage;

    if (err.type() != QDBusError::Other) {
        qCritical() << err.type();
        qCritical() << err.name();
        qCritical() << err.message();

        /*
         * if we got another error code then we have some sort of
         * problems with DBus: try to reset DBus interface
         * */
        if (err.type() == QDBusError::UnknownObject) {
            delete m_DBusInterface;
            m_DBusInterface = NULL;
        }

    } else if (err.name() == SIGNOND_SESSION_CANCELED_ERR_NAME) {
        errCode = Error::SessionCanceled;
    } else if (err.name() == SIGNOND_TIMED_OUT_ERR_NAME) {
        errCode = Error::TimedOut;
    } else if (err.name() == SIGNOND_INVALID_CREDENTIALS_ERR_NAME) {
        errCode = Error::InvalidCredentials;
    } else if (err.name() == SIGNOND_NOT_AUTHORIZED_ERR_NAME) {
        errCode = Error::NotAuthorized;
    } else if (err.name() == SIGNOND_OPERATION_NOT_SUPPORTED_ERR_NAME) {
        errCode = Error::OperationNotSupported;
    } else if (err.name() == SIGNOND_PERMISSION_DENIED_ERR_NAME) {
        errCode = Error::PermissionDenied;
    } else if (err.name() == SIGNOND_METHOD_OR_MECHANISM_NOT_ALLOWED_ERR_NAME) {
        errCode = Error::MethodOrMechanismNotAllowed;
    } else if (err.name() == SIGNOND_WRONG_STATE_ERR_NAME) {
        errCode = Error::WrongState;
    } else if (err.name() == SIGNOND_MECHANISM_NOT_AVAILABLE_ERR_NAME) {
        errCode = Error::MechanismNotAvailable;
    } else if (err.name() == SIGNOND_METHOD_NOT_KNOWN_ERR_NAME) {
        m_isValid = false;
        m_isAuthInProcessing = false;
        errCode = Error::MethodNotAvailable;
    } else if (err.name() == SIGNOND_MISSING_DATA_ERR_NAME) {
         errCode = Error::MissingData;
    } else if (err.name() == SIGNOND_RUNTIME_ERR_NAME) {
        errCode = Error::Runtime;
    } else if (err.name() == SIGNOND_NO_CONNECTION_ERR_NAME) {
        errCode = Error::NoConnection;
    } else if (err.name() == SIGNOND_NETWORK_ERR_NAME) {
        errCode = Error::Network;
    } else if (err.name() == SIGNOND_SSL_ERR_NAME) {
        errCode = Error::Ssl;
    } else if (err.name() == SIGNOND_USER_INTERACTION_ERR_NAME) {
        errCode = Error::UserInteraction;
    } else if (err.name() == SIGNOND_OPERATION_FAILED_ERR_NAME) {
        errCode = Error::OperationFailed;
    } else if (err.name() == SIGNOND_ENCRYPTION_FAILED_ERR_NAME) {
        errCode = Error::EncryptionFailure;
    } else if (err.name() == SIGNOND_TOS_NOT_ACCEPTED_ERR_NAME) {
        errCode = Error::TOSNotAccepted;
    } else if (err.name() == SIGNOND_FORGOT_PASSWORD_ERR_NAME) {
        errCode = Error::ForgotPassword;
    } else if (err.name() == SIGNOND_INCORRECT_DATE_ERR_NAME) {
        errCode = Error::IncorrectDate;
    } else if (err.name() == SIGNOND_USER_ERROR_ERR_NAME){
        //the error message comes in as "code:message"
        bool ok = false;
        errCode = err.message().section(QLatin1Char(':'), 0, 0).toInt(&ok);
        errMessage = err.message().section(QLatin1Char(':'), 1, 1);
        if (!ok)
            errCode = Error::Unknown;
    } else {
        if (m_isAuthInProcessing) {
            qCritical() << "Cannot connect to SignonDaemon: " << err;

            m_isValid = false;
            m_isAuthInProcessing = false;

            emit m_parent->error(
                    Error(Error::InternalCommunication,
                          SIGNOND_AUTHSESSION_CONNECTION_PROBLEM));
            return;
        }
    }

    if (errMessage.isEmpty())
        errMessage = err.message();

    emit m_parent->error(Error(errCode, errMessage));

}

void AuthSessionImpl::authenticationSlot(const QString &path)
{
    if (!path.isEmpty()) {
        m_DBusInterface = new DBusInterface(SIGNOND_SERVICE,
                                            path,
                                            SIGNOND_AUTH_SESSION_INTERFACE_C,
                                            SIGNOND_BUS);
        m_DBusInterface->connect("stateChanged", this,
                                 SLOT(stateSlot(int, const QString&)));
        m_DBusInterface->connect("unregistered", this,
                                 SLOT(unregisteredSlot()));

        if (m_operationQueueHandler.queuedOperationsCount() > 0)
            m_operationQueueHandler.execQueuedOperations();
    } else {
        int numberOfErrorReplies =
            m_operationQueueHandler.queuedOperationsCount();
        for (int i = 0; i < numberOfErrorReplies; i++) {

            emit m_parent->error(
                    Error(Error::Unknown,
                          QLatin1String("The given session cannot be "
                                        "accessed.")));
        }
        m_isValid = false;
    }

    m_isAuthInProcessing = false;
    if (m_operationQueueHandler.queuedOperationsCount() > 0)
        m_operationQueueHandler.clearOperationsQueue();
}

void AuthSessionImpl::mechanismsAvailableSlot(const QStringList& mechanisms)
{
    emit m_parent->mechanismsAvailable(mechanisms);
}

void AuthSessionImpl::responseSlot(const QVariantMap &sessionDataVa)
{
    m_isBusy = false;

    emit m_parent->response(SessionData(sessionDataVa));
}

void AuthSessionImpl::stateSlot(int state, const QString &message)
{
    emit m_parent->stateChanged((AuthSession::AuthSessionState)state, message);
}

void AuthSessionImpl::unregisteredSlot()
{
    delete m_DBusInterface;
    m_DBusInterface = NULL;

    m_isAuthInProcessing = false;
    m_isValid = true;
}
