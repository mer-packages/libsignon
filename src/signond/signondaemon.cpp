/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2013 Canonical Ltd.
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

extern "C" {
    #include <sys/socket.h>
    #include <sys/stat.h>
    #include <sys/types.h>
}

#define QT_DISABLE_DEPRECATED_BEFORE QT_VERSION_CHECK(4, 0, 0)

#include <QtDebug>
#include <QDir>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QPluginLoader>
#include <QProcessEnvironment>
#include <QSocketNotifier>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QStandardPaths>
#endif

#include "SignOn/misc.h"

#include "signondaemon.h"
#include "signond-common.h"
#include "signontrace.h"
#include "signondaemonadaptor.h"
#include "signonidentity.h"
#include "signonauthsession.h"
#include "accesscontrolmanagerhelper.h"
#include "backupifadaptor.h"

#define SIGNON_RETURN_IF_CAM_UNAVAILABLE(_ret_arg_) do {                   \
        if (m_pCAMManager && !m_pCAMManager->credentialsSystemOpened()) {  \
            setLastError(internalServerErrName,                            \
                         internalServerErrStr +                            \
                         QLatin1String("Could not access Signon "          \
                                       "Database."));                      \
            return _ret_arg_;           \
        }                               \
    } while(0)

#define BACKUP_DIR_NAME() \
    (QDir::separator() + QLatin1String("backup"))

using namespace SignOn;

namespace SignonDaemonNS {

/* ---------------------- SignonDaemonConfiguration ---------------------- */

SignonDaemonConfiguration::SignonDaemonConfiguration():
    m_pluginsDir(QLatin1String(SIGNOND_PLUGINS_DIR)),
    m_extensionsDir(QLatin1String(SIGNOND_EXTENSIONS_DIR)),
    m_camConfiguration(),
    m_daemonTimeout(0), // 0 = no timeout
    m_identityTimeout(300),//secs
    m_authSessionTimeout(300)//secs
{}

SignonDaemonConfiguration::~SignonDaemonConfiguration()
{
    TRACE();
}

/*
    --- Configuration file template ---

    [General]
    UseSecureStorage=yes
    StoragePath=~/.signon/
    ;0 - fatal, 1 - critical(default), 2 - info/debug
    LoggingLevel=1

    [SecureStorage]
    FileSystemName=signonfs
    Size=8
    FileSystemType=ext2

    [ObjectTimeouts]
    IdentityTimeout=300
    AuthSessionTimeout=300
 */
void SignonDaemonConfiguration::load()
{
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();

    //Daemon configuration file

    QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope,
                       environment.value(QLatin1String("SSO_CONFIG_FILE_DIR"),
                                         QLatin1String("/etc")));

    QSettings settings(QLatin1String("signond"));

    int loggingLevel =
        settings.value(QLatin1String("LoggingLevel"), 1).toInt();
    setLoggingLevel(loggingLevel);

    QString cfgStoragePath =
        settings.value(QLatin1String("StoragePath")).toString();
    if (!cfgStoragePath.isEmpty()) {
        QString storagePath = QDir(cfgStoragePath).path();
        m_camConfiguration.setStoragePath(storagePath);
    } else {
        QString xdgConfigHome = QLatin1String(qgetenv("XDG_CONFIG_HOME"));
        if (xdgConfigHome.isEmpty())
            xdgConfigHome = QDir::homePath() + QLatin1String("/.config");
        m_camConfiguration.setStoragePath(xdgConfigHome +
                                          QLatin1String("/signond"));
    }

    // Secure storage

    // Support legacy setting "UseSecureStorage"
    QString useSecureStorage =
        settings.value(QLatin1String("UseSecureStorage")).toString();
    if (useSecureStorage == QLatin1String("yes") ||
        useSecureStorage == QLatin1String("true")) {
        m_camConfiguration.addSetting(QLatin1String("CryptoManager"),
                                      QLatin1String("cryptsetup"));
    }

