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

#include "signon-ui.h"
#include "testauthsession.h"
#include "testthread.h"
#include "SignOn/identity.h"
#include <sys/wait.h>
#include <unistd.h>

#define SSO_TEST_CREATE_AUTH_SESSION(__session__, __method__) \
    do {                                                            \
        Identity *id = Identity::newIdentity(IdentityInfo(), this); \
        __session__ = id->createSession(QLatin1String(__method__)); \
    } while(0)


static AuthSession *g_currentSession = NULL;
static QStringList g_processReplyRealmsList;
static int g_bigStringSize = 50000;
static int g_bigStringReplySize = 0;

TestAuthSession::TestAuthSession(SignOnUI *signOnUI, QObject *parent):
    QObject(parent),
    m_signOnUI(signOnUI)
{
}

void TestAuthSession::initTestCase()
{
    qDebug() << "HI!";
}

void TestAuthSession::cleanupTestCase()
{
    qDebug() << "BYE!";
}

void TestAuthSession::sessionData()
{
    QVariantMap originalMap;
    originalMap["Hello"] = "World";
    originalMap["Int"] = 4;

    SessionData sessionData(originalMap);
    QCOMPARE(sessionData.toMap(), originalMap);
}

void TestAuthSession::queryMechanisms_existing_method()
{
    AuthSession *as;
    SSO_TEST_CREATE_AUTH_SESSION(as, "ssotest");

    QStringList wantedMechs;

    QSignalSpy spy(as, SIGNAL(mechanismsAvailable(const QStringList&)));
    QSignalSpy errorCounter(as, SIGNAL(error(const SignOn::Error &)));
    QEventLoop loop;

    QObject::connect(as, SIGNAL(mechanismsAvailable(const QStringList&)),
                     &loop, SLOT(quit()));
    QObject::connect(as, SIGNAL(error(const SignOn::Error &)),
                     &loop, SLOT(quit()));
    QTimer::singleShot(10*1000, &loop, SLOT(quit()));

    as->queryAvailableMechanisms(wantedMechs);

    if (!errorCounter.count())
        loop.exec();

    errorCounter.clear();

    QCOMPARE(spy.count(), 1);
    QStringList result = spy.at(0).at(0).toStringList();
    QCOMPARE(result.size(), 4);

    wantedMechs += "mech1";

    QTimer::singleShot(10*1000, &loop, SLOT(quit()));

    as->queryAvailableMechanisms(wantedMechs);

    if (!errorCounter.count())
        loop.exec();

    errorCounter.clear();

    QCOMPARE(spy.count(), 2);
    result = spy.at(1).at(0).toStringList();
    QCOMPARE(result.size(), 1);

    wantedMechs += "mech2";

    QTimer::singleShot(10*1000, &loop, SLOT(quit()));

    as->queryAvailableMechanisms(wantedMechs);

    if (!errorCounter.count())
        loop.exec();

    errorCounter.clear();

    QCOMPARE(spy.count(), 3);
    result = spy.at(2).at(0).toStringList();
    QCOMPARE(result.size(), 2);

    wantedMechs = QStringList("fake");

    QTimer::singleShot(10*1000, &loop, SLOT(quit()));

    as->queryAvailableMechanisms(wantedMechs);

    if (!errorCounter.count())
        loop.exec();

    errorCounter.clear();

    QCOMPARE(spy.count(), 4);
    result = spy.at(3).at(0).toStringList();
    QCOMPARE(result.size(), 0);
 }

 void TestAuthSession::queryMechanisms_nonexisting_method()
 {
     AuthSession *as;
     SSO_TEST_CREATE_AUTH_SESSION(as, "nonexisting");

     QStringList wantedMechs;

     QSignalSpy spy(as, SIGNAL(error(const SignOn::Error &)));
     QEventLoop loop;

     QObject::connect(as, SIGNAL(mechanismsAvailable(const QStringList&)), &loop, SLOT(quit()));
     QObject::connect(as, SIGNAL(error(const SignOn::Error &)), &loop, SLOT(quit()));
     QTimer::singleShot(10*1000, &loop, SLOT(quit()));

     as->queryAvailableMechanisms(wantedMechs);
     loop.exec();

     QCOMPARE(spy.count(), 1);
 }

 void TestAuthSession::process_with_new_identity()
 {
     AuthSession *as;
     SSO_TEST_CREATE_AUTH_SESSION(as, "ssotest");

     g_processReplyRealmsList.clear();
     connect(as, SIGNAL(response(const SignOn::SessionData &)), this, SLOT(response(const SignOn::SessionData &)));

     QSignalSpy spy(as, SIGNAL(response(const SignOn::SessionData &)));
     QSignalSpy errorCounter(as, SIGNAL(error(const SignOn::Error &)));
     QSignalSpy stateCounter(as, SIGNAL(stateChanged(AuthSession::AuthSessionState, const QString&)));
     QEventLoop loop;

     QObject::connect(as, SIGNAL(response(const SignOn::SessionData &)), &loop, SLOT(quit()));
     QObject::connect(as, SIGNAL(error(const SignOn::Error &)), &loop, SLOT(quit()));
     QTimer::singleShot(10*1000, &loop, SLOT(quit()));

     SessionData inData;

     inData.setSecret("testSecret");
     inData.setUserName("testUsername");

     as->process(inData, "mech1");
     if (!errorCounter.count())
         loop.exec();

     QCOMPARE(stateCounter.count(), 12);
     stateCounter.clear();
     errorCounter.clear();

     as->process(inData, "mech1");
     if (!errorCounter.count())
         loop.exec();

     QCOMPARE(stateCounter.count(), 12);
     stateCounter.clear();
     errorCounter.clear();

     as->process(inData, "mech1");
     if (!errorCounter.count())
         loop.exec();

     QCOMPARE(stateCounter.count(), 12);
     stateCounter.clear();
     errorCounter.clear();

     as->process(inData, "mech1");
     if (!errorCounter.count())
         loop.exec();

     QCOMPARE(stateCounter.count(), 12);
     stateCounter.clear();
     errorCounter.clear();

     QCOMPARE(spy.count(), 4);

     QVERIFY(g_processReplyRealmsList.at(0) == "testRealm_after_test");
     QVERIFY(g_processReplyRealmsList.at(1) == "testRealm_after_test");
     QVERIFY(g_processReplyRealmsList.at(2) == "testRealm_after_test");
     QVERIFY(g_processReplyRealmsList.at(3) == "testRealm_after_test");
 }

 void TestAuthSession::process_with_existing_identity()
 {
     AuthSession *as;
     SSO_TEST_CREATE_AUTH_SESSION(as, "ssotest");

     g_processReplyRealmsList.clear();
     connect(as, SIGNAL(response(const SignOn::SessionData &)), this, SLOT(response(const SignOn::SessionData &)));

     QSignalSpy errorCounter(as, SIGNAL(error(const SignOn::Error &)));
     QSignalSpy stateCounter(as, SIGNAL(stateChanged(AuthSession::AuthSessionState, const QString&)));
     QSignalSpy spy(as, SIGNAL(response(const SignOn::SessionData&)));
     QEventLoop loop;

     QObject::connect(as, SIGNAL(response(const SignOn::SessionData&)), &loop, SLOT(quit()));
     QObject::connect(as, SIGNAL(error(const SignOn::Error &)), &loop, SLOT(quit()));
     QTimer::singleShot(10*1000, &loop, SLOT(quit()));

     SessionData inData;

     inData.setSecret("testSecret");
     inData.setUserName("testUsername");

     as->process(inData, "mech1");
     if (!errorCounter.count())
         loop.exec();
     QCOMPARE(stateCounter.count(), 12);
     stateCounter.clear();
     errorCounter.clear();

     as->process(inData, "mech1");
     if (!errorCounter.count())
         loop.exec();
     QCOMPARE(stateCounter.count(), 12);
     stateCounter.clear();
     errorCounter.clear();

     as->process(inData, "mech1");
     if (!errorCounter.count())
         loop.exec();
     QCOMPARE(stateCounter.count(), 12);
     stateCounter.clear();
     errorCounter.clear();

     as->process(inData, "mech1");
     if (!errorCounter.count())
         loop.exec();
     QCOMPARE(stateCounter.count(), 12);
     stateCounter.clear();
     errorCounter.clear();

     QCOMPARE(spy.count(), 4);

     QVERIFY(g_processReplyRealmsList.at(0) == "testRealm_after_test");
     QVERIFY(g_processReplyRealmsList.at(1) == "testRealm_after_test");
     QVERIFY(g_processReplyRealmsList.at(2) == "testRealm_after_test");
     QVERIFY(g_processReplyRealmsList.at(3) == "testRealm_after_test");

 }

 void TestAuthSession::process_with_nonexisting_type()
 {
     AuthSession *as;
     SSO_TEST_CREATE_AUTH_SESSION(as, "nonexisting");

     QSignalSpy spyResponse(as, SIGNAL(response(const SignOn::SessionData &)));
     QSignalSpy spyError(as, SIGNAL(error(const SignOn::Error &)));
     QSignalSpy stateCounter(as, SIGNAL(stateChanged(AuthSession::AuthSessionState, const QString&)));
     QEventLoop loop;

     QObject::connect(as, SIGNAL(error(const SignOn::Error &)), &loop, SLOT(quit()),  Qt::QueuedConnection);
     QTimer::singleShot(10*1000, &loop, SLOT(quit()));

     SessionData inData;

     inData.setSecret("testSecret");
     inData.setUserName("testUsername");

     as->process(inData, "mech1");
     loop.exec();

     as->process(inData, "mech1");
     loop.exec();

     as->process(inData, "mech1");
     loop.exec();

     as->process(inData, "mech1");
     loop.exec();

     QCOMPARE(spyResponse.count(), 0);
     QCOMPARE(spyError.count(), 4);
     QCOMPARE(stateCounter.count(), 0);
 }


 void TestAuthSession::process_with_nonexisting_method()
 {
     AuthSession *as;
     SSO_TEST_CREATE_AUTH_SESSION(as, "ssotest");

     QSignalSpy spyResponse(as, SIGNAL(response(const SignOn::SessionData&)));
     QSignalSpy spyError(as, SIGNAL(error(const SignOn::Error &)));
     QSignalSpy stateCounter(as, SIGNAL(stateChanged(AuthSession::AuthSessionState, const QString&)));
     QEventLoop loop;

     QObject::connect(as, SIGNAL(error(const SignOn::Error &)), &loop, SLOT(quit()),  Qt::QueuedConnection);
     QTimer::singleShot(10*1000, &loop, SLOT(quit()));

     SessionData inData;

     inData.setSecret("testSecret");
     inData.setUserName("testUsername");

     as->process(inData, "nonexisting");
     loop.exec();

     as->process(inData, "nonexisting");
     loop.exec();

     as->process(inData, "nonexisting");
     loop.exec();

     as->process(inData, "nonexisting");
     loop.exec();

     QCOMPARE(spyResponse.count(), 0);
     QCOMPARE(spyError.count(), 4);
     QCOMPARE(stateCounter.count(), 8);
 }

