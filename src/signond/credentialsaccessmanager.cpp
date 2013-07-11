/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2011 Intel Corporation.
 *
 * Contact: Aurel Popirtac <mailto:ext-Aurel.Popirtac@nokia.com>
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

#define SIGNON_ENABLE_UNSTABLE_APIS
#include "credentialsaccessmanager.h"

#include "default-crypto-manager.h"
#include "default-key-authorizer.h"
#include "default-secrets-storage.h"
#include "signond-common.h"

#include "SignOn/ExtensionInterface"
#include "SignOn/misc.h"

#include <QFile>
#include <QBuffer>


#define RETURN_IF_NOT_INITIALIZED(return_value)                  \
    do {                                                         \
        if (!m_isInitialized) {                                  \
            m_error = NotInitialized;                            \
            TRACE() << "CredentialsAccessManager not initialized."; \
            return return_value;                                \
        }                                                       \
    } while (0)

using namespace SignonDaemonNS;
using namespace SignOn;

/* ---------------------- CAMConfiguration ---------------------- */

CAMConfiguration::CAMConfiguration():
    m_dbName(QLatin1String(signonDefaultDbName)),
    m_secretsDbName(QLatin1String(signonDefaultSecretsDbName)),
    m_encryptionPassphrase(QByteArray())
{
    setStoragePath(QLatin1String(signonDefaultStoragePath));
}

void CAMConfiguration::serialize(QIODevice *device)
{
    if (device == NULL)
        return;

    if (!device->open(QIODevice::ReadWrite)) {
        return;
    }

    QString buffer;
    QTextStream stream(&buffer);
    stream << "\n\n====== Credentials Access Manager Configuration ======\n\n";
    const char *usingEncryption = useEncryption() ? "true" : "false";
    stream << "Using encryption: " << usingEncryption << '\n';
    stream << "Metadata DB path: " << metadataDBPath() << '\n';
    stream << "Cryptomanager name: " << cryptoManagerName() << '\n';
    stream << "ACL manager name: " << accessControlManagerName() << '\n';
    stream << "Secrets storage name: " << secretsStorageName() << '\n';
    stream << "======================================================\n\n";
    device->write(buffer.toUtf8());
    device->close();
}

QString CAMConfiguration::metadataDBPath() const
{
    return m_storagePath + QDir::separator() + m_dbName;
}

QString CAMConfiguration::cryptoManagerName() const
{
    return m_settings.value(QLatin1String("CryptoManager"),
                            QLatin1String("default")).toString();
}

QString CAMConfiguration::accessControlManagerName() const
{
    return m_settings.value(QLatin1String("AccessControlManager"),
                            QLatin1String("default")).toString();
}

bool CAMConfiguration::useEncryption() const
{
    return cryptoManagerName() != QLatin1String("default");
}

QString CAMConfiguration::secretsStorageName() const
{
    return m_settings.value(QLatin1String("SecretsStorage"),
                            QLatin1String("default")).toString();
}

void CAMConfiguration::setStoragePath(const QString &storagePath) {
    m_storagePath = storagePath;
    if (m_storagePath.startsWith(QLatin1Char('~')))
        m_storagePath.replace(0, 1, QDir::homePath());
    // CryptoSetup extensions are given the m_settings dictionary only
    addSetting(QLatin1String("StoragePath"), m_storagePath);
}

/* ---------------------- CredentialsAccessManager ---------------------- */

CredentialsAccessManager *CredentialsAccessManager::m_pInstance = NULL;

CredentialsAccessManager::CredentialsAccessManager(
                                          const CAMConfiguration &configuration,
                                          QObject *parent):
    QObject(parent),
    m_isInitialized(false),
    m_systemOpened(false),
    m_error(NoError),
    keyManagers(),
    m_pCredentialsDB(NULL),
    m_cryptoManager(NULL),
    m_keyHandler(NULL),
    m_keyAuthorizer(NULL),
    m_secretsStorage(NULL),
    m_CAMConfiguration(configuration),
    m_acManager(NULL),
    m_acManagerHelper(NULL)
{
    if (!m_pInstance) {
        m_pInstance = this;
    } else {
        BLAME() << "Creating a second instance of the CAM";
    }

    m_keyHandler = new SignOn::KeyHandler(this);
}

