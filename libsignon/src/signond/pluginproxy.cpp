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

#include "pluginproxy.h"

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#include <QStringList>
#include <QThreadStorage>
#include <QThread>
#include <QDataStream>

#include "signond-common.h"
#include "SignOn/uisessiondata_priv.h"
#include "SignOn/signonplugincommon.h"

/*
 *   TODO: remove the "SignOn/authpluginif.h" include below after the removal
 *         of the deprecated error handling (needed here only for the deprecated
 *         AuthPluginError::PLUGIN_ERROR_GENERAL).
 */
#include "SignOn/authpluginif.h"

// signon-plugins-common
#include "SignOn/blobiohandler.h"
#include "SignOn/ipc.h"

using namespace SignOn;

#define REMOTEPLUGIN_BIN_PATH QLatin1String("signonpluginprocess")
#define PLUGINPROCESS_START_TIMEOUT 5000
#define PLUGINPROCESS_STOP_TIMEOUT 1000

using namespace SignOn;

namespace SignonDaemonNS {

/* ---------------------- PluginProcess ---------------------- */

PluginProcess::PluginProcess(QObject *parent):
    QProcess(parent)
{
}

PluginProcess::~PluginProcess()
{
}

/* ---------------------- PluginProxy ---------------------- */

PluginProxy::PluginProxy(QString type, QObject *parent):
    QObject(parent)
{
    TRACE();

    m_type = type;
    m_isProcessing = false;
    m_isResultObtained = false;
    m_currentResultOperation = -1;
    m_process = new PluginProcess(this);

#ifdef SIGNOND_TRACE
    if (criticalsEnabled()) {
        const char *level = debugEnabled() ? "2" : "1";
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QLatin1String("SSO_DEBUG"), QLatin1String(level));
        m_process->setProcessEnvironment(env);
    }
#endif

    connect(m_process, SIGNAL(readyReadStandardError()),
            this, SLOT(onReadStandardError()));

    /*
     * TODO: some error handling should be added here, at least remove of
     * current request data from the top of the queue and reply an error code
     */
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onExit(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(onError(QProcess::ProcessError)));
}

PluginProxy::~PluginProxy()
{
    if (m_process != NULL &&
        m_process->state() != QProcess::NotRunning)
    {
        if (m_isProcessing)
            cancel();

        stop();

        /* Closing the write channel ensures that the plugin process
         * will not get stuck on the next read.
         */
        m_process->closeWriteChannel();

        if (!m_process->waitForFinished(PLUGINPROCESS_STOP_TIMEOUT)) {
            qCritical() << "The signon plugin does not react on demand to "
                "stop: need to kill it!!!";
            m_process->kill();

            if (!m_process->waitForFinished(PLUGINPROCESS_STOP_TIMEOUT))
            {
                if (m_process->pid()) {
                    qCritical() << "The signon plugin seems to ignore kill(), "
                        "killing it from command line";
                    QString killProcessCommand(QString::fromLatin1("kill -9 %1").arg(m_process->pid()));
                    QProcess::execute(killProcessCommand);
                }
            }
        }
    }
}

PluginProxy* PluginProxy::createNewPluginProxy(const QString &type)
{
    PluginProxy *pp = new PluginProxy(type);

    QStringList args = QStringList() << pp->m_type;
    pp->m_process->start(REMOTEPLUGIN_BIN_PATH, args);

    QByteArray tmp;

    if (!pp->waitForStarted(PLUGINPROCESS_START_TIMEOUT)) {
        TRACE() << "The process cannot be started";
        delete pp;
        return NULL;
    }

    if (!pp->readOnReady(tmp, PLUGINPROCESS_START_TIMEOUT)) {
        TRACE() << "The process cannot load plugin";
        delete pp;
        return NULL;
    }

    if (debugEnabled()) {
        QString pluginType = pp->queryType();
        if (pluginType != pp->m_type) {
            BLAME() << QString::fromLatin1("Plugin returned type '%1', "
                                           "expected '%2'").
                arg(pluginType).arg(pp->m_type);
        }
    }
    pp->m_mechanisms = pp->queryMechanisms();

    connect(pp->m_process, SIGNAL(readyRead()),
            pp, SLOT(onReadStandardOutput()));

    TRACE() << "The process is started";
    return pp;
}