void TestAuthSession::process_with_unauthorized_method()
{
    MechanismsList mechs;
    mechs.append("mech1");
    QMap<MethodName,MechanismsList> methods;
    methods.insert(QLatin1String("ssotest"), mechs);
    IdentityInfo info("test_caption", "test_user_name", methods);
    info.setSecret("test_secret");
    Identity *id = Identity::newIdentity(info, this);

    QSignalSpy spyResponseStoreCreds(id, SIGNAL(credentialsStored(const quint32)));
    QSignalSpy spyErrorStoreCreds(id, SIGNAL(error(const SignOn::Error &)));

    QEventLoop loopStoreCreds;

    QObject::connect(id, SIGNAL(error(const SignOn::Error &)), &loopStoreCreds, SLOT(quit()),  Qt::QueuedConnection);
    QObject::connect(id, SIGNAL(credentialsStored(const quint32)), &loopStoreCreds, SLOT(quit()));
    QTimer::singleShot(10*1000, &loopStoreCreds, SLOT(quit()));

    id->storeCredentials();
    loopStoreCreds.exec();

    QCOMPARE(spyResponseStoreCreds.count(), 1);
    QCOMPARE(spyErrorStoreCreds.count(), 0);

    AuthSession *as = id->createSession(QLatin1String("ssotest"));

    QSignalSpy spyResponse(as, SIGNAL(response(const SignOn::SessionData &)));
    QSignalSpy spyError(as, SIGNAL(error(const SignOn::Error &)));
    QEventLoop loop;

    QObject::connect(as, SIGNAL(error(const SignOn::Error &)), &loop, SLOT(quit()),  Qt::QueuedConnection);
    QObject::connect(as, SIGNAL(response(const SignOn::SessionData &)), &loop, SLOT(quit()));
    QTimer::singleShot(10*1000, &loop, SLOT(quit()));

    SessionData inData;
    as->process(inData, "mech2");
    loop.exec();

    QCOMPARE(spyResponse.count(), 0);
    QCOMPARE(spyError.count(), 1);
    // Still make sure the error really was about bad auth method/mechanism
    SignOn::Error::ErrorType errorType = SignOn::Error::Unknown;
    QVariant var = spyError.at(0).at(0);
    if (QLatin1String("SignOn::Error") == var.typeName()) {
       SignOn::Error error = var.value<SignOn::Error>();
       errorType = (SignOn::Error::ErrorType)error.type();
    }
    QCOMPARE(errorType, SignOn::Error::MethodOrMechanismNotAllowed);
}

