/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2012-2013 Canonical Ltd.
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

#ifndef SIGNONDAEMON_H_
#define SIGNONDAEMON_H_

extern "C" {
    #include <signal.h>
    #include <unistd.h>
    #include <errno.h>
    #include <stdio.h>
    #include <sys/types.h>
}

#include <QtCore>
#include <QtDBus>

#include "credentialsaccessmanager.h"

#ifndef SIGNOND_PLUGINS_DIR
    #define SIGNOND_PLUGINS_DIR "/usr/lib/signon"
#endif

#ifndef SIGNOND_PLUGIN_PREFIX
    #define SIGNOND_PLUGIN_PREFIX QLatin1String("lib")
#endif

#ifndef SIGNOND_PLUGIN_SUFFIX
    #define SIGNOND_PLUGIN_SUFFIX QLatin1String("plugin.so")
#endif

class QSocketNotifier;

namespace SignonDaemonNS {

/*!
 * @class SignonDaemonConfiguration
 * The daemon's configuration object;
 * loads date from the daemon configuration file.
 */
class SignonDaemonConfiguration
{
public:
    SignonDaemonConfiguration();
    ~SignonDaemonConfiguration();

    const CAMConfiguration &camConfiguration() const {
        return m_camConfiguration;
    }
    void setEncryptionPassphrase(const QByteArray &passphrase) {
        m_camConfiguration.m_encryptionPassphrase = passphrase;
    }

    void load();

    QString pluginsDir() const { return m_pluginsDir; }
    QString extensionsDir() const { return m_extensionsDir; }
    QString busAddress() const { return m_busAddress; }
    uint daemonTimeout() const { return m_daemonTimeout; }
    uint identityTimeout() const { return m_identityTimeout; }
    uint authSessionTimeout() const { return m_authSessionTimeout; }

private:
    QString m_pluginsDir;
    QString m_extensionsDir;
    QString m_busAddress;

    // storage configuration
    CAMConfiguration m_camConfiguration;

    //object timeouts
    uint m_daemonTimeout;
    uint m_identityTimeout;
    uint m_authSessionTimeout;
};

class SignonIdentity;

/*!
 * @class SignonDaemon
 * Daemon core.
 * @todo description.
 */
class SignonDaemon: public QObject, protected QDBusContext
{
    Q_OBJECT

    friend class SignonSessionCore;
    friend class SignonDaemonAdaptor;

public:
    static SignonDaemon *instance();
    virtual ~SignonDaemon();

    Q_INVOKABLE void init();

    /*!
     * Returns the number of seconds of inactivity after which identity
     * objects might be automatically deleted.
     */
    int identityTimeout() const;
    int authSessionTimeout() const;

public:
    QObject *registerNewIdentity();
    QObject *getIdentity(const quint32 id, QVariantMap &identityData);
    QObject *getAuthSession(const quint32 id, const QString type,
                            pid_t ownerPid);

    QStringList queryMethods();
    QStringList queryMechanisms(const QString &method);
    QList<QVariantMap> queryIdentities(const QVariantMap &filter);
    bool clear();

    QString lastErrorName() const { return m_lastErrorName; }
    QString lastErrorMessage() const { return m_lastErrorMessage; }
    bool lastErrorIsValid() const { return !m_lastErrorName.isEmpty(); }

private Q_SLOTS:
    void onDisconnected();
    void onNewConnection(const QDBusConnection &connection);
    void onIdentityStored(SignonIdentity *identity);
    void onIdentityDestroyed();

public Q_SLOTS: // backup METHODS
    uchar backupStarts();
    uchar backupFinished();
    uchar restoreStarts();
    uchar restoreFinished();

private:
    SignonDaemon(QObject *parent);
    void initExtensions();
    void initExtension(const QString &filePath);
    bool initStorage();

    void watchIdentity(SignonIdentity *identity);
    void setupSignalHandlers();

    void eraseBackupDir() const;
    bool copyToBackupDir(const QStringList &fileNames) const;
    bool copyFromBackupDir(const QStringList &fileNames) const;
    bool createStorageFileTree(const QStringList &fileNames) const;

    void setLastError(const QString &name, const QString &msg);
    void clearLastError();

private:
    /*
     * The list of created SignonIdentities
     * */
    QMap<quint32, SignonIdentity *> m_storedIdentities;

    SignonDaemonConfiguration *m_configuration;

    /*
     * The instance of CAM
     * */
    CredentialsAccessManager *m_pCAMManager;

    bool m_backup;

    int m_identityTimeout;
    int m_authSessionTimeout;

    QDBusServer *m_dbusServer;

    QString m_lastErrorName;
    QString m_lastErrorMessage;

    /*
     * UNIX signals handling related
     * */
public:
    static void signalHandler(int signal);
    Q_INVOKABLE void handleUnixSignal();

private:
    QSocketNotifier *m_sigSn;
    static SignonDaemon *m_instance;
}; //class SignonDaemon

} //namespace SignonDaemonNS

#endif /* SIGNONDAEMON_H_ */