    settings.beginGroup(QLatin1String("SecureStorage"));

    QVariantMap storageOptions;
    foreach (const QString &key, settings.childKeys()) {
        m_camConfiguration.addSetting(key, settings.value(key));
    }

    settings.endGroup();

    //Timeouts
    settings.beginGroup(QLatin1String("ObjectTimeouts"));

    bool isOk = false;
    uint aux = settings.value(QLatin1String("IdentityTimeout")).toUInt(&isOk);
    if (isOk)
        m_identityTimeout = aux;

    aux = settings.value(QLatin1String("AuthSessionTimeout")).toUInt(&isOk);
    if (isOk)
        m_authSessionTimeout = aux;

    aux = settings.value(QLatin1String("DaemonTimeout")).toUInt(&isOk);
    if (isOk)
        m_daemonTimeout = aux;

    settings.endGroup();

    //Environment variables

    int value = 0;
    if (environment.contains(QLatin1String("SSO_DAEMON_TIMEOUT"))) {
        value = environment.value(
            QLatin1String("SSO_DAEMON_TIMEOUT")).toInt(&isOk);
        if (value > 0 && isOk) m_daemonTimeout = value;
    }

    if (environment.contains(QLatin1String("SSO_IDENTITY_TIMEOUT"))) {
        value = environment.value(
            QLatin1String("SSO_IDENTITY_TIMEOUT")).toInt(&isOk);
        if (value > 0 && isOk) m_identityTimeout = value;
    }

    if (environment.contains(QLatin1String("SSO_AUTHSESSION_TIMEOUT"))) {
        value = environment.value(
            QLatin1String("SSO_AUTHSESSION_TIMEOUT")).toInt(&isOk);
        if (value > 0 && isOk) m_authSessionTimeout = value;
    }

    if (environment.contains(QLatin1String("SSO_LOGGING_LEVEL"))) {
        value = environment.value(
            QLatin1String("SSO_LOGGING_LEVEL")).toInt(&isOk);
        if (isOk)
            setLoggingLevel(value);
    }

    QString logOutput = environment.value(QLatin1String("SSO_LOGGING_OUTPUT"),
                                          QLatin1String("syslog"));
    SignonTrace::initialize(logOutput == QLatin1String("syslog") ?
                            SignonTrace::Syslog : SignonTrace::Stdout);

    if (environment.contains(QLatin1String("SSO_STORAGE_PATH"))) {
        m_camConfiguration.setStoragePath(
            environment.value(QLatin1String("SSO_STORAGE_PATH")));
    }

    if (environment.contains(QLatin1String("SSO_PLUGINS_DIR"))) {
        m_pluginsDir = environment.value(QLatin1String("SSO_PLUGINS_DIR"));
    }

