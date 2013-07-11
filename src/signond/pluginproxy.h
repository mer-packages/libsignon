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

#ifndef PLUGINPROXY_H
#define PLUGINPROXY_H

#include <QDBusConnection>
#include <QDBusMessage>
#include <QtCore>

namespace SignOn {
    class BlobIOHandler;
    class EncryptedDevice;
};

namespace SignonDaemonNS {

/*!
 * @class PluginProcess
 * Process to run authentication.
 * @todo description.
 */
class PluginProcess: public QProcess
{
    Q_OBJECT
    friend class PluginProxy;

    PluginProcess(QObject* parent = NULL);
    ~PluginProcess();
};

/*!
 * @class PluginProxy
 * Plugin proxy.
 * @todo description.
 */
class PluginProxy: public QObject
{
    Q_OBJECT

    friend class SignonIdentity;
    friend class TestAuthSession;

public:
    static PluginProxy *createNewPluginProxy(const QString &type);
    virtual ~PluginProxy();

    bool restartIfRequired();
    bool isProcessing();

public Q_SLOTS:
    QString type() const { return m_type; }
    QStringList mechanisms() const { return m_mechanisms; }
    bool process(const QVariantMap &inData,
                 const QString &mechanism);
    bool processUi(const QVariantMap &inData);
    bool processRefresh(const QVariantMap &inData);
    void cancel();
    void stop();

Q_SIGNALS:
    void processResultReply(const QVariantMap &data);
    void processStore(const QVariantMap &data);
    void processUiRequest(const QVariantMap &data);
    void processRefreshRequest(const QVariantMap &data);
    void processError(int error,
                      const QString &message);
    void stateChanged(int state,
                      const QString &message);

private:
    QString queryType();
    QStringList queryMechanisms();

    bool waitForStarted(int timeout);
    bool waitForFinished(int timeout);

    bool readOnReady(QByteArray &buffer, int timeout);

    void handlePluginResponse(const quint32 resultOperation,
                              const QVariantMap &sessionDataMap = QVariantMap());

    bool isResultOperationCodeValid(const int opCode) const;

private Q_SLOTS:
    void onReadStandardOutput();
    void onReadStandardError();
    void onExit(int exitCode, QProcess::ExitStatus exitStatus);
    void onError(QProcess::ProcessError err);
    void sessionDataReceived(const QVariantMap &map);
    void blobIOError();

private:
    PluginProxy(QString type, QObject *parent = NULL);

    bool m_isProcessing;
    bool m_isResultObtained;
    QString m_type;
    QStringList m_mechanisms;
    int m_uiPolicy;
    int m_currentResultOperation;

    PluginProcess *m_process;
    SignOn::BlobIOHandler *m_blobIOHandler;
};

} //namespace SignonDaemonNS

#endif /* PLUGINPROXY_H */