CredentialsAccessManager::~CredentialsAccessManager()
{
    closeCredentialsSystem();

    m_pInstance = NULL;
}

CredentialsAccessManager *CredentialsAccessManager::instance()
{
    return m_pInstance;
}

void CredentialsAccessManager::finalize()
{
    TRACE() << "Enter";

    if (m_systemOpened)
        closeCredentialsSystem();

    // Disconnect all key managers
    foreach (SignOn::AbstractKeyManager *keyManager, keyManagers)
        keyManager->disconnect();

    m_isInitialized = false;
    m_error = NoError;
}

bool CredentialsAccessManager::init()
{
    if (m_isInitialized) {
        TRACE() << "CAM already initialized.";
        m_error = AlreadyInitialized;
        return false;
    }

    QBuffer config;
    m_CAMConfiguration.serialize(&config);
    TRACE() << "Initializing CredentialsAccessManager with configuration: " <<
        config.data();

    if (!createStorageDir()) {
        BLAME() << "Failed to create storage directory.";
        return false;
    }

    if (m_secretsStorage == 0) {
        QString name = m_CAMConfiguration.secretsStorageName();
        if (name != QLatin1String("default")) {
            BLAME() << "Couldn't load SecretsStorage:" << name;
        }
        TRACE() << "No SecretsStorage set, using default (dummy)";
        m_secretsStorage = new DefaultSecretsStorage(this);
    }

    //Initialize AccessControlManager
    if (m_acManager == 0) {
        QString name = m_CAMConfiguration.accessControlManagerName();
        if (name != QLatin1String("default")) {
            BLAME() << "Couldn't load AccessControlManager:" << name;
        }
        TRACE() << "No AccessControlManager set, using default (dummy)";
        m_acManager = new SignOn::AbstractAccessControlManager(this);
    }

    //Initialize AccessControlManagerHelper
    if (m_acManagerHelper == 0) {
        m_acManagerHelper = new AccessControlManagerHelper(m_acManager);
    }

    //Initialize CryptoManager
    if (m_cryptoManager == 0) {
        QString name = m_CAMConfiguration.cryptoManagerName();
        if (name != QLatin1String("default")) {
            BLAME() << "Couldn't load CryptoManager:" << name;
        }
        TRACE() << "No CryptoManager set, using default (dummy)";
        m_cryptoManager = new DefaultCryptoManager(this);
    }
    QObject::connect(m_cryptoManager, SIGNAL(fileSystemMounted()),
                     this, SLOT(onEncryptedFSMounted()));
    QObject::connect(m_cryptoManager, SIGNAL(fileSystemUnmounting()),
                     this, SLOT(onEncryptedFSUnmounting()));
    m_cryptoManager->initialize(m_CAMConfiguration.m_settings);

    /* This check is an optimization: instantiating the KeyAuthorizer is
     * probably not harmful if useEncryption() is false, but it's certainly
     * useless. */
    if (m_CAMConfiguration.useEncryption()) {
        if (m_keyAuthorizer == 0) {
            TRACE() << "No key authorizer set, using default";
            m_keyAuthorizer = new DefaultKeyAuthorizer(m_keyHandler, this);
        }
        QObject::connect(m_keyAuthorizer,
                         SIGNAL(keyAuthorizationQueried(const SignOn::Key,int)),
                         this,
                         SLOT(onKeyAuthorizationQueried(const SignOn::Key,int)));

        /* These signal connections should be done after instantiating the
         * KeyAuthorizer, so that the KeyAuthorizer's slot will be called
         * first (or we could connect to them in queued mode)
         */
        QObject::connect(m_keyHandler, SIGNAL(ready()),
                         this, SIGNAL(credentialsSystemReady()));
        QObject::connect(m_keyHandler, SIGNAL(keyInserted(SignOn::Key)),
                         this, SLOT(onKeyInserted(SignOn::Key)));
        QObject::connect(m_keyHandler,
                         SIGNAL(lastAuthorizedKeyRemoved(SignOn::Key)),
                         this,
                         SLOT(onLastAuthorizedKeyRemoved(SignOn::Key)));
        QObject::connect(m_keyHandler, SIGNAL(keyRemoved(SignOn::Key)),
                         this, SLOT(onKeyRemoved(SignOn::Key)));
        m_keyHandler->initialize(m_cryptoManager, keyManagers);
    }

    m_isInitialized = true;
    m_error = NoError;

    TRACE() << "CredentialsAccessManager successfully initialized...";
    return true;
}