    if (environment.contains(QLatin1String("SSO_EXTENSIONS_DIR"))) {
        m_extensionsDir =
            environment.value(QLatin1String("SSO_EXTENSIONS_DIR"));
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QString runtimeDir =
        QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
#else
    QString runtimeDir = environment.value(QLatin1String("XDG_RUNTIME_DIR"));
#endif
    if (!runtimeDir.isEmpty()) {
        QString socketFileName =
            QString::fromLatin1("%1/" SIGNOND_SOCKET_FILENAME).arg(runtimeDir);
        QDir socketDir = QFileInfo(socketFileName).absoluteDir();
        if (!socketDir.exists() && !socketDir.mkpath(socketDir.path())) {
            BLAME() << "Cannot create socket directory" << socketDir;
        } else {
            m_busAddress =
                QString::fromLatin1("unix:path=%1").arg(socketFileName);
        }
    } else {
        BLAME() << "XDG_RUNTIME_DIR unset, disabling p2p bus";
    }
}

/* ---------------------- SignonDaemon ---------------------- */

const QString internalServerErrName = SIGNOND_INTERNAL_SERVER_ERR_NAME;
const QString internalServerErrStr = SIGNOND_INTERNAL_SERVER_ERR_STR;

static int sigFd[2];

SignonDaemon *SignonDaemon::m_instance = NULL;

SignonDaemon::SignonDaemon(QObject *parent):
    QObject(parent),
    m_configuration(0),
    m_pCAMManager(0),
    m_dbusServer(0)
{
    // Files created by signond must be unreadable by "other"
    umask(S_IROTH | S_IWOTH);

    // Register D-Bus meta types
    qDBusRegisterMetaType<MethodMap>();
    qDBusRegisterMetaType<MapList>();
}

SignonDaemon::~SignonDaemon()
{
    ::close(sigFd[0]);
    ::close(sigFd[1]);

    if (m_backup) {
        exit(0);
    }

    SignonAuthSession::stopAllAuthSessions();
    m_storedIdentities.clear();

    if (m_pCAMManager) {
        m_pCAMManager->closeCredentialsSystem();
        delete m_pCAMManager;
    }

    QDBusConnection sessionConnection = QDBusConnection::sessionBus();

    sessionConnection.unregisterObject(SIGNOND_DAEMON_OBJECTPATH
                                       + QLatin1String("/Backup"));
    sessionConnection.unregisterService(SIGNOND_SERVICE
                                        + QLatin1String(".Backup"));
    if (m_backup == false)
    {
        sessionConnection.unregisterObject(SIGNOND_DAEMON_OBJECTPATH);
        sessionConnection.unregisterService(SIGNOND_SERVICE);
    }

    delete m_configuration;

    QMetaObject::invokeMethod(QCoreApplication::instance(),
                              "quit",
                              Qt::QueuedConnection);
}

void SignonDaemon::setupSignalHandlers()
{
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigFd) != 0)
        BLAME() << "Couldn't create HUP socketpair";

    m_sigSn = new QSocketNotifier(sigFd[1], QSocketNotifier::Read, this);
    connect(m_sigSn, SIGNAL(activated(int)),
            this, SLOT(handleUnixSignal()));
}

void SignonDaemon::signalHandler(int signal)
{
    int ret = ::write(sigFd[0], &signal, sizeof(signal));
    Q_UNUSED(ret);
}

void SignonDaemon::handleUnixSignal()
{
    m_sigSn->setEnabled(false);

    int signal;
    int ret = read(sigFd[1], &signal, sizeof(signal));
    Q_UNUSED(ret);

    TRACE() << "signal received: " << signal;

    switch (signal) {
        case SIGHUP: {
            TRACE() << "\n\n SIGHUP \n\n";
            //todo restart daemon
            deleteLater();

            // reset the m_instance
            m_instance = NULL;
            QMetaObject::invokeMethod(instance(),
                                      "init",
                                      Qt::QueuedConnection);
            break;
        }
        case SIGTERM: {
            TRACE() << "\n\n SIGTERM \n\n";
            //gently stop daemon
            deleteLater();
            QMetaObject::invokeMethod(QCoreApplication::instance(),
                                      "quit",
                                      Qt::QueuedConnection);
            break;
        }
        case SIGINT:  {
            TRACE() << "\n\n SIGINT \n\n";
            //gently stop daemon
            deleteLater();
            QMetaObject::invokeMethod(QCoreApplication::instance(),
                                      "quit",
                                      Qt::QueuedConnection);
            break;
        }
        default: break;
    }

    m_sigSn->setEnabled(true);
}

SignonDaemon *SignonDaemon::instance()
{
    if (m_instance != NULL)
        return m_instance;

    QCoreApplication *app = QCoreApplication::instance();

    if (!app)
        qFatal("SignonDaemon requires a QCoreApplication instance to be "
               "constructed first");

    TRACE() << "Creating new daemon instance.";
    m_instance = new SignonDaemon(app);
    return m_instance;
}