void TestAuthSession::process_from_other_process()
{
    // In order to try reusing same authentication session from
    // another process we need the session object path, which isn't
    // available through the client API. To work around this we
    // don't use the API but make direct D-Bus calls instead

    // The session bus used by the API cannot be accessed outside
    // the API library so create our own bus
    QDBusConnection dbuscon1 =
        QDBusConnection::connectToBus(QDBusConnection::SessionBus,
                                      "originalconnection");
    QDBusInterface iface(SIGNOND_SERVICE,
                         SIGNOND_DAEMON_OBJECTPATH,
                         SIGNOND_DAEMON_INTERFACE,
                         dbuscon1);

    SlotMachine slotMachine;
    QEventLoop sessionLoop;
    QObject::connect(&slotMachine, SIGNAL(done()), &sessionLoop, SLOT(quit()));
    QTimer::singleShot(10*1000, &sessionLoop, SLOT(quit()));

    QVariantList arguments;
    arguments += (quint32)SIGNOND_NEW_IDENTITY;
    arguments += QString::fromLatin1("ssotest");
    iface.callWithCallback(QLatin1String("getAuthSessionObjectPath"),
                           arguments, &slotMachine,
                           SLOT(authenticationSlot(const QString&)),
                           SLOT(errorSlot(const QDBusError&)));

    sessionLoop.exec();

    QString qs;
    if (slotMachine.m_path.isEmpty())
        qDebug() << "getAuthSessionObjectPath failed: " << slotMachine.m_errorMessage.toLatin1().data();
    QVERIFY(slotMachine.m_path.length() > 0);

    int exitCode = 1;
    pid_t childPid = fork();
    QVERIFY(childPid != -1);

    if (childPid != 0) {
        int status = 0;
        childPid = waitpid(childPid, &status, 0);
        QVERIFY(childPid != -1 && WIFEXITED(status));
        exitCode = WEXITSTATUS(status);
    } else {
        // We're in the child process now...
        // Do not reuse existing session bus because it is seen by signond
        // as if coming from the parent process and we want to test connection
		 // from other process
        QDBusConnection dbuscon2 = QDBusConnection::connectToBus(QDBusConnection::SessionBus, "otherconnection");
        QDBusInterface *dbus = new QDBusInterface(SIGNOND_SERVICE,
                                                  slotMachine.m_path,
                                                  QLatin1String(SIGNOND_AUTH_SESSION_INTERFACE),
                                                  dbuscon2);

        SessionData inData;
        inData.setSecret("testSecret");
        inData.setUserName("testUsername");
        QVariantMap inDataVarMap;
        foreach(QString key, inData.propertyNames()) {
            if (!inData.getProperty(key).isNull() && inData.getProperty(key).isValid())
                inDataVarMap[key] = inData.getProperty(key);
        }

        arguments.clear();
        arguments += inDataVarMap;
        arguments += QString::fromLatin1("mech1");

        QDBusMessage msg = QDBusMessage::createMethodCall(dbus->service(),
                                                          dbus->path(),
                                                          dbus->interface(),
                                                          QString::fromLatin1("process"));
        msg.setArguments(arguments);

        QEventLoop processLoop;
        QObject::connect(&slotMachine, SIGNAL(done()), &processLoop, SLOT(quit()));
        QTimer::singleShot(10*1000, &processLoop, SLOT(quit()));

        dbus->connection().callWithCallback(msg, &slotMachine,
                                            SLOT(responseSlot(const QVariantMap&)),
                                            SLOT(errorSlot(const QDBusError&)),
                                            SIGNOND_MAX_TIMEOUT);

        processLoop.exec();

        delete dbus;

        if (slotMachine.m_responseReceived) {
            qDebug() << "AuthSession::process succeeded even though it was expected to fail";
            exit(1);
        } else {
            if (slotMachine.m_errorName == SIGNOND_PERMISSION_DENIED_ERR_NAME)
                exit(0);

            qDebug() << "AuthSession::process failed but with unexpected error: " <<
                        slotMachine.m_errorMessage;
            exit(1);
        }
    }

    QCOMPARE(exitCode, 0);
}