void CredentialsAccessManager::addKeyManager(
    SignOn::AbstractKeyManager *keyManager)
{
    keyManagers.append(keyManager);
}

bool CredentialsAccessManager::initExtension(QObject *plugin)
{
    bool extensionInUse = false;

    SignOn::ExtensionInterface *extension;
    SignOn::ExtensionInterface2 *extension2;
    SignOn::ExtensionInterface3 *extension3;

    extension3 = qobject_cast<SignOn::ExtensionInterface3 *>(plugin);

    if (extension3 != 0)
        extension2 = extension3;
    else
        extension2 = qobject_cast<SignOn::ExtensionInterface2 *>(plugin);

    if (extension2 != 0)
        extension = extension2;
    else
        extension = qobject_cast<SignOn::ExtensionInterface *>(plugin);

    if (extension == 0) {
        qWarning() << "Plugin instance is not an ExtensionInterface";
        return false;
    }

    SignOn::AbstractKeyManager *keyManager = extension->keyManager(this);
    if (keyManager) {
        addKeyManager(keyManager);
        extensionInUse = true;
    }

    /* Check if the extension implements the new interface and provides a key
     * authorizer. */
    if (extension2 != 0) {
        SignOn::AbstractKeyAuthorizer *keyAuthorizer =
            extension2->keyAuthorizer(m_keyHandler, this);
        if (keyAuthorizer != 0) {
            if (m_keyAuthorizer == 0) {
                m_keyAuthorizer = keyAuthorizer;
                extensionInUse = true;
            } else {
                TRACE() << "Key authorizer already set";
                delete keyAuthorizer;
            }
        }
    }

    if (extension3 != 0) {
        /* Instantiate this plugin's CryptoManager only if it's the plugin
         * requested in the config file. */
        if (plugin->objectName() == m_CAMConfiguration.cryptoManagerName()) {
            SignOn::AbstractCryptoManager *cryptoManager =
                extension3->cryptoManager(this);
            if (cryptoManager != 0) {
                if (m_cryptoManager == 0) {
                    m_cryptoManager = cryptoManager;
                    extensionInUse = true;
                } else {
                    TRACE() << "Crypto manager already set";
                    delete cryptoManager;
                }
            }
        }

        if (plugin->objectName() == m_CAMConfiguration.secretsStorageName()) {
            SignOn::AbstractSecretsStorage *secretsStorage =
                extension3->secretsStorage(this);
            if (secretsStorage != 0) {
                if (m_secretsStorage == 0) {
                    m_secretsStorage = secretsStorage;
                    extensionInUse = true;
                } else {
                    TRACE() << "SecretsStorage already set";
                    delete secretsStorage;
                }
            }
        }

        /* Instantiate this plugin's AccessControlManager only if it's the
         * plugin requested in the config file. */
        if (plugin->objectName() ==
            m_CAMConfiguration.accessControlManagerName()) {
            SignOn::AbstractAccessControlManager *acManager =
                extension3->accessControlManager(this);
            if (acManager != 0) {
                if (m_acManager == 0) {
                    m_acManager = acManager;
                    extensionInUse = true;
                } else {
                    TRACE() << "Access control manager already set";
                    delete acManager;
                }
            }
        }
    }
    return extensionInUse;
}

QStringList CredentialsAccessManager::backupFiles() const
{
    QStringList files;

    files << m_cryptoManager->backupFiles();
    return files;
}

