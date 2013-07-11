/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
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
/*!
 * @copyright Copyright (C) 2009-2011 Nokia Corporation.
 * @license LGPL
 */

#ifndef AUTHPLUGINIF_H
#define AUTHPLUGINIF_H

#include <QtCore/qobject.h>
#include <QtCore/qpointer.h>
#include <QtCore/qplugin.h>

#include <QVariantMap>
#include <SignOn/sessiondata.h>
#include <SignOn/uisessiondata.h>
#include <SignOn/signonerror.h>

QT_BEGIN_NAMESPACE
class QString;
class QStringList;
class QByteArray;
class QVariant;
QT_END_NAMESPACE

/*!
 * Predefined states to be used for progress reporting.
 */
enum AuthPluginState {
    PLUGIN_STATE_NONE = 0,             /**< State unknown. */
    PLUGIN_STATE_RESOLVING,          /**< Resolving remote server host name. */
    PLUGIN_STATE_CONNECTING,      /**< Connecting to remote server. */
    PLUGIN_STATE_SENDING,             /**< Sending data to remote server. */
    PLUGIN_STATE_WAITING,            /**< Waiting for reply from remote server. */
    PLUGIN_STATE_PENDING,             /**< Waiting for response from user. */
    PLUGIN_STATE_REFRESHING,       /**< Refreshing ui request. */
    PLUGIN_STATE_CANCELING,         /**< Canceling current process. */
    PLUGIN_STATE_HOLDING,            /**< Holding long non-expired token. Process should be kept alive. */
    PLUGIN_STATE_DONE                    /**< Process is finished. Process can be terminated. */
};

/*!
 * Macro to create declarations of
 * SSO authentication plugin.
 * */
#define SIGNON_PLUGIN_INSTANCE(pluginclass) \
        { \
            static AuthPluginInterface *_instance = 0; \
            if (!_instance)      \
                _instance = static_cast<AuthPluginInterface *>(new pluginclass()); \
            return _instance; \
        }

#define SIGNON_DECL_AUTH_PLUGIN(pluginclass) \
        Q_EXTERN_C AuthPluginInterface *auth_plugin_instance() \
        SIGNON_PLUGIN_INSTANCE(pluginclass)

/*!
 * @class AuthPluginInterface.
 * Interface definition for authentication plugins
 */
class AuthPluginInterface : public QObject
{
    Q_OBJECT

public:
    AuthPluginInterface(QObject *parent = 0) : QObject(parent)
        { qRegisterMetaType<SignOn::Error>("SignOn::Error"); }

    /*!
     * Destructor
     */
    virtual ~AuthPluginInterface() {}

    /*!
     * Gets the type of the plugin
     *
     * @return Plugin type
     */
    virtual QString type() const = 0;

    /*!
     * Gets the list of supported mechanisms.
     *
     * @return List of mechanisms
     */
    virtual QStringList mechanisms() const = 0;

    /*!
     * Requests to cancel the process.
     * Process is terminated after this call.
     * Reimplement this in order to execute specific instructions before
     * the effective cancel occurres.
     */
    virtual void cancel() {}

    /*!
     * Requests to abort the process.
     * Process is terminated after this call.
     * Reimplement this in order to execute specific instructions before
     * process is killed.
     */
    virtual void abort() {}

    /*!
     * Process authentication.
     * Authentication can be logging to a server, obtain token(s) from a server,
     * calculate response using given challenge, etc.
     * Given session data is used to do authentication and return response.
     * Signal result() is emitted when authentication is completed,
     * or signal error() if authentication failed.
     * @see result
     * @see error
     *
     * @param inData Input data for authentication
     * @param mechanism Mechanism to use to do authentication
     */
    virtual void process(const SignOn::SessionData &inData,
                         const QString &mechanism = QString()) = 0;

Q_SIGNALS:
    /*!
     * Emitted when authentication process has been completed for given data
     * and there are no errors.
     *
     * @param data Resulting SessionData, need to be returned to client
     */
    void result(const SignOn::SessionData &data);

    /*!
     * Emitted when authentication process want to store session data parameters
     * for later use. Stored parameters are added into SessionData in following process calls.
     * This is useful when authentication is using permanent tokens.
     *
     * @note This is shared within same identity using same method only.
     * @note There can be storage size limitation for data that can be stored.
     *
     * @param data Resulting SessionData, need to be returned to client
     */
    void store(const SignOn::SessionData &data);

    /*!
     * Emitted when authentication process has been completed for given data
     * and resulting an error.
     *
     * @param err The error object
     * @param errorMessage Resulting error message
     */
    void error(const SignOn::Error &err);

    /*!
     * Emitted when authentication process need to interact with user.
     * Basic use cases are: query password, verify captcha, show url.
     * Can also be used to get username/password for proxy authentication etc.
     * Slot userActionFinished() is called when interaction is completed.
     *
     * @see userActionFinished
     * @see SignOn::UiSessionData
     * @note slot userActionFinished() should be reimplemented to get result.
     *
     * @param data Ui session data to be filled within user interaction
     */
    void userActionRequired(const SignOn::UiSessionData &data);

    /*!
     * Emitted when authentication process has completed refresh request.
     * Plugin must emit signal refreshed() to response to refresh() call.
     * @see refreshed
     *
     * @param data Refreshed ui session data
     */
    void refreshed(const SignOn::UiSessionData &data);

     /*!
     * Emitted to report status of authentication process to signond for
     * informing client application.
     *
     * @param state Plugin process state @see AuthPluginState
     * @param message Optional message for client application
     */
    void statusChanged(const AuthPluginState state,
                       const QString &message = QString());

public Q_SLOTS:
    /*!
     * User interaction completed.
     * Signond uses this slot to notice the end of ui session.
     * This is a response to userActionRequired() signal.
     * This must be reimplemented to get the response from the user interaction.
     * @see UiSessionData
     * @see userActionRequired
     *
     * @param data User completed ui session data
     */
    virtual void userActionFinished(const SignOn::UiSessionData &data) {
        Q_UNUSED(data);
    }

    /*!
     * Refreshes given session.
     * Signond uses this slot to refresh data in given ui session.
     * Mostly used to refresh a captcha images during the user interaction.
     * Signal refreshed() or error() must be emitted when refresh is completed.
     * This must be reimplemented to refresh the captcha image.
     * @see UiSessionData
     * @see refreshed
     * @note emitting signal userActionRequired() is not allowed to use before ui session is finished.
     *
     * @param data Ui session data to be refreshed
     */
    virtual void refresh(const SignOn::UiSessionData &data) {
        emit refreshed(data);
    }

};

QT_BEGIN_NAMESPACE
 Q_DECLARE_INTERFACE(AuthPluginInterface,
                     "com.nokia.SingleSignOn.PluginInterface/1.3")
QT_END_NAMESPACE
#endif // AUTHPLUGINIF_H