void TestAuthSession::process_many_times_after_auth()
{
    AuthSession *as;
    SSO_TEST_CREATE_AUTH_SESSION(as, "ssotest");

    QSignalSpy spyResponse(as, SIGNAL(response(const SignOn::SessionData&)));
    QSignalSpy spyError(as, SIGNAL(error(const SignOn::Error &)));
    QEventLoop loop;

    QObject::connect(as, SIGNAL(error(const SignOn::Error &)), &loop, SLOT(quit()));
    QObject::connect(as, SIGNAL(response(const SignOn::SessionData&)), &loop, SLOT(quit()));
    QTimer::singleShot(10*1000, &loop, SLOT(quit()));

    SessionData inData;

    inData.setSecret("testSecret");
    inData.setUserName("testUsername");

    as->process(inData, "mech1");
    loop.exec();
    QCOMPARE(spyResponse.count(), 1);

    as->process(inData, "mech1");
    as->process(inData, "mech1");
    as->process(inData, "mech1");
    as->process(inData, "mech1");

    loop.exec();

    QCOMPARE(spyResponse.count(), 2);
    QCOMPARE(spyError.count(), 3);

    as->process(inData, "mech1");
    as->process(inData, "mech1");
    as->process(inData, "mech1");
    as->process(inData, "mech1");

    loop.exec();

    QCOMPARE(spyResponse.count(), 3);
    QCOMPARE(spyError.count(), 6);

    as->process(inData, "mech1");
    as->process(inData, "mech1");
    as->process(inData, "mech1");
    as->process(inData, "mech1");

    loop.exec();

    QCOMPARE(spyResponse.count(), 4);
    QCOMPARE(spyError.count(), 9);
}