bool CredentialsAccessManager::openSecretsDB()
{
    if (!m_cryptoManager->fileSystemIsMounted()) {
        /* Do not attempt to mount the FS; we know that it will be mounted
         * automatically, as soon as some encryption keys are provided */
        m_error = CredentialsDbNotMounted;
        return false;
    }

    QString dbPath = m_cryptoManager->fileSystemMountPath()
        + QDir::separator()
        + m_CAMConfiguration.m_secretsDbName;

    TRACE() << "Database name: [" << dbPath << "]";

    if (!m_pCredentialsDB->openSecretsDB(dbPath))
        return false;

    m_error = NoError;
    return true;
}

bool CredentialsAccessManager::isSecretsDBOpen()
{
    return m_pCredentialsDB->isSecretsDBOpen();
}

bool CredentialsAccessManager::closeSecretsDB()
{
    m_pCredentialsDB->closeSecretsDB();

    if (!m_cryptoManager->unmountFileSystem()) {
        m_error = CredentialsDbUnmountFailed;
        return false;
    }

    return true;
}

bool CredentialsAccessManager::createStorageDir()
{
    QString dbPath = m_CAMConfiguration.metadataDBPath();

    QFileInfo fileInfo(dbPath);
    if (!fileInfo.exists()) {
        QDir storageDir(fileInfo.dir());
        if (!storageDir.mkpath(storageDir.path())) {
            BLAME() << "Could not create storage directory:" <<
                storageDir.path();
            m_error = CredentialsDbSetupFailed;
            return false;
        }
        setUserOwnership(storageDir.path());
    }
    return true;

}
bool CredentialsAccessManager::openMetaDataDB()
{
    QString dbPath = m_CAMConfiguration.metadataDBPath();

    m_pCredentialsDB = new CredentialsDB(dbPath, m_secretsStorage);

    if (!m_pCredentialsDB->init()) {
        m_error = CredentialsDbConnectionError;
        return false;
    }

    return true;
}

void CredentialsAccessManager::closeMetaDataDB()
{
    if (m_pCredentialsDB) {
        delete m_pCredentialsDB;
        m_pCredentialsDB = NULL;
    }
}

bool CredentialsAccessManager::openCredentialsSystem()
{
    RETURN_IF_NOT_INITIALIZED(false);

    if (!openMetaDataDB()) {
        BLAME() << "Couldn't open metadata DB!";
        return false;
    }

    m_systemOpened = true;

    if (m_cryptoManager->fileSystemIsMounted()) {
        if (!openSecretsDB()) {
            BLAME() << "Failed to open secrets DB.";
            /* Even if the secrets DB couldn't be opened, signond is still
             * usable: that's why we return "true" anyways. */
        }
    } else {
        /* The secrets DB will be opened as soon as the encrypted FS is
         * mounted.
         */
        m_cryptoManager->mountFileSystem();
    }

    return true;
}

bool CredentialsAccessManager::closeCredentialsSystem()
{
    RETURN_IF_NOT_INITIALIZED(false);

    if (!credentialsSystemOpened())
        return true;

    bool allClosed = true;
    if (isSecretsDBOpen() && !closeSecretsDB())
        allClosed = false;

    closeMetaDataDB();

    m_error = NoError;
    m_systemOpened = false;
    return allClosed;
}

bool CredentialsAccessManager::deleteCredentialsSystem()
{
    RETURN_IF_NOT_INITIALIZED(false);

    if (m_systemOpened && !closeCredentialsSystem()) {
        /* The close operation failed: we cannot proceed */
        return false;
    }

    BLAME() << "Not implemented";
    return false;
}

CredentialsDB *CredentialsAccessManager::credentialsDB() const
{
    RETURN_IF_NOT_INITIALIZED(NULL);

    return m_pCredentialsDB;
}

bool CredentialsAccessManager::isCredentialsSystemReady() const
{
    return (m_keyHandler != 0) ? m_keyHandler->isReady() : true;
}

void CredentialsAccessManager::onKeyInserted(const SignOn::Key key)
{
    TRACE() << "Key inserted.";

    if (!m_keyHandler->keyIsAuthorized(key))
        m_keyAuthorizer->queryKeyAuthorization(
            key, AbstractKeyAuthorizer::KeyInserted);
}