void SignonDaemon::init()
{
    if (!(m_configuration = new SignonDaemonConfiguration))
        qWarning("SignonDaemon could not create the configuration object.");

    m_configuration->load();

    if (getuid() != 0) {
        BLAME() << "Failed to SUID root. Secure storage will not be available.";
    }

    QCoreApplication *app = QCoreApplication::instance();
    if (!app)
        qFatal("SignonDaemon requires a QCoreApplication instance to be "
               "constructed first");

    setupSignalHandlers();
    m_backup = app->arguments().contains(QLatin1String("-backup"));
    m_pCAMManager =
        new CredentialsAccessManager(m_configuration->camConfiguration());

    /* backup dbus interface */
    QDBusConnection sessionConnection = QDBusConnection::sessionBus();

    if (!sessionConnection.isConnected()) {
        QDBusError err = sessionConnection.lastError();
        TRACE() << "Session connection cannot be established:" <<
            err.errorString(err.type());
        TRACE() << err.message();

        qFatal("SignonDaemon requires session bus to start working");
    }

    QDBusConnection::RegisterOptions registerSessionOptions =
        QDBusConnection::ExportAdaptors;

    (void)new BackupIfAdaptor(this);

    if (!sessionConnection.registerObject(SIGNOND_DAEMON_OBJECTPATH
                                          + QLatin1String("/Backup"),
                                          this, registerSessionOptions)) {
        TRACE() << "Object cannot be registered";

        qFatal("SignonDaemon requires to register backup object");
    }

    if (!sessionConnection.registerService(SIGNOND_SERVICE +
                                           QLatin1String(".Backup"))) {
        QDBusError err = sessionConnection.lastError();
        TRACE() << "Service cannot be registered: " <<
            err.errorString(err.type());

        qFatal("SignonDaemon requires to register backup service");
    }

    if (m_backup) {
        TRACE() << "Signond initialized in backup mode.";
        //skip rest of initialization in backup mode
        return;
    }

    /* DBus Service init */
    QDBusConnection connection = SIGNOND_BUS;

    if (!connection.isConnected()) {
        QDBusError err = connection.lastError();
        TRACE() << "Connection cannot be established:" <<
            err.errorString(err.type());
        TRACE() << err.message();

        qFatal("SignonDaemon requires DBus to start working");
    }

    QDBusConnection::RegisterOptions registerOptions =
        QDBusConnection::ExportAllContents;

    (void)new SignonDaemonAdaptor(this);
    registerOptions = QDBusConnection::ExportAdaptors;

    // p2p connection
#ifdef ENABLE_P2P
    m_dbusServer = new QDBusServer(m_configuration->busAddress(), this);
    QObject::connect(m_dbusServer,
                     SIGNAL(newConnection(const QDBusConnection &)),
                     this, SLOT(onNewConnection(const QDBusConnection &)));
#endif

    // session bus
    if (!connection.registerObject(SIGNOND_DAEMON_OBJECTPATH,
                                   this, registerOptions)) {
        TRACE() << "Object cannot be registered";

        qFatal("SignonDaemon requires to register daemon's object");
    }

    if (!connection.registerService(SIGNOND_SERVICE)) {
        QDBusError err = connection.lastError();
        TRACE() << "Service cannot be registered: " <<
            err.errorString(err.type());

        qFatal("SignonDaemon requires to register daemon's service");
    }

    // handle D-Bus disconnection
    connection.connect(QString(),
                       QLatin1String("/org/freedesktop/DBus/Local"),
                       QLatin1String("org.freedesktop.DBus.Local"),
                       QLatin1String("Disconnected"),
                       this, SLOT(onDisconnected()));

    initExtensions();

    if (!initStorage())
        BLAME() << "Signond: Cannot initialize credentials storage.";

    if (m_configuration->daemonTimeout() > 0) {
        SignonDisposable::invokeOnIdle(m_configuration->daemonTimeout(),
                                       this, SLOT(deleteLater()));
    }

    TRACE() << "Signond SUCCESSFULLY initialized.";
}