void TestAuthSession::process_many_times_before_auth()
{
    AuthSession *as;
    SSO_TEST_CREATE_AUTH_SESSION(as, "ssotest");

    QSignalSpy spyResponse(as, SIGNAL(response(const SignOn::SessionData&)));
    QSignalSpy spyError(as, SIGNAL(error(const SignOn::Error &)));
    QEventLoop loop;

    QObject::connect(as, SIGNAL(response(const SignOn::SessionData&)),
                     &loop, SLOT(quit()));
    QTimer::singleShot(10*1000, &loop, SLOT(quit()));

    SessionData inData;

    inData.setSecret("testSecret");
    inData.setUserName("testUsername");

    as->process(inData, "mech1");
    as->process(inData, "mech1");
    as->process(inData, "mech1");
    as->process(inData, "mech1");

    loop.exec();

    QCOMPARE(spyError.count(), 3);
    QCOMPARE(spyResponse.count(), 1);
}

void TestAuthSession::process_with_big_session_data()
{
    //TODO once bug Bug#222200 is fixed, this test case can be enabled
    QSKIP("This test requires fix", SkipSingle);
    AuthSession *as;
    SSO_TEST_CREATE_AUTH_SESSION(as, "ssotest");

    QSignalSpy spyResponse(as, SIGNAL(response(const SignOn::SessionData&)));
    QSignalSpy spyError(as, SIGNAL(error(const SignOn::Error &)));
    QEventLoop loop;

    QObject::connect(as, SIGNAL(response(const SignOn::SessionData&)),
                     this, SLOT(response(const SignOn::SessionData&)));
    QObject::connect(as, SIGNAL(response(const SignOn::SessionData&)),
                     &loop, SLOT(quit()));
    QTimer::singleShot(10*1000, &loop, SLOT(quit()));

    SessionData inData;

    inData.setSecret("testSecret");

    QString bigString;
    bigString.fill(QChar('A'), g_bigStringSize);
    inData.setCaption(bigString);

    as->process(inData, "BLOB");

    loop.exec();

    QCOMPARE(spyError.count(), 0);
    QCOMPARE(spyResponse.count(), 1);
    QCOMPARE(g_bigStringReplySize, g_bigStringSize);
}