void CredentialsAccessManager::onLastAuthorizedKeyRemoved(const SignOn::Key key)
{
    Q_UNUSED(key);
    TRACE() << "All keys disabled. Closing secure storage.";
    if (isSecretsDBOpen() || m_cryptoManager->fileSystemIsMounted())
        if (!closeSecretsDB())
            BLAME() << "Error occurred while closing secure storage.";
}

void CredentialsAccessManager::onKeyRemoved(const SignOn::Key key)
{
    TRACE() << "Key removed.";

    if (m_keyHandler->keyIsAuthorized(key)) {
        if (!m_keyHandler->revokeKeyAuthorization(key)) {
            BLAME() << "Revoking key authorization failed";
        }
    }
}

void CredentialsAccessManager::onKeyAuthorizationQueried(const SignOn::Key key,
                                                         int result)
{
    TRACE() << "result:" << result;

    if (result != AbstractKeyAuthorizer::Denied) {
        KeyHandler::AuthorizeFlags flags = KeyHandler::None;
        if (result == AbstractKeyAuthorizer::Exclusive) {
            TRACE() << "Reformatting secure storage.";
            flags |= KeyHandler::FormatStorage;
        }

        if (!m_keyHandler->authorizeKey(key, flags)) {
            BLAME() << "Authorization failed";
        }
    }

    replyToSecureStorageEventNotifiers();
}

bool CredentialsAccessManager::keysAvailable() const
{
    if (m_keyHandler == 0) return false;
    return !m_keyHandler->insertedKeys().isEmpty();
}

void CredentialsAccessManager::replyToSecureStorageEventNotifiers()
{
    TRACE();
    //Notify secure storage notifiers if any.
    int eventType = SIGNON_SECURE_STORAGE_NOT_AVAILABLE;
    if ((m_pCredentialsDB != 0) && m_pCredentialsDB->isSecretsDBOpen())
        eventType = SIGNON_SECURE_STORAGE_AVAILABLE;

    // Signal objects that posted secure storage not available events
    foreach (EventSender object, m_secureStorageEventNotifiers) {
        if (object.isNull())
            continue;

        SecureStorageEvent *secureStorageEvent =
            new SecureStorageEvent((QEvent::Type)eventType);

        QCoreApplication::postEvent(
            object.data(),
            secureStorageEvent,
            Qt::HighEventPriority);
    }

    m_secureStorageEventNotifiers.clear();
}

void CredentialsAccessManager::customEvent(QEvent *event)
{
    TRACE() << "Custom event received.";
    if (event->type() != SIGNON_SECURE_STORAGE_NOT_AVAILABLE) {
        QObject::customEvent(event);
        return;
    }

    SecureStorageEvent *localEvent =
        static_cast<SecureStorageEvent *>(event);

    /* All senders of this event will receive a reply when
     * the secure storage becomes available or an error occurs. */
    m_secureStorageEventNotifiers.append(localEvent->m_sender);

    TRACE() << "Processing secure storage not available event.";
    if ((localEvent == 0) || (m_pCredentialsDB == 0)) {
        replyToSecureStorageEventNotifiers();
        QObject::customEvent(event);
        return;
    }

    //Double check if the secrets DB is indeed unavailable
    if (m_pCredentialsDB->isSecretsDBOpen()) {
        replyToSecureStorageEventNotifiers();
        QObject::customEvent(event);
        return;
    }

    SignOn::Key key; /* we don't specity any key */
    m_keyAuthorizer->queryKeyAuthorization(key,
                                           AbstractKeyAuthorizer::StorageNeeded);

    QObject::customEvent(event);
}

void CredentialsAccessManager::onEncryptedFSMounted()
{
    TRACE();
    if (!credentialsSystemOpened()) return;

    if (!isSecretsDBOpen()) {
        if (openSecretsDB()) {
            TRACE() << "Secrets DB opened.";
        } else {
            BLAME() << "Failed to open secrets DB.";
        }
    } else {
        BLAME() << "Secrets DB already opened?";
    }
}

void CredentialsAccessManager::onEncryptedFSUnmounting()
{
    TRACE();
    if (!credentialsSystemOpened()) return;

    if (isSecretsDBOpen()) {
        m_pCredentialsDB->closeSecretsDB();
    }
}