bool PluginProxy::process(const QVariantMap &inData,
                          const QString &mechanism)
{
    if (!restartIfRequired())
        return false;

    m_isResultObtained = false;
    QVariant value = inData.value(SSOUI_KEY_UIPOLICY);
    m_uiPolicy = value.toInt();

    QDataStream in(m_process);
    in << (quint32)PLUGIN_OP_PROCESS;
    in << mechanism;

    m_blobIOHandler->sendData(inData);

    m_isProcessing = true;
    return true;
}

bool PluginProxy::processUi(const QVariantMap &inData)
{
    TRACE();

    if (!restartIfRequired())
        return false;

    QDataStream in(m_process);

    in << (quint32)PLUGIN_OP_PROCESS_UI;

    m_blobIOHandler->sendData(inData);

    m_isProcessing = true;

    return true;
}

bool PluginProxy::processRefresh(const QVariantMap &inData)
{
    TRACE();

    if (!restartIfRequired())
        return false;

    QDataStream in(m_process);

    in << (quint32)PLUGIN_OP_REFRESH;

    m_blobIOHandler->sendData(inData);

    m_isProcessing = true;

    return true;
}

void PluginProxy::cancel()
{
    TRACE();
    QDataStream in(m_process);
    in << (quint32)PLUGIN_OP_CANCEL;
}

void PluginProxy::stop()
{
    TRACE();
    QDataStream in(m_process);
    in << (quint32)PLUGIN_OP_STOP;
}

bool PluginProxy::readOnReady(QByteArray &buffer, int timeout)
{
    bool ready = m_process->waitForReadyRead(timeout);

    if (ready) {
        if (!m_process->bytesAvailable())
            return false;

        while (m_process->bytesAvailable())
            buffer += m_process->readAllStandardOutput();
    }

    return ready;
}

bool PluginProxy::isProcessing()
{
    return m_isProcessing;
}

void PluginProxy::blobIOError()
{
    TRACE();
    disconnect(m_blobIOHandler, SIGNAL(error()), this, SLOT(blobIOError()));
    stop();

    connect(m_process, SIGNAL(readyRead()), this, SLOT(onReadStandardOutput()));
    emit processError(
        (int)Error::InternalServer,
        QLatin1String("Failed to I/O session data to/from the authentication "
                      "plugin."));
}

bool PluginProxy::isResultOperationCodeValid(const int opCode) const
{
    if (opCode == PLUGIN_RESPONSE_RESULT
        || opCode == PLUGIN_RESPONSE_STORE
        || opCode == PLUGIN_RESPONSE_ERROR
        || opCode == PLUGIN_RESPONSE_SIGNAL
        || opCode == PLUGIN_RESPONSE_UI
        || opCode == PLUGIN_RESPONSE_REFRESHED) return true;

    return false;
}