void SignonDaemon::onNewConnection(const QDBusConnection &connection)
{
    TRACE() << "New p2p connection" << connection.name();
    QDBusConnection conn(connection);
    if (!conn.registerObject(SIGNOND_DAEMON_OBJECTPATH,
                             this, QDBusConnection::ExportAdaptors)) {
        qFatal("Failed to register SignonDaemon object");
    }
}

void SignonDaemon::initExtensions()
{
    /* Scan the directory containing signond extensions and attempt loading
     * all of them.
     */
    QDir dir(m_configuration->extensionsDir());
    QStringList filters(QLatin1String("lib*.so"));
    QStringList extensionList = dir.entryList(filters, QDir::Files);
    foreach(const QString &filename, extensionList)
        initExtension(dir.filePath(filename));
}

void SignonDaemon::initExtension(const QString &filePath)
{
    TRACE() << "Loading plugin " << filePath;

    QPluginLoader pluginLoader(filePath);
    QObject *plugin = pluginLoader.instance();
    if (!plugin) {
        qWarning() << "Couldn't load plugin:" << pluginLoader.errorString();
        return;
    }

    /* Check whether the extension implements some useful objects; if not,
     * unload it. */
    if (!m_pCAMManager->initExtension(plugin))
        pluginLoader.unload();
}

bool SignonDaemon::initStorage()
{
    if (!m_pCAMManager->credentialsSystemOpened()) {
        m_pCAMManager->finalize();

        if (!m_pCAMManager->init()) {
            BLAME() << "CAM initialization failed";
            return false;
        }

        // If encryption is in use this will just open the metadata DB
        if (!m_pCAMManager->openCredentialsSystem()) {
            qCritical("Signond: Cannot open CAM credentials system...");
            return false;
        }
    } else {
        TRACE() << "Secure storage already initialized...";
        return false;
    }

    return true;
}

void SignonDaemon::onIdentityStored(SignonIdentity *identity)
{
    m_storedIdentities.insert(identity->id(), identity);
}

void SignonDaemon::onIdentityDestroyed()
{
    SignonIdentity *identity = qobject_cast<SignonIdentity*>(sender());
    m_storedIdentities.remove(identity->id());
}

void SignonDaemon::watchIdentity(SignonIdentity *identity)
{
    QObject::connect(identity, SIGNAL(stored(SignonIdentity*)),
                     this, SLOT(onIdentityStored(SignonIdentity*)));
    QObject::connect(identity, SIGNAL(unregistered()),
                     this, SLOT(onIdentityDestroyed()));

    if (identity->id() != SIGNOND_NEW_IDENTITY) {
        m_storedIdentities.insert(identity->id(), identity);
    }
}

QObject *SignonDaemon::registerNewIdentity()
{
    clearLastError();

    TRACE() << "Registering new identity:";

    SignonIdentity *identity =
        SignonIdentity::createIdentity(SIGNOND_NEW_IDENTITY, this);

    Q_ASSERT(identity != NULL);
    watchIdentity(identity);

    return identity;
}

int SignonDaemon::identityTimeout() const
{
    return (m_configuration == NULL ?
                                     300 :
                                     m_configuration->identityTimeout());
}

int SignonDaemon::authSessionTimeout() const
{
    return (m_configuration == NULL ?
                                     300 :
                                     m_configuration->authSessionTimeout());
}

QObject *SignonDaemon::getIdentity(const quint32 id,
                                   QVariantMap &identityData)
{
    clearLastError();

    SIGNON_RETURN_IF_CAM_UNAVAILABLE(0);

    TRACE() << "Registering identity:" << id;

    //1st check if the existing identity is in cache
    SignonIdentity *identity = m_storedIdentities.value(id, NULL);

    //if not create it
    if (identity == NULL)
        identity = SignonIdentity::createIdentity(id, this);
    Q_ASSERT(identity != NULL);

    bool ok;
    SignonIdentityInfo info = identity->queryInfo(ok, false);

    if (info.isNew())
    {
        setLastError(SIGNOND_IDENTITY_NOT_FOUND_ERR_NAME,
                     SIGNOND_IDENTITY_NOT_FOUND_ERR_STR);
        identity->destroy();
        return 0;
    }

    watchIdentity(identity);
    identity->keepInUse();

    identityData = info.toMap();

    TRACE() << "DONE REGISTERING IDENTITY";
    return identity;
}

