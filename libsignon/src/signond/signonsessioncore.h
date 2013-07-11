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

#ifndef SIGNONSESSIONCORE_H_
#define SIGNONSESSIONCORE_H_

#include <QtCore>
#include <QtDBus>

/*
 * TODO: remove invocation of plugin operations into the main signond process
 */

#include "pluginproxy.h"
#include "signondisposable.h"
#include "signonsessioncoretools.h"

using namespace SignOn;

class SignonUiAdaptor;

namespace SignonDaemonNS {

class SignonDaemon;

/*!
 * @class SignonSessionCore
 * Daemon side representation of authentication session.
 * @todo description.
 */
class SignonSessionCore: public SignonDisposable
{
    Q_OBJECT

public:
    static SignonSessionCore *sessionCore(const quint32 id,
                                          const QString &method,
                                          SignonDaemon *parent);
    virtual ~SignonSessionCore();
    quint32 id() const;
    QString method() const;
    bool setupPlugin();
    /*
     * just for any case
     * */
    static void stopAllAuthSessions();
    static QStringList loadedPluginMethods(const QString &method);

    void destroy();

public Q_SLOTS:
    QStringList queryAvailableMechanisms(const QStringList &wantedMechanisms);

    void process(const QDBusConnection &connection,
                 const QDBusMessage &message,
                 const QVariantMap &sessionDataVa,
                 const QString &mechanism,
                 const QString &cancelKey);

    void cancel(const QString &cancelKey);
    void setId(quint32 id);

    /* When the credentials system is ready, session processing will begin.
     * This mechanism helps avoiding the display of eroneous secure storage
     * related messages on query credentials dialogs (e.g. The `No key present`
     * scenario - keys might actually be present but the querying of them is
     * not complete at the time of the auth. session processing).
     */
    void credentialsSystemReady();

Q_SIGNALS:
    void stateChanged(const QString &requestId,
                      int state,
                      const QString &message);

private Q_SLOTS:
    void startNewRequest();

    void processResultReply(const QVariantMap &data);
    void processStore(const QVariantMap &data);
    void processUiRequest(const QVariantMap &data);
    void processRefreshRequest(const QVariantMap &data);
    void processError(int err, const QString &message);
    void stateChangedSlot(int state,
                          const QString &message);

    void queryUiSlot(QDBusPendingCallWatcher *call);

protected:
    SignonSessionCore(quint32 id,
                      const QString &method,
                      int timeout,
                      QObject *parent);

    void childEvent(QChildEvent *ce);
    void customEvent(QEvent *event);

private:
    void startProcess();
    void replyError(const QDBusConnection &conn,
                    const QDBusMessage &msg,
                    int err,
                    const QString &message);
    void processStoreOperation(const StoreOperation &operation);
    void requestDone();

private:
    PluginProxy *m_plugin;
    QQueue<RequestData> m_listOfRequests;
    SignonUiAdaptor *m_signonui;

    QDBusPendingCallWatcher *m_watcher;

    bool m_requestIsActive;
    bool m_canceled;

    uint m_id;
    QString m_method;
    /* the original request parameters, for the request currently being
     * processed */
    QVariantMap m_clientData;

    //Temporary caching
    QString m_tmpUsername;
    QString m_tmpPassword;

    /* Flag used for handling post ui querying results' processing.
     * Secure storage not available events won't be posted if the current
     * session processing was not preceded by a signon UI query credentials
     * interaction, when this flag is set to true. */
    bool m_queryCredsUiDisplayed;

    Q_DISABLE_COPY(SignonSessionCore)
}; //class SignonDaemon

} //namespace SignonDaemonNS

#endif //SIGNONSESSIONQUEUE_H_
