/*
 * This file is part of signon
 *
 * Copyright (C) 2013 Canonical Ltd.
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

#include "access-control-manager.h"

#include <QByteArray>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QFile>
#include <QProcess>
#ifdef ENABLE_P2P
#include <dbus/dbus.h>
#endif

static const char keychainAppId[] = "SignondKeychain";

class AccessReply: public SignOn::AccessReply
{
    Q_OBJECT

public:
    AccessReply(const SignOn::AccessRequest &request,
                const QString &appId,
                QObject *parent = 0);

private Q_SLOTS:
    void onProcessFinished();

private:
    QProcess m_helper;
};

AccessReply::AccessReply(const SignOn::AccessRequest &request,
                         const QString &appId,
                         QObject *parent):
    SignOn::AccessReply(request, parent)
{
    /* Spawn the IDENTITY_AC_HELPER process, which will decide whether access
     * to the identity should be allowed (based on some fields found in the
     * identity itself), and will exit with code 1 if the access has been
     * allowed.
     */
    QObject::connect(&m_helper,
                     SIGNAL(finished(int,QProcess::ExitStatus)),
                     this, SLOT(onProcessFinished()));
    m_helper.setProcessChannelMode(QProcess::ForwardedChannels);
    QStringList args;
    args << QString::number(request.identity());
    args << appId;
    m_helper.start(IDENTITY_AC_HELPER, args);
}

void AccessReply::onProcessFinished()
{
    qDebug() << Q_FUNC_INFO << m_helper.exitStatus() << m_helper.exitCode();

    if (m_helper.exitStatus() == QProcess::NormalExit &&
        m_helper.exitCode() == EXIT_SUCCESS) {
        accept();
    } else {
        decline();
    }
}

AccessControlManager::AccessControlManager(QObject *parent):
    SignOn::AbstractAccessControlManager(parent)
{
}

AccessControlManager::~AccessControlManager()
{
}

QString AccessControlManager::keychainWidgetAppId()
{
    return QLatin1String(keychainAppId);
}

bool AccessControlManager::isPeerAllowedToAccess(
                                       const QDBusConnection &peerConnection,
                                       const QDBusMessage &peerMessage,
                                       const QString &securityContext)
{
    QString appId = appIdOfPeer(peerConnection, peerMessage);
    if (appId == securityContext ||
        appId == IDENTITY_AC_HELPER) {
        qDebug() << "Allowing";
        return true;
    } else {
        qDebug() << "Blocking access";
        return false;
    }
}

QString AccessControlManager::appIdOfPeer(const QDBusConnection &peerConnection,
                                          const QDBusMessage &peerMessage)
{
    pid_t pid = pidOfPeer(peerConnection, peerMessage);
    QFile file(QString::fromLatin1("/proc/%1/cmdline").arg(pid));
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't read name of pid" << pid;
        return QString::fromLatin1("pid-%1").arg(pid);
    }

    QByteArray command = file.readLine().split('\0').value(0, "unknown");
    return QString::fromUtf8(command);
}

pid_t AccessControlManager::pidOfPeer(const QDBusConnection &peerConnection,
                                      const QDBusMessage &peerMessage)
{
    QString service = peerMessage.service();
    if (service.isEmpty()) {
#ifdef ENABLE_P2P
        DBusConnection *connection =
            (DBusConnection *)peerConnection.internalPointer();
        unsigned long pid = 0;
        dbus_bool_t ok = dbus_connection_get_unix_process_id(connection,
                                                             &pid);
        if (Q_UNLIKELY(!ok)) {
            qWarning() << "Couldn't get PID of caller!";
            return 0;
        }
        return pid;
#else
        qWarning() << "Empty caller name, and no P2P support enabled";
        return 0;
#endif
    } else {
        return peerConnection.interface()->servicePid(service).value();
    }
}

SignOn::AccessReply *
AccessControlManager::handleRequest(const SignOn::AccessRequest &request)
{
    QString appId = appIdOfPeer(request.peerConnection(),
                                request.peerMessage());
    qDebug() << Q_FUNC_INFO << appId;
    return new AccessReply(request, appId, this);
}

#include "access-control-manager.moc"
