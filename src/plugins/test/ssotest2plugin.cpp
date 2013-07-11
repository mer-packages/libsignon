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

#include <QMutex>
#include <QMutexLocker>
#include <QImage>
#include <unistd.h>

#include "ssotest2plugin.h"
#include "ssotest2data.h"

#include "SignOn/signonplugincommon.h"
#include "SignOn/uisessiondata.h"
#include "SignOn/uisessiondata_priv.h"

using namespace SignOn;

namespace SsoTest2PluginNS {

static QMutex mutex;
static bool is_canceled = false;
static QEventLoop uiLoop;
static SignOn::UiSessionData uiData;

SsoTest2Plugin::SsoTest2Plugin(QObject *parent):
    AuthPluginInterface(parent)
{
    TRACE();

    m_type = QLatin1String("ssotest2");
    m_mechanisms = QStringList(QLatin1String("mech1"));
    m_mechanisms += QLatin1String("mech2");
    m_mechanisms += QLatin1String("mech3");

    qRegisterMetaType<SignOn::SessionData>("SignOn::SessionData");
}

SsoTest2Plugin::~SsoTest2Plugin()
{

}

void SsoTest2Plugin::cancel()
{
    TRACE();
    QMutexLocker locker(&mutex);
    is_canceled = true;
    if (uiLoop.isRunning()) uiLoop.quit();
}

/*
 * dummy plugin is used for testing purposes only
 * */
void SsoTest2Plugin::process(const SignOn::SessionData &inData,
                             const QString &mechanism)
{
    if (! mechanisms().contains(mechanism) ) {
        emit error(Error::MechanismNotAvailable);
        return;
    }

    QMetaObject::invokeMethod(this,
                              "execProcess",
                              Qt::QueuedConnection,
                              Q_ARG(SignOn::SessionData, inData),
                              Q_ARG(QString, mechanism));
}

static QByteArray loadImage(const QString &name)
{
    //TODO: adopt to something changeable
    QString resource = QLatin1String(":/");
    QByteArray ba;

    QImage realImage(resource + name);
    QBuffer buffer(&ba);
    buffer.open(QIODevice::ReadWrite);
    ba.clear();
    realImage.save(&buffer);
    return ba;
}

static QVariantMap nameToParameters(const QString &dialogName)
{
    QVariantMap data;

    if (dialogName  == QLatin1String("Browser")) {
        static int ii = 0;
        ii++;

        if (ii % 2)
            data["OpenUrl"] = "www.yahoo.com";
        else
            data["OpenUrl"] = "www.google.com";

        data["watchdog"] = QString(SSOUI_KEY_SLOT_ACCEPT);

    } else if (dialogName  == QLatin1String("Login")) {
        data["UserName"] = "testUsername";
        data["Secret"] = "testSecret";
        data["QueryMessageId"] = 0;
        data["QueryUserName"] = true;
        data["QueryPassword"] = true;
        data["watchdog"] = QString(SSOUI_KEY_SLOT_ACCEPT);
    } else if (dialogName  == QLatin1String("Captcha")) {
        data["QueryMessageId"] = 0;
        data["CaptchaImage"] = loadImage("Captcha1.jpg");
        data["watchdog"] = QString(SSOUI_KEY_SLOT_REJECT);
    } else if (dialogName  == QLatin1String("LoginAndCaptcha")) {
        data["UserName"] = "testUsername";
        data["Secret"] = "testSecret";
        data["QueryMessageId"] = 0;
        data["QueryUserName"] = true;
        data["QueryPassword"] = true;
        data["QueryMessageId"] = 0;

        data["CaptchaImage"] = loadImage("Captcha1.jpg");
        data["watchdog"] = QString(SSOUI_KEY_SLOT_REJECT);
    }

    return data;
}

void SsoTest2Plugin::execProcess(const SignOn::SessionData &inData,
                                 const QString &mechanism)
{
    Q_UNUSED(mechanism);
    int err;
    SsoTest2Data testData = inData.data<SsoTest2Data>();
    QStringList chainOfResults;

    for (int i = 0; i < testData.ChainOfStates().length(); i++)
        if (!is_canceled) {
            quint32 currState = testData.CurrentState();
            QString message =
                QString("message from plugin, state : %1").arg(currState);
            TRACE() << message;
            emit statusChanged(PLUGIN_STATE_WAITING, message);
            usleep(0.1 * 1000000);

            QString dialogName = testData.ChainOfStates().at(currState);

            QVariantMap parameters = nameToParameters(dialogName);
            SignOn::UiSessionData data(parameters);

            emit userActionRequired(data);
            uiLoop.exec();

            int errorCode = data.QueryErrorCode();

            if ( dialogName == QLatin1String("Browser") ) {
                if ( errorCode == SignOn::QUERY_ERROR_NONE ||
                    errorCode == SignOn::QUERY_ERROR_BAD_URL )
                    chainOfResults.append(QLatin1String("OK"));
                else
                    chainOfResults.append(QString("FAIL"));
            } else if ( dialogName == QLatin1String("Login") ) {
                if (errorCode == SignOn::QUERY_ERROR_NONE)
                    chainOfResults.append(QLatin1String("OK"));
                else
                    chainOfResults.append(QString("FAIL"));
            } else if ( dialogName == QLatin1String("Captcha") ||
                      dialogName == QLatin1String("LoginAndCaptcha") ) {
                if (errorCode == SignOn::QUERY_ERROR_CANCELED)
                    chainOfResults.append(QLatin1String("OK"));
                else
                    chainOfResults.append(QLatin1String("FAIL"));
            }

            testData.setCurrentState(currState+1);
        }

    if (is_canceled) {
        TRACE() << "Operation is canceled";
        QMutexLocker locker(&mutex);
        is_canceled = false;
        emit error(Error::SessionCanceled);
        return;
    }

    if (!testData.ChainOfStates().length() ||
        testData.CurrentState() >= (quint32)testData.ChainOfStates().length()) {
        err = 0;
    }

    testData.setSecret("testSecret_after_test");

    foreach(QString key, testData.propertyNames())
        TRACE() << key << ": " << testData.getProperty(key);

    TRACE() << "Emit the signal";
    if (err)
        emit error(err);
    else
        emit result(testData);
}

void SsoTest2Plugin::userActionFinished(const SignOn::UiSessionData &data)
{
    TRACE();
    uiData = data;
    uiLoop.quit();
    TRACE() << "Completed";
}

void SsoTest2Plugin::refresh(const SignOn::UiSessionData &data)
{
    TRACE();
    static int ii = 2;

    uiData = data;
    QString imageName = QString("Captcha%1.jpg").arg(ii);
    TRACE() << imageName;
    uiData.setCaptchaImage(loadImage(imageName));
    ii++;
    if (ii>4)
        ii = 1;
    emit refreshed(uiData);
    TRACE() << "Completed";
}

SIGNON_DECL_AUTH_PLUGIN(SsoTest2Plugin)
} //namespace SsoTest2PluginNS