QStringList SignonDaemon::queryMethods()
{
    QDir pluginsDir(m_configuration->pluginsDir());
    //TODO: in the future remove the sym links comment
    QStringList fileNames = pluginsDir.entryList(
            QStringList() << QLatin1String("*.so*"),
            QDir::Files | QDir::NoDotAndDotDot);

    QStringList ret;
    QString fileName;
    foreach (fileName, fileNames) {
        if (fileName.startsWith(QLatin1String("lib"))) {
            fileName =
                fileName.mid(3, fileName.indexOf(QLatin1String("plugin")) -3);
            if ((fileName.length() > 0) && !ret.contains(fileName))
                ret << fileName;
        }
    }

    return ret;
}

QStringList SignonDaemon::queryMechanisms(const QString &method)
{
    clearLastError();

    TRACE() << method;

    QStringList mechs = SignonSessionCore::loadedPluginMethods(method);

    if (!mechs.isEmpty())
        return mechs;

    PluginProxy *plugin = PluginProxy::createNewPluginProxy(method);

    if (!plugin) {
        TRACE() << "Could not load plugin of type: " << method;
        setLastError(SIGNOND_METHOD_NOT_KNOWN_ERR_NAME,
                     SIGNOND_METHOD_NOT_KNOWN_ERR_STR +
                     QString::fromLatin1("Method %1 is not known or could "
                                         "not load specific configuration.").
                     arg(method));
        return QStringList();
    }

    mechs = plugin->mechanisms();
    delete plugin;

    return mechs;
}

QList<QVariantMap> SignonDaemon::queryIdentities(const QVariantMap &filter)
{
    clearLastError();

    SIGNON_RETURN_IF_CAM_UNAVAILABLE(QList<QVariantMap>());

    TRACE() << "Querying identities";

    CredentialsDB *db = m_pCAMManager->credentialsDB();
    if (!db) {
        qCritical() << Q_FUNC_INFO << m_pCAMManager->lastError();
        return QList<QVariantMap>();
    }

    QMap<QString, QString> filterLocal;
    QMapIterator<QString, QVariant> it(filter);
    while (it.hasNext()) {
        it.next();
        filterLocal.insert(it.key(), it.value().toString());
    }

    QList<SignonIdentityInfo> credentials = db->credentials(filterLocal);

    if (db->errorOccurred()) {
        setLastError(internalServerErrName,
                     internalServerErrStr +
                     QLatin1String("Querying database error occurred."));
        return QList<QVariantMap>();
    }

    QList<QVariantMap> mapList;
    foreach (const SignonIdentityInfo &info, credentials) {
        mapList.append(info.toMap());
    }
    return mapList;
}

bool SignonDaemon::clear()
{
    clearLastError();

    SIGNON_RETURN_IF_CAM_UNAVAILABLE(false);

    TRACE() << "\n\n\n Clearing DB\n\n";
    CredentialsDB *db = m_pCAMManager->credentialsDB();
    if (!db) {
        qCritical() << Q_FUNC_INFO << m_pCAMManager->lastError();
        return false;
    }

    if (!db->clear()) {
        setLastError(SIGNOND_INTERNAL_SERVER_ERR_NAME,
                     SIGNOND_INTERNAL_SERVER_ERR_STR +
                     QLatin1String("Database error occurred."));
        return false;
    }
    return true;
}