void TestAuthSession::cancel_immidiately()
{
    AuthSession *as;
    SSO_TEST_CREATE_AUTH_SESSION(as, "ssotest");

    QSignalSpy spyResponse(as, SIGNAL(response(const SignOn::SessionData&)));
    QSignalSpy spyError(as, SIGNAL(error(const SignOn::Error &)));
    QEventLoop loop;

    QObject::connect(as, SIGNAL(error(const SignOn::Error &)),
                     &loop, SLOT(quit()));
    QObject::connect(as, SIGNAL(error(const SignOn::Error &)),
                     &loop, SLOT(quit()),  Qt::QueuedConnection);
    QTimer::singleShot(10*1000, &loop, SLOT(quit()));

    SessionData inData;

    inData.setSecret("testSecret");
    inData.setUserName("testUsername");

    as->process(inData, "mech1");
    as->cancel();
    loop.exec();

    QCOMPARE(spyResponse.count(), 0);
    QCOMPARE(spyError.count(), 1);

    as->process(inData, "mech1");
    as->cancel();
    loop.exec();

    QCOMPARE(spyResponse.count(), 0);
    QCOMPARE(spyError.count(), 2);

    as->process(inData, "mech1");
    as->cancel();
    loop.exec();

    QCOMPARE(spyResponse.count(), 0);
    QCOMPARE(spyError.count(), 3);

    as->process(inData, "mech1");
    as->cancel();
    loop.exec();

    QCOMPARE(spyResponse.count(), 0);
    QCOMPARE(spyError.count(), 4);
}

void TestAuthSession::cancel_with_delay()
{
    AuthSession *as;
    SSO_TEST_CREATE_AUTH_SESSION(as, "ssotest");
    g_currentSession = as;

    QSignalSpy spyResponse(as, SIGNAL(response(const SignOn::SessionData&)));
    QSignalSpy spyError(as, SIGNAL(error(const SignOn::Error &)));
    QEventLoop loop;

    QObject::connect(as, SIGNAL(error(const SignOn::Error &)),
                     &loop, SLOT(quit()));
    QTimer::singleShot(10*1000, &loop, SLOT(quit()));

    SessionData inData;

    inData.setSecret("testSecret");
    inData.setUserName("testUsername");

    as->process(inData, "mech1");
    QTimer::singleShot(0.1*1000, this, SLOT(cancel()));
    loop.exec();

    QCOMPARE(spyResponse.count(), 0);
    QCOMPARE(spyError.count(), 1);

    as->process(inData, "mech1");
    QTimer::singleShot(0.1*1000, this, SLOT(cancel()));
    loop.exec();

    QCOMPARE(spyResponse.count(), 0);
    QCOMPARE(spyError.count(), 2);

    as->process(inData, "mech1");
    QTimer::singleShot(0.1*1000, this, SLOT(cancel()));
    loop.exec();

    QCOMPARE(spyResponse.count(), 0);
    QCOMPARE(spyError.count(), 3);

    as->process(inData, "mech1");
    QTimer::singleShot(0.1*1000, this, SLOT(cancel()));
    loop.exec();

    QCOMPARE(spyResponse.count(), 0);
    QCOMPARE(spyError.count(), 4);
}