void PluginProxy::onReadStandardOutput()
{
    disconnect(m_process, SIGNAL(readyRead()),
               this, SLOT(onReadStandardOutput()));

    if (!m_process->bytesAvailable()) {
        qCritical() << "No information available on process";
        m_isProcessing = false;
        emit processError(Error::InternalServer, QString());
        return;
    }

    QDataStream reader(m_process);
    reader >> m_currentResultOperation;

    TRACE() << "PROXY RESULT OPERATION:" << m_currentResultOperation;

    if (!isResultOperationCodeValid(m_currentResultOperation)) {
        TRACE() << "Unknown operation code - skipping.";

        //flushing the stdin channel
        Q_UNUSED(m_process->readAllStandardOutput());

        connect(m_process, SIGNAL(readyRead()),
                this, SLOT(onReadStandardOutput()));
        return;
    }

    if (m_currentResultOperation != PLUGIN_RESPONSE_SIGNAL &&
        m_currentResultOperation != PLUGIN_RESPONSE_ERROR) {

        connect(m_blobIOHandler, SIGNAL(error()),
                this, SLOT(blobIOError()));

        int expectedDataSize = 0;
        reader >> expectedDataSize;
        TRACE() << "PROXY EXPECTED DATA SIZE:" << expectedDataSize;

        m_blobIOHandler->receiveData(expectedDataSize);
    } else {
        handlePluginResponse(m_currentResultOperation);
    }
}

void PluginProxy::sessionDataReceived(const QVariantMap &map)
{
    handlePluginResponse(m_currentResultOperation, map);
}

void PluginProxy::handlePluginResponse(const quint32 resultOperation,
                                       const QVariantMap &sessionDataMap)
{
    TRACE() << resultOperation;

    if (resultOperation == PLUGIN_RESPONSE_RESULT) {
        TRACE() << "PLUGIN_RESPONSE_RESULT";

        m_isProcessing = false;

        if (!m_isResultObtained)
            emit processResultReply(sessionDataMap);
        else
            BLAME() << "Unexpected plugin response: ";

        m_isResultObtained = true;
    } else if (resultOperation == PLUGIN_RESPONSE_STORE) {
        TRACE() << "PLUGIN_RESPONSE_STORE";

        if (!m_isResultObtained)
            emit processStore(sessionDataMap);
        else
            BLAME() << "Unexpected plugin store: ";

    } else if (resultOperation == PLUGIN_RESPONSE_UI) {
        TRACE() << "PLUGIN_RESPONSE_UI";

        if (!m_isResultObtained) {
            bool allowed = true;

            if (m_uiPolicy == NoUserInteractionPolicy)
                allowed = false;

            if (m_uiPolicy == ValidationPolicy) {
                bool credentialsQueried =
                    (sessionDataMap.contains(SSOUI_KEY_QUERYUSERNAME)
                    || sessionDataMap.contains(SSOUI_KEY_QUERYPASSWORD));

                bool captchaQueried  =
                    (sessionDataMap.contains(SSOUI_KEY_CAPTCHAIMG)
                     || sessionDataMap.contains(SSOUI_KEY_CAPTCHAURL));

                if (credentialsQueried && !captchaQueried)
                    allowed = false;
            }

            if (!allowed) {
                //set error and return;
                TRACE() << "ui policy prevented ui launch";

                QVariantMap nonConstMap = sessionDataMap;
                nonConstMap.insert(SSOUI_KEY_ERROR, QUERY_ERROR_FORBIDDEN);
                processUi(nonConstMap);
            } else {
                TRACE() << "open ui";
                emit processUiRequest(sessionDataMap);
            }
        } else {
            BLAME() << "Unexpected plugin ui response: ";
        }
    } else if (resultOperation == PLUGIN_RESPONSE_REFRESHED) {
        TRACE() << "PLUGIN_RESPONSE_REFRESHED";

        if (!m_isResultObtained)
            emit processRefreshRequest(sessionDataMap);
        else
            BLAME() << "Unexpected plugin ui response: ";
    } else if (resultOperation == PLUGIN_RESPONSE_ERROR) {
        TRACE() << "PLUGIN_RESPONSE_ERROR";
        quint32 err;
        QString errorMessage;

        QDataStream stream(m_process);
        stream >> err;
        stream >> errorMessage;
        m_isProcessing = false;

        if (!m_isResultObtained)
            emit processError((int)err, errorMessage);
        else
            BLAME() << "Unexpected plugin error: " << errorMessage;

        m_isResultObtained = true;
    } else if (resultOperation == PLUGIN_RESPONSE_SIGNAL) {
        TRACE() << "PLUGIN_RESPONSE_SIGNAL";
        quint32 state;
        QString message;

        QDataStream stream(m_process);
        stream >> state;
        stream >> message;

        if (!m_isResultObtained)
            emit stateChanged((int)state, message);
        else
            BLAME() << "Unexpected plugin signal: " << state << message;
    }

    connect(m_process, SIGNAL(readyRead()), this, SLOT(onReadStandardOutput()));
    if (m_process->bytesAvailable()) {
        TRACE() << "plugin has more to read after handling a response";
        onReadStandardOutput();
    }
}