QObject *SignonDaemon::getAuthSession(const quint32 id,
                                      const QString type,
                                      pid_t ownerPid)
{
    clearLastError();

    SignonAuthSession *authSession =
        SignonAuthSession::createAuthSession(id, type, this, ownerPid);
    if (authSession == NULL) {
        setLastError(SIGNOND_METHOD_NOT_KNOWN_ERR_NAME,
                     SIGNOND_METHOD_NOT_KNOWN_ERR_STR);
        return 0;
    }

    return authSession;
}

void SignonDaemon::eraseBackupDir() const
{
    const CAMConfiguration config = m_configuration->camConfiguration();
    QString backupRoot = config.m_storagePath + BACKUP_DIR_NAME();

    QDir target(backupRoot);
    if (!target.exists()) return;

    QStringList targetEntries = target.entryList(QDir::Files);
    foreach (QString entry, targetEntries) {
        target.remove(entry);
    }

    target.rmdir(backupRoot);
}

bool SignonDaemon::copyToBackupDir(const QStringList &fileNames) const
{
    const CAMConfiguration config = m_configuration->camConfiguration();
    QString backupRoot = config.m_storagePath + BACKUP_DIR_NAME();

    QDir target(backupRoot);
    if (!target.exists() && !target.mkpath(backupRoot)) {
        qCritical() << "Cannot create target directory";
        return false;
    }

    setUserOwnership(backupRoot);

    /* Now copy the files to be backed up */
    bool ok = true;
    foreach (const QString &fileName, fileNames) {
        /* Remove the target file, if it exists */
        if (target.exists(fileName))
            target.remove(fileName);

        /* Copy the source into the target directory */
        QString source = config.m_storagePath + QDir::separator() + fileName;
        if (!QFile::exists(source)) continue;

        QString destination = backupRoot + QDir::separator() + fileName;
        ok = QFile::copy(source, destination);
        if (!ok) {
            BLAME() << "Copying" << source << "to" << destination << "failed";
            break;
        }

        setUserOwnership(destination);
    }

    return ok;
}

bool SignonDaemon::copyFromBackupDir(const QStringList &fileNames) const
{
    const CAMConfiguration config = m_configuration->camConfiguration();
    QString backupRoot = config.m_storagePath + BACKUP_DIR_NAME();

    QDir sourceDir(backupRoot);
    if (!sourceDir.exists()) {
        TRACE() << "Backup directory does not exist!";
    }

    if (!sourceDir.exists(config.m_dbName)) {
        TRACE() << "Backup does not contain DB:" << config.m_dbName;
    }

    /* Now restore the files from the backup */
    bool ok = true;
    QDir target(config.m_storagePath);
    QStringList movedFiles, copiedFiles;
    foreach (const QString &fileName, fileNames) {
        /* Remove the target file, if it exists */
        if (target.exists(fileName)) {
            if (target.rename(fileName, fileName + QLatin1String(".bak")))
                movedFiles += fileName;
        }

        /* Copy the source into the target directory */
        QString source = backupRoot + QDir::separator() + fileName;
        if (!QFile::exists(source)) {
            TRACE() << "Ignoring file not present in backup:" << source;
            continue;
        }

        QString destination =
            config.m_storagePath + QDir::separator() + fileName;

        ok = QFile::copy(source, destination);
        if (ok) {
            copiedFiles << fileName;
        } else {
            qWarning() << "Copy failed for:" << source;
            break;
        }
    }

    if (!ok) {
        qWarning() << "Restore failed, recovering previous DB";

        foreach (const QString &fileName, copiedFiles) {
            target.remove(fileName);
        }

        foreach (const QString &fileName, movedFiles) {
            if (!target.rename(fileName + QLatin1String(".bak"), fileName)) {
                qCritical() << "Could not recover:" << fileName;
            }
        }
    } else {
        /* delete ".bak" files */
        foreach (const QString &fileName, movedFiles) {
            target.remove(fileName + QLatin1String(".bak"));
        }

    }
    return ok;
}