void TestAuthSession::cancel_without_process()
{
    AuthSession *as;
    SSO_TEST_CREATE_AUTH_SESSION(as, "ssotest");
    g_currentSession = as;

    QSignalSpy spyError(as, SIGNAL(error(const SignOn::Error &)));
    QEventLoop loop;

    QObject::connect(as, SIGNAL(error(const SignOn::Error &)),
                     &loop, SLOT(quit()),  Qt::QueuedConnection);
    QTimer::singleShot(10*1000, &loop, SLOT(quit()));

    QTimer::singleShot(1*1000, &loop, SLOT(quit()));
    as->cancel();
    loop.exec();

    QCOMPARE(spyError.count(), 0);

    QTimer::singleShot(1*1000, &loop, SLOT(quit()));
    QTimer::singleShot(0.1*1000, this, SLOT(cancel()));
    loop.exec();

    QCOMPARE(spyError.count(), 0);

    QTimer::singleShot(0.1*1000, this, SLOT(cancel()));
    QTimer::singleShot(1*1000, &loop, SLOT(quit()));
    loop.exec();

    QCOMPARE(spyError.count(), 0);

    SessionData inData;

    inData.setSecret("testSecret");
    inData.setUserName("testUsername");

    as->process(inData, "mech1");
    as->cancel();
    as->cancel();
    as->cancel();
    loop.exec();

    QCOMPARE(spyError.count(), 1);
}

void TestAuthSession::handle_destroyed_signal()
{
    QSKIP("testing in sb", SkipSingle);
    AuthSession *as;
    SSO_TEST_CREATE_AUTH_SESSION(as, "ssotest");
    g_currentSession = as;

    QSignalSpy spy(as, SIGNAL(mechanismsAvailable(const QStringList&)));
    QSignalSpy errorCounter(as, SIGNAL(error(const SignOn::Error &)));
    QEventLoop loop;

    QObject::connect(as, SIGNAL(mechanismsAvailable(const QStringList&)),
                     &loop, SLOT(quit()));
    QObject::connect(as, SIGNAL(error(const SignOn::Error &)),
                     &loop, SLOT(quit()));

    /*
     * 5 minutes + 10 seconds
     * */
    QTimer::singleShot(5 * 62 *1000, &loop, SLOT(quit()));
    loop.exec();

    QTimer::singleShot(5 * 1000, &loop, SLOT(quit()));
    loop.exec();

    QStringList wantedMechs;
    as->queryAvailableMechanisms(wantedMechs);

    if (!errorCounter.count())
        loop.exec();

    QCOMPARE(spy.count(), 1);
    QStringList result = spy.at(0).at(0).toStringList();
    QCOMPARE(result.size(), 4);
}

void TestAuthSession::multi_thread_test()
{
    //execute a SignOn call in a separate thread
    TestThread thread;
    thread.start();
    thread.wait(g_testThreadTimeout + 1000);

    //do the same in this thread - this test succeeds if the
    //following succeeds
    process_with_new_identity();
}

void TestAuthSession::cancel()
{
    g_currentSession->cancel();
}

void TestAuthSession::response(const SignOn::SessionData &data)
{
    g_processReplyRealmsList << data.Realm();
    g_bigStringReplySize = data.Caption().size();
}

void TestAuthSession::processUi_with_existing_identity()
{
    AuthSession *as;
    SSO_TEST_CREATE_AUTH_SESSION(as, "ssotest2");

    QSignalSpy errorCounter(as, SIGNAL(error(const SignOn::Error &)));
    QSignalSpy stateCounter(as,
          SIGNAL(stateChanged(AuthSession::AuthSessionState, const QString&)));
    QSignalSpy spy(as, SIGNAL(response(const SignOn::SessionData&)));
    QEventLoop loop;

    QObject::connect(as, SIGNAL(response(const SignOn::SessionData&)),
                     &loop, SLOT(quit()));
    QObject::connect(as, SIGNAL(error(const SignOn::Error &)),
                     &loop, SLOT(quit()));
    QTimer::singleShot(500*1000, &loop, SLOT(quit()));

    /*
     * chain of UiSessionData
     * */
    QStringList chainOfStates;

    SsoTest2PluginNS::SsoTest2Data testData;

    chainOfStates << "Browser" <<"Login" << "Captcha" << "LoginAndCaptcha";
    chainOfStates << "Browser" <<"Login" << "Captcha" << "LoginAndCaptcha";
    chainOfStates << "Browser" <<"Login" << "Captcha" << "LoginAndCaptcha";
    chainOfStates << "Browser" <<"Login" << "Captcha" << "LoginAndCaptcha";

    testData.setChainOfStates(chainOfStates);
    testData.setCurrentState(0);

    as->process(testData, "mech1");

    if (!errorCounter.count())
        loop.exec();

    QCOMPARE(spy.count(), 1);
    if (errorCounter.count())
        qDebug() << errorCounter.at(0).at(1).toString();

    QCOMPARE(errorCounter.count(), 0);


    SignOn::SessionData outData =
        spy.at(0).at(0).value<SignOn::SessionData>();
    SsoTest2PluginNS::SsoTest2Data resultData =
        outData.data<SsoTest2PluginNS::SsoTest2Data>();

    foreach(QString result, resultData.ChainOfResults())
        QCOMPARE(result, QString("OK"));
}