void PluginProxy::onReadStandardError()
{
    QString ba = QString::fromLatin1(m_process->readAllStandardError());
}

void PluginProxy::onExit(int exitCode, QProcess::ExitStatus exitStatus)
{
    TRACE() << "Plugin process exit with code " << exitCode <<
        " : " << exitStatus;

    if (m_isProcessing || exitStatus == QProcess::CrashExit) {
        qCritical() << "Challenge produces CRASH!";
        emit processError(Error::InternalServer,
                          QLatin1String("plugin processed crashed"));
    }
    if (exitCode == 2) {
        TRACE() << "plugin process terminated because cannot change user";
    }

    m_isProcessing = false;
}

void PluginProxy::onError(QProcess::ProcessError err)
{
    TRACE() << "Error: " << err;
}

QString PluginProxy::queryType()
{
    TRACE();

    if (!restartIfRequired())
        return QString();

    QDataStream ds(m_process);
    ds << (quint32)PLUGIN_OP_TYPE;

    QByteArray buffer;
    bool result;

    if (!(result = readOnReady(buffer, PLUGINPROCESS_START_TIMEOUT)))
        qCritical("PluginProxy returned NULL result");

    QString type;
    QDataStream out(buffer);
    out >> type;
    return type;
}

QStringList PluginProxy::queryMechanisms()
{
    TRACE();

    if (!restartIfRequired())
        return QStringList();

    QDataStream in(m_process);
    in << (quint32)PLUGIN_OP_MECHANISMS;

    QByteArray buffer;
    QStringList strList;
    bool result;

    if ((result = readOnReady(buffer, PLUGINPROCESS_START_TIMEOUT))) {

        QVariant mechanismsVar;
        QDataStream out(buffer);

        out >> mechanismsVar;
        QVariantList varList = mechanismsVar.toList();

        for (int i = 0; i < varList.count(); i++)
                strList << varList.at(i).toString();

        TRACE() << strList;
    } else
        qCritical("PluginProxy returned NULL result");

    return strList;
}

bool PluginProxy::waitForStarted(int timeout)
{
    if (!m_process->waitForStarted(timeout))
        return false;

    m_blobIOHandler = new BlobIOHandler(m_process, m_process, this);

    connect(m_blobIOHandler,
            SIGNAL(dataReceived(const QVariantMap &)),
            this,
            SLOT(sessionDataReceived(const QVariantMap &)));

    QSocketNotifier *readNotifier =
        new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read, this);

    readNotifier->setEnabled(false);
    m_blobIOHandler->setReadChannelSocketNotifier(readNotifier);

    return true;
}

bool PluginProxy::waitForFinished(int timeout)
{
    return m_process->waitForFinished(timeout);
}

bool PluginProxy::restartIfRequired()
{
    if (m_process->state() == QProcess::NotRunning) {
        TRACE() << "RESTART REQUIRED";
        m_process->start(REMOTEPLUGIN_BIN_PATH, QStringList(m_type));

        QByteArray tmp;
        if (!waitForStarted(PLUGINPROCESS_START_TIMEOUT) ||
            !readOnReady(tmp, PLUGINPROCESS_START_TIMEOUT))
            return false;
    }
    return true;
}

} //namespace SignonDaemonNS