bool SignonDaemon::createStorageFileTree(const QStringList &backupFiles) const
{
    QString storageDirPath = m_configuration->camConfiguration().m_storagePath;
    QDir storageDir(storageDirPath);

    if (!storageDir.exists()) {
        if (!storageDir.mkpath(storageDirPath)) {
            qCritical() << "Could not create storage dir for backup.";
            return false;
        }
    }

    foreach (const QString &fileName, backupFiles) {
        if (storageDir.exists(fileName)) continue;

        QString filePath = storageDir.path() + QDir::separator() + fileName;
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qCritical() << "Failed to create empty file for backup:" << filePath;
            return false;
        } else {
            file.close();
        }
    }

    return true;
}

uchar SignonDaemon::backupStarts()
{
    TRACE() << "backup";
    if (!m_backup && m_pCAMManager->credentialsSystemOpened())
    {
        m_pCAMManager->closeCredentialsSystem();
        if (m_pCAMManager->credentialsSystemOpened())
        {
            qCritical() << "Cannot close credentials database";
            return 2;
        }
    }

    const CAMConfiguration config = m_configuration->camConfiguration();

    /* do backup copy: prepare the list of files to be backed up */
    QStringList backupFiles;
    backupFiles << config.m_dbName;
    backupFiles << m_pCAMManager->backupFiles();

    /* make sure that all the backup files and storage directory exist:
       create storage dir and empty files if not so, as backup/restore
       operations must be consistent */
    if (!createStorageFileTree(backupFiles)) {
        qCritical() << "Cannot create backup file tree.";
        return 2;
    }

    /* perform the copy */
    eraseBackupDir();
    if (!copyToBackupDir(backupFiles)) {
        qCritical() << "Cannot copy database";
        if (!m_backup)
            m_pCAMManager->openCredentialsSystem();
        return 2;
    }

    if (!m_backup)
    {
        //mount file system back
        if (!m_pCAMManager->openCredentialsSystem()) {
            qCritical() << "Cannot reopen database";
        }
    }
    return 0;
}

uchar SignonDaemon::backupFinished()
{
    TRACE() << "close";

    eraseBackupDir();

    if (m_backup)
    {
        //close daemon
        TRACE() << "close daemon";
        this->deleteLater();
    }

    return 0;
 }

/*
 * Does nothing but start-on-demand
 * */
uchar SignonDaemon::restoreStarts()
{
    TRACE();
    return 0;
}

uchar SignonDaemon::restoreFinished()
{
    TRACE() << "restore";
    //restore requested
    if (m_pCAMManager->credentialsSystemOpened())
    {
        //umount file system
        if (!m_pCAMManager->closeCredentialsSystem())
        {
            qCritical() << "database cannot be closed";
            return 2;
        }
    }

    const CAMConfiguration config = m_configuration->camConfiguration();

    QStringList backupFiles;
    backupFiles << config.m_dbName;
    backupFiles << m_pCAMManager->backupFiles();

    /* perform the copy */
    if (!copyFromBackupDir(backupFiles)) {
        qCritical() << "Cannot copy database";
        m_pCAMManager->openCredentialsSystem();
        return 2;
    }

    eraseBackupDir();

    //TODO check database integrity
    if (!m_backup)
    {
        //mount file system back
         if (!m_pCAMManager->openCredentialsSystem())
             return 2;
    }

    return 0;
}

void SignonDaemon::onDisconnected()
{
    TRACE() << "Disconnected from session bus: exiting";
    this->deleteLater();
    QMetaObject::invokeMethod(QCoreApplication::instance(),
                              "quit",
                              Qt::QueuedConnection);
}

void SignonDaemon::setLastError(const QString &name, const QString &msg)
{
    m_lastErrorName = name;
    m_lastErrorMessage = msg;
}

void SignonDaemon::clearLastError()
{
    m_lastErrorName = QString();
    m_lastErrorMessage = QString();
}

} //namespace SignonDaemonNS