void TestAuthSession::processUi_and_cancel()
{
    AuthSession *as;
    SSO_TEST_CREATE_AUTH_SESSION(as, "ssotest2");
    g_currentSession = as;

    m_signOnUI->setDelay(4);

    QSignalSpy errorCounter(as, SIGNAL(error(const SignOn::Error &)));
    QSignalSpy stateCounter(as,
          SIGNAL(stateChanged(AuthSession::AuthSessionState, const QString&)));
    QSignalSpy spy(as, SIGNAL(response(const SignOn::SessionData&)));
    QEventLoop loop;

    QObject::connect(as, SIGNAL(response(const SignOn::SessionData&)),
                     &loop, SLOT(quit()));
    QObject::connect(as, SIGNAL(error(const SignOn::Error &)),
                     &loop, SLOT(quit()));
    QTimer::singleShot(500*1000, &loop, SLOT(quit()));

    /*
     * chain of UiSessionData
     * */
    QStringList chainOfStates;

    SsoTest2PluginNS::SsoTest2Data testData;

    chainOfStates << "Browser" <<"Login" << "Captcha" << "LoginAndCaptcha";
    chainOfStates << "Browser" <<"Login" << "Captcha" << "LoginAndCaptcha";
    chainOfStates << "Browser" <<"Login" << "Captcha" << "LoginAndCaptcha";
    chainOfStates << "Browser" <<"Login" << "Captcha" << "LoginAndCaptcha";

    testData.setChainOfStates(chainOfStates);
    testData.setCurrentState(0);

    as->process(testData, "mech1");
    QTimer::singleShot(3*1000, this, SLOT(cancel()));

    if (!errorCounter.count())
        loop.exec();

    QCOMPARE(spy.count(), 0);
    QCOMPARE(errorCounter.count(), 1);

    m_signOnUI->setDelay(0);
}

void TestAuthSession::windowId()
{
    AuthSession *as;
    SSO_TEST_CREATE_AUTH_SESSION(as, "ssotest2");

    QSignalSpy errorCounter(as, SIGNAL(error(const SignOn::Error &)));
    QSignalSpy spy(as, SIGNAL(response(const SignOn::SessionData&)));
    QEventLoop loop;

    QObject::connect(as, SIGNAL(response(const SignOn::SessionData&)),
                     &loop, SLOT(quit()));
    QObject::connect(as, SIGNAL(error(const SignOn::Error &)),
                     &loop, SLOT(quit()));
    QTimer::singleShot(500*1000, &loop, SLOT(quit()));

    /*
     * chain of UiSessionData
     * */
    QStringList chainOfStates;

    SsoTest2PluginNS::SsoTest2Data testData;

    chainOfStates << "Browser" << "Browser";
    testData.setChainOfStates(chainOfStates);
    testData.setCurrentState(0);
    testData.setWindowId(0xdeadbeef);

    as->process(testData, "mech1");
    if (!errorCounter.count())
        loop.exec();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(errorCounter.count(), 0);
    QCOMPARE(m_signOnUI->clientData().value("WindowId").toUInt(),
             0xdeadbeef);
    QCOMPARE(m_signOnUI->method(), QLatin1String("ssotest2"));
    QCOMPARE(m_signOnUI->mechanism(), QLatin1String("mech1"));
}
