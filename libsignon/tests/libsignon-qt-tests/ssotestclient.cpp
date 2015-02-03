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

#include "SignOn/uisessiondata.h"
#include "SignOn/uisessiondata_priv.h"
#include "signon-ui.h"
#include "ssotestclient.h"

#include <QEventLoop>
#include <QTimer>
#include <QTest>
#include <QThread>
#include <QDir>

using namespace SignOn;

int startedClients = 0;
int finishedClients = 0;

/*
 * test timeout 10 seconds
 * */
#define test_timeout 10000
#define pause_time 0

#define TEST_START qDebug("\n\n\n\n ----------------- %s ----------------\n\n",  __func__);

#define TEST_DONE  qDebug("\n\n ----------------- %s PASS ----------------\n\n",  __func__);

// ACL Tokens for the queryAuthPluginACL test
#define ACL_TOKEN_0 "token_0"
#define ACL_TOKEN_1 "token_1"
#define ACL_TOKEN_2 "libsignon-qt-tests::sso-encryption-token"
#define ACL_TOKEN_3 "token_3"
#define ACL_TOKEN_4 "token_4"
#define ACL_TOKEN_5 "AID::com.nokia.maemo.libsignon-qt-tests.libsignon-qt-tests-id"

SsoTestClient::SsoTestClient(SignOnUI *signOnUI, QObject *parent):
    QObject(parent),
    m_signOnUI(signOnUI)
{
}

void SsoTestClient::initTestCase()
{
    clearDB();
    initAuthServiceTest();
}

void SsoTestClient::cleanupTestCase()
{
    clearDB();
}

QString SsoTestClient::errCodeAsStr(const Error::ErrorType err)
{
    switch(err) {
    case Error::Unknown: return "Unknown";
    case Error::InternalServer: return "InternalServer";
    case Error::InternalCommunication: return "InternalCommunication";
    case Error::PermissionDenied: return "PermissionDenied";
    case Error::MethodNotKnown: return "MethodNotKnown";
    case Error::ServiceNotAvailable: return "ServiceNotAvailable";
    case Error::InvalidQuery: return "InvalidQuery";
    case Error::MethodNotAvailable: return "MethodNotAvailable";
    case Error::IdentityNotFound: return "IdentityNotFound";
    case Error::StoreFailed: return "StoreFailed";
    case Error::RemoveFailed: return "RemoveFailed";
    case Error::SignOutFailed: return "SignOutFailed";
    case Error::IdentityOperationCanceled: return "IdentityOperationCanceled";
    case Error::CredentialsNotAvailable: return "CredentialsNotAvailable";
    case Error::AuthSessionErr: return "AuthSessionErr";
    case Error::MechanismNotAvailable: return "MechanismNotAvailable";
    case Error::MissingData: return "MissingData";
    case Error::InvalidCredentials: return "InvalidCredentials";
    case Error::WrongState: return "WrongState";
    case Error::OperationNotSupported: return "OperationNotSupported";
    case Error::NoConnection: return "NoConnection";
    case Error::Network: return "Network";
    case Error::Ssl: return "Ssl";
    case Error::Runtime: return "Runtime";
    case Error::SessionCanceled: return "SessionCanceled";
    case Error::TimedOut: return "TimedOut";
    case Error::UserInteraction: return "UserInteraction";
    case Error::OperationFailed: return "OperationFailed";
    default: return "DEFAULT error type reached.";
    }
}

bool SsoTestClient::storeCredentialsPrivate(const SignOn::IdentityInfo &info)
{
    Identity *identity = Identity::newIdentity(info, this);

    QEventLoop loop;

    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(identity, SIGNAL(credentialsStored(const quint32)),
            &m_identityResult, SLOT(credentialsStored(const quint32)));
    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->storeCredentials();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    bool ok = false;
    if (m_identityResult.m_responseReceived == TestIdentityResult::NormalResp) {
        m_storedIdentityInfo = info;
        m_storedIdentityId = identity->id();
        ok = true;
    } else {
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << errCodeAsStr(m_identityResult.m_error);
        ok = false;
    }
    delete identity;
    return ok;
}

QString SsoTestClient::pluginsDir() const
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (env.contains(QLatin1String("SSO_PLUGINS_DIR"))) {
        return env.value(QLatin1String("SSO_PLUGINS_DIR"));
    } else {
        return QLatin1String(SIGNOND_PLUGINS_DIR);
    }
}

void SsoTestClient::queryAvailableMetods()
{
    TEST_START

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setAccessControlList(QStringList() << "*");

    if (!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for querying available methods.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId, this);

    if (identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    m_identityResult.reset();

    QEventLoop loop;

    connect(identity, SIGNAL(methodsAvailable(const QStringList &)),
            &m_identityResult, SLOT(methodsAvailable(const QStringList &)));

    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->queryAvailableMethods();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived !=
             TestIdentityResult::InexistentResp,
             "A response was not received.");

    if (m_identityResult.m_responseReceived == TestIdentityResult::NormalResp) {
        qDebug() << "Remote:" << m_identityResult.m_methods;
        qDebug() << "Local:" << m_storedIdentityInfo.methods();

        QVERIFY(m_identityResult.m_methods == m_storedIdentityInfo.methods());
    } else {
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << errCodeAsStr(m_identityResult.m_error);
        QVERIFY(false);
    }

    TEST_DONE
}

void SsoTestClient::requestCredentialsUpdate()
{
    TEST_START

    m_signOnUI->setPassword("Hello there, this is my password");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId, this);

    if (identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    m_identityResult.reset();

    QEventLoop loop;

    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));
    connect(identity, SIGNAL(credentialsStored(const quint32)),
            &m_identityResult, SLOT(credentialsStored(const quint32)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->requestCredentialsUpdate();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived !=
             TestIdentityResult::InexistentResp,
             "A response was not received.");

    qDebug() << m_signOnUI->parameters();
    QVERIFY(m_identityResult.m_responseReceived ==
            TestIdentityResult::NormalResp);

    /* Verify that the password is the one set by signon UI */
    connect(identity, SIGNAL(secretVerified(const bool)),
            &m_identityResult, SLOT(secretVerified(const bool)));

    identity->verifySecret(m_signOnUI->password());

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QCOMPARE(m_identityResult.m_responseReceived,
             TestIdentityResult::NormalResp);

    QVERIFY(m_identityResult.m_secretVerified);

    TEST_DONE
}

void SsoTestClient::storeCredentials()
{
    TEST_START

    if (!testAddingNewCredentials()) {
        QFAIL("Adding new credentials test failed.");
    }

    if (!testUpdatingCredentials()) {
        QFAIL("Updating existing credentials test failed.");
    }

    TEST_DONE
}

void SsoTestClient::remove()
{
    QSKIP("Skipping until secure storage gets stabilized.", SkipSingle);
    TEST_START
    m_identityResult.reset();

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setAccessControlList(QStringList() << "*");

    if (!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for removing identity.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId, this);
    if (identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    QEventLoop loop;

    connect(identity,
            SIGNAL(removed()),
            &m_identityResult,
            SLOT(removed()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->remove();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived !=
             TestIdentityResult::InexistentResp,
             "A response was not received.");

    if (m_identityResult.m_responseReceived == TestIdentityResult::NormalResp) {
        QVERIFY(m_identityResult.m_removed);
        connect(
            identity,
            SIGNAL(info(const SignOn::IdentityInfo &)),
            &m_identityResult,
            SLOT(info(const SignOn::IdentityInfo &)));

        qDebug() << "Going to query info";
        identity->queryInfo();

        QVERIFY(m_identityResult.m_responseReceived ==
                TestIdentityResult::ErrorResp);
        QVERIFY(m_identityResult.m_error == Error::IdentityNotFound);
    } else {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received error reply");
    }

    TEST_DONE
}

void SsoTestClient::sessionTest()
{
    TEST_START

    Identity *id = Identity::newIdentity();

    QLatin1String method1Str("mtd1");

    AuthSessionP session1 = id->createSession(method1Str);
    QVERIFY2(session1 != 0, "Could not create auth session.");

    AuthSessionP session2 = id->createSession(method1Str);
    QVERIFY2(session2 == 0, "Multiple auth. sessions with same method created.");

    AuthSessionP session3 = id->createSession("method2");
    QVERIFY2(session3 != 0, "Could not create auth session.");

    id->destroySession(session1);
    session2 = id->createSession(method1Str);
    AuthSessionP session5 = NULL;

    id->destroySession(session5);

    delete id;

    TEST_DONE
}

void SsoTestClient::removeStoreRemove()
{
    QSKIP("Skipping until secure storage gets stabilized.", SkipSingle);
    TEST_START
    m_identityResult.reset();

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setAccessControlList(QStringList() << "*");

    if (!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for removing identity.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId, this);
    if (identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    QEventLoop loop;

    connect(
            identity,
            SIGNAL(removed()),
            &m_identityResult,
            SLOT(removed()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    m_identityResult.reset();
    identity->remove();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    if (m_identityResult.m_responseReceived == TestIdentityResult::NormalResp) {
        QVERIFY(m_identityResult.m_removed);
        m_identityResult.reset();

        qDebug() << "Checking for removed identity ID : " << identity->id();;
        connect(identity, SIGNAL(credentialsStored(const quint32)),
                &m_identityResult, SLOT(credentialsStored(const quint32)));

        QMap<MethodName, MechanismsList> methods;
        methods.insert("method4", QStringList() << "mech10" << "mech20");
        methods.insert("method5", QStringList() << "mech10" << "mech20" << "mech30");
        IdentityInfo updateInfo("TEST_CAPTION_10",
                          "TEST_USERNAME_10",
                          methods);
        updateInfo.setSecret("TEST_PASSWORD_10");
        updateInfo.setAccessControlList(QStringList() << "*");

        m_identityResult.m_responseReceived = TestIdentityResult::InexistentResp;
        identity->storeCredentials(updateInfo);

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();

        if (m_identityResult.m_responseReceived ==
            TestIdentityResult::NormalResp) {
            m_storedIdentityInfo = info;
            m_storedIdentityId = identity->id();
            qDebug() << "Identity ID = " << m_storedIdentityId;
        } else {
            qDebug() << "Error reply: " << m_identityResult.m_errMsg << "."
                "\nError code: " << errCodeAsStr(m_identityResult.m_error);
            QFAIL("Failed to store the credentials");
        }

        connect(
            identity,
            SIGNAL(info(const SignOn::IdentityInfo &)),
            &m_identityResult,
            SLOT(info(const SignOn::IdentityInfo &)));

        qDebug() << "Checking if the identity info is updated in identity";
        m_identityResult.reset();
        identity->queryInfo();

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();

        QVERIFY(m_identityResult.m_responseReceived == TestIdentityResult::NormalResp);
        if (!TestIdentityResult::compareIdentityInfos(m_identityResult.m_idInfo,
                                                      updateInfo)) {
            QFAIL("Compared identity infos are not the same.");
        }

        qDebug() << "Removing after store";
        m_identityResult.reset();
        identity->remove();

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();

        QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
                 "A response was not received.");
        QVERIFY(m_identityResult.m_removed);

        qDebug() << "Removing again";
        m_identityResult.reset();
        identity->remove();

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();

        QVERIFY(m_identityResult.m_responseReceived == TestIdentityResult::ErrorResp);
        QVERIFY(m_identityResult.m_error == Error::IdentityNotFound);
    } else {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received error reply");
    }

    delete identity;

    TEST_DONE
}

void SsoTestClient::multipleRemove()
{
    QSKIP("Skipping until secure storage gets stabilized.", SkipSingle);
    TEST_START
    m_identityResult.reset();

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setAccessControlList(QStringList() << "*");

    if (!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for removing identity.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId, this);
    if (identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    QEventLoop loop;

    connect(
            identity,
            SIGNAL(removed()),
            &m_identityResult,
            SLOT(removed()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->remove();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    if (m_identityResult.m_responseReceived == TestIdentityResult::NormalResp) {
        QVERIFY(m_identityResult.m_removed);

        qDebug() << "Removing again";
        m_identityResult.reset();
        identity->remove();

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();

        QVERIFY(m_identityResult.m_responseReceived == TestIdentityResult::ErrorResp);
        QVERIFY(m_identityResult.m_error == Error::IdentityNotFound);
    } else {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received error reply");
    }

    delete identity;

    TEST_DONE
}

void SsoTestClient::storeCredentialsWithoutAuthMethodsTest()
{
    TEST_START

    if (!testAddingNewCredentials(false)) {
        QFAIL("Adding new credentials test failed.");
    }

    if (!testUpdatingCredentials()) {
        QFAIL("Updating existing credentials test failed.");
    }

    TEST_DONE
}

void SsoTestClient::queryInfo()
{
    TEST_START
    m_identityResult.reset();

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setRealms(QStringList() << "test_realm");
    QStringList acl;
    acl << "*";
    info.setAccessControlList(acl);

    if (!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for querying info.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId);

    if (identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    QEventLoop loop;

    connect(
            identity,
            SIGNAL(info(const SignOn::IdentityInfo &)),
            &m_identityResult,
            SLOT(info(const SignOn::IdentityInfo &)));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->queryInfo();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived !=
             TestIdentityResult::InexistentResp,
             "A response was not received.");

    QCOMPARE(m_identityResult.m_idInfo.isStoringSecret(), true);
    QCOMPARE(m_identityResult.m_idInfo.accessControlList(), acl);

    if (m_identityResult.m_responseReceived == TestIdentityResult::NormalResp) {
        QVERIFY(m_identityResult.m_id == m_storedIdentityId);
        QVERIFY(TestIdentityResult::compareIdentityInfos(
                m_storedIdentityInfo,
                m_identityResult.m_idInfo, false));
    } else {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received an error reply.");
    }

    TEST_DONE
}

void SsoTestClient::addReference()
{
    TEST_START
    m_identityResult.reset();

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setRealms(QStringList() << "test_realm");
    info.setAccessControlList(QStringList() << "*");

    if (!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for querying info.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId);

    if (identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");
    m_identityResult.m_removed = false;
    QEventLoop loop;

    connect(
            identity,
            SIGNAL(referenceAdded()),
            &m_identityResult,
            SLOT(referenceAdded()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->addReference(QLatin1String("testref"));

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived !=
             TestIdentityResult::InexistentResp,
             "A response was not received.");

    if (m_identityResult.m_responseReceived == TestIdentityResult::NormalResp) {
        QVERIFY(m_identityResult.m_removed);
    } else {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received an error reply.");
    }

    TEST_DONE
}

void SsoTestClient::removeReference()
{
    m_identityResult.reset();

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setRealms(QStringList() << "test_realm");
    info.setAccessControlList(QStringList() << "*");

    if (!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for querying info.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId);

    if (identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");
    m_identityResult.m_removed = false;
    QEventLoop loop;

    identity->addReference(QLatin1String("testref"));

    connect(
            identity,
            SIGNAL(referenceRemoved()),
            &m_identityResult,
            SLOT(referenceRemoved()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->removeReference(QLatin1String("testref"));

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    if (m_identityResult.m_responseReceived == TestIdentityResult::NormalResp) {
        QVERIFY(m_identityResult.m_removed);
    } else {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received an error reply.");
    }

    TEST_DONE
}

void SsoTestClient::verifyUser()
{
    TEST_START
    m_identityResult.reset();

    const QString password(QLatin1String("A strong password"));
    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret(password);
    info.setAccessControlList(QStringList() << "*");

    if (!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for verifying user.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId);

    if (identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    QEventLoop loop;

    connect(
            identity,
            SIGNAL(userVerified(const bool)),
            &m_identityResult,
            SLOT(userVerified(const bool)));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    m_signOnUI->setPassword(password);
    identity->verifyUser("message");

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived !=
             TestIdentityResult::InexistentResp,
             "A response was not received.");

    QVERIFY(m_identityResult.m_responseReceived ==
            TestIdentityResult::NormalResp);

    /* check that the parameters received by SignOnUI were correct */
    QVariantMap uiParameters = m_signOnUI->parameters();
    QCOMPARE(uiParameters.value(SSOUI_KEY_MESSAGE).toString(),
             QLatin1String("message"));
    QVERIFY(uiParameters.contains(SSOUI_KEY_QUERYPASSWORD));

    /* check that the verification was successful */
    QVERIFY(m_identityResult.m_userVerified);

    TEST_DONE
}

void SsoTestClient::verifySecret()
{
    TEST_START
    m_identityResult.reset();

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setAccessControlList(QStringList() << "*");

    if (!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for verifying secret.");

    Identity *identity = Identity::existingIdentity(m_storedIdentityId);
    if (identity == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    QEventLoop loop;

    connect(
            identity,
            SIGNAL(secretVerified(const bool)),
            &m_identityResult,
            SLOT(secretVerified(const bool)));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->verifySecret("TEST_PASSWORD_1");

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    if (m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
    {
        QVERIFY(m_identityResult.m_secretVerified);
    }
    else
    {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;
        QFAIL("Should not have received an error reply.");
    }

    TEST_DONE
}

void SsoTestClient::signOut()
{
    TEST_START

    m_identityResult.reset();
    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret("TEST_PASSWORD_1");
    info.setAccessControlList(QStringList() << "*");

    if (!storeCredentialsPrivate(info))
        QFAIL("Failed to initialize test for signing out.");

    //create the existing identities
    Identity *identity = Identity::existingIdentity(m_storedIdentityId);
    Identity *identity1 = Identity::existingIdentity(m_storedIdentityId);
    Identity *identity2 = Identity::existingIdentity(m_storedIdentityId);
    Identity *identity3 = Identity::existingIdentity(m_storedIdentityId);

    if (identity == NULL || identity1 == NULL || identity2 == NULL || identity3 == NULL)
        QFAIL("Could not create existing identity. '0' ID provided?");

    connect(
            identity,
            SIGNAL(signedOut()),
            &m_identityResult,
            SLOT(signedOut()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    //connect the other 2 identities to designated identity test result objects
    TestIdentityResult identityResult1;
    TestIdentityResult identityResult2;
    TestIdentityResult identityResult3;

    connect(
            identity1,
            SIGNAL(signedOut()),
            &identityResult1,
            SLOT(signedOut()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(
            identity2,
            SIGNAL(signedOut()),
            &identityResult2,
            SLOT(signedOut()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(
            identity3,
            SIGNAL(signedOut()),
            &identityResult3,
            SLOT(signedOut()));
    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    /* Interesting - NOT IN THE GOOD WAY !!!
        - this wait has to be added so that the last identity gets to be registered
          and so that the server can signal it to sign out, too, @TODO
    */
    QTest::qWait(100);

    identity->signOut();

    //this test is likelly to take longer
    QTest::qWait(2000);

    QVERIFY2(m_identityResult.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    QVERIFY2(identityResult1.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    QVERIFY2(identityResult2.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    QVERIFY2(identityResult3.m_responseReceived != TestIdentityResult::InexistentResp,
             "A response was not received.");

    if ((m_identityResult.m_responseReceived == TestIdentityResult::NormalResp)
       && (identityResult1.m_responseReceived == TestIdentityResult::NormalResp)
       && (identityResult2.m_responseReceived == TestIdentityResult::NormalResp)
       && (identityResult3.m_responseReceived == TestIdentityResult::NormalResp))
    {
       // probably will do something here in the future
    }
    else
    {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received error reply");
    }

    TEST_DONE
}

void SsoTestClient::clearDB()
{
    AuthService *service = new AuthService(this);
    QEventLoop loop;

    connect(service, SIGNAL(cleared()), &m_serviceResult, SLOT(cleared()));
    connect(service, SIGNAL(error(const SignOn::Error &)),
            &m_serviceResult, SLOT(error(const SignOn::Error &)));
    connect(&m_serviceResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    service->clear();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();
}

void SsoTestClient::initAuthServiceTest()
{
    //small params preparing
    m_numberOfInsertedCredentials = 5;
    m_expectedNumberOfMethods = (QDir(pluginsDir())).entryList(
            QStringList() << "*.so", QDir::Files).count();
    if (!m_expectedMechanisms.length())
        m_expectedMechanisms << "mech1" << "mech2" << "mech3" << "BLOB";

    m_methodToQueryMechanisms = "ssotest";

#ifdef SSO_TESTS_RUNNING_AS_UNTRUSTED
    return;
#endif

    /* clearing DB
     * This may fail if the CAM is not ready; since the CAM state is not
     * exposed in the client API, in case we get the InternalServer error
     * (which is emitted in these cases) we wait some time and we retry.
     */
    int retries = 0;
    do {
        clearDB();
        if (m_serviceResult.m_responseReceived != TestAuthServiceResult::ErrorResp)
            break;
        if (m_serviceResult.m_error != Error::InternalServer)
            break;

        QTest::qSleep(2000);
        retries++;
    } while (retries < 5);

    if (m_serviceResult.m_responseReceived != TestAuthServiceResult::NormalResp) {
        /* DB clearing can fail - if the secrets DB is not available.
         * Do not fail in general if the clearing of the DB reported an error;
         * just print the debug output. */
        QString codeStr = errCodeAsStr(m_serviceResult.m_error);
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << codeStr;
    }

    //inserting some credentials
    for(int i = 0; i < m_numberOfInsertedCredentials; i++)
    {
        QMap<MethodName, MechanismsList> methods;
        methods.insert("method1", QStringList() << "mech1" << "mech2");
        methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
        IdentityInfo info(QString("TEST_CAPTION_%1").arg(i),
                          QString("TEST_USERNAME_%1").arg(i),
                          methods);
        info.setSecret(QString("TEST_PASSWORD_%1").arg(i));
        Identity *identity = Identity::newIdentity(info);

        QEventLoop loop;

        connect(identity, SIGNAL(error(const SignOn::Error &)),
                &m_identityResult, SLOT(error(const SignOn::Error &)));
        connect(identity, SIGNAL(credentialsStored(const quint32)),
                &m_identityResult, SLOT(credentialsStored(const quint32)));
        connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

        identity->storeCredentials();

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();

        if (m_identityResult.m_responseReceived != TestIdentityResult::NormalResp) {

            QString codeStr = errCodeAsStr(m_identityResult.m_error);
            qDebug() << "Error reply: " << m_identityResult.m_errMsg
                     << ".\nError code: " << codeStr;

            QFAIL("Failed to prepare the AuthService test suite.");
        }

        delete identity;
    }
}

void SsoTestClient::queryMethods()
{
    TEST_START

    m_serviceResult.reset();

    AuthService service;

    QEventLoop loop;

    connect(
            &service,
            SIGNAL(methodsAvailable(const QStringList &)),
            &m_serviceResult,
            SLOT(methodsAvailable(const QStringList &)));
    connect(&service, SIGNAL(error(const SignOn::Error &)),
            &m_serviceResult, SLOT(error(const SignOn::Error &)));

    connect(&m_serviceResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    service.queryMethods();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_serviceResult.m_responseReceived != TestAuthServiceResult::InexistentResp,
             "A response was not received.");

    if (m_serviceResult.m_responseReceived == TestAuthServiceResult::NormalResp)
    {
        //this should compare the actual lists not only their count
        QCOMPARE(m_serviceResult.m_methods.count(), m_expectedNumberOfMethods);
    }
    else
    {
        QString codeStr = errCodeAsStr(m_serviceResult.m_error);
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received error reply");
    }

    TEST_DONE
}

void SsoTestClient::queryMechanisms()
{
    TEST_START
    m_serviceResult.reset();

    AuthService service;

    QEventLoop loop;

    connect(
            &service,
            SIGNAL(mechanismsAvailable(const QString &, const QStringList &)),
            &m_serviceResult,
            SLOT(mechanismsAvailable(const QString &, const QStringList &)));
    connect(&service, SIGNAL(error(const SignOn::Error &)),
            &m_serviceResult, SLOT(error(const SignOn::Error &)));

    connect(&m_serviceResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    service.queryMechanisms(m_methodToQueryMechanisms);

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_serviceResult.m_responseReceived != TestAuthServiceResult::InexistentResp,
             "A response was not received.");

    if (m_serviceResult.m_responseReceived == TestAuthServiceResult::NormalResp)
    {
        qDebug() << m_serviceResult.m_queriedMechsMethod;
        qDebug() << m_methodToQueryMechanisms;

        qDebug() << m_serviceResult.m_mechanisms.second;
        qDebug() << m_expectedMechanisms;

        QCOMPARE(m_serviceResult.m_queriedMechsMethod, m_methodToQueryMechanisms);
        bool equal = (m_serviceResult.m_mechanisms.second == m_expectedMechanisms);
        QVERIFY(equal);
    }
    else
    {
        QString codeStr = errCodeAsStr(m_serviceResult.m_error);
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << codeStr;
        QFAIL("Should not have received error reply");
    }

    TEST_DONE
}

void SsoTestClient::queryIdentities()
{
    TEST_START
    m_serviceResult.reset();

    AuthService service;

    QEventLoop loop;

    connect(&service,
            SIGNAL(identities(const QList<SignOn::IdentityInfo> &)),
            &m_serviceResult,
            SLOT(identities(const QList<SignOn::IdentityInfo> &)));
    connect(&service, SIGNAL(error(const SignOn::Error &)),
            &m_serviceResult, SLOT(error(const SignOn::Error &)));

    connect(&m_serviceResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    service.queryIdentities();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_serviceResult.m_responseReceived != TestAuthServiceResult::InexistentResp,
             "A response was not received.");

    if (m_serviceResult.m_responseReceived == TestAuthServiceResult::NormalResp)
    {
        QListIterator<IdentityInfo> it(m_serviceResult.m_identities);
        while (it.hasNext())
        {
            IdentityInfo info = it.next();
            qDebug() << "Identity record: "
                    << "id:" << info.id()
                    << " username: " << info.userName()
                    << " caption: " << info.caption()
                    << " methods:";

            foreach(QString method, info.methods())
                qDebug() << QPair<QString, QStringList>(method, info.mechanisms(method));

        }
        QCOMPARE(m_serviceResult.m_identities.count(),
                 m_numberOfInsertedCredentials);
    }
    else
    {
        QString codeStr = errCodeAsStr(m_serviceResult.m_error);
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << codeStr;
        QFAIL("Should not have received error reply");
    }

    TEST_DONE
}

void SsoTestClient::queryIdentitiesWithFilter()
{
    QSKIP("Test requires the implementation of the filtering feature.",
          SkipSingle);

    TEST_START
    m_serviceResult.reset();
    int filteredIdentitiesCount = 2;

    IdentityInfo info(QLatin1String("CAPTION"),
                      QLatin1String("TEST_FILTER_USERNAME"),
                      QMap<MethodName, MechanismsList>());
    QVERIFY(storeCredentialsPrivate(info));
    info.setRealms(QStringList() << QLatin1String("www.realm-filter.com"));
    QVERIFY(storeCredentialsPrivate(info));

    AuthService service;

    QEventLoop loop;

    connect(&service,
            SIGNAL(identities(const QList<SignOn::IdentityInfo> &)),
            &m_serviceResult,
            SLOT(identities(const QList<SignOn::IdentityInfo> &)));
    connect(&service, SIGNAL(error(const SignOn::Error &)),
            &m_serviceResult, SLOT(error(const SignOn::Error &)));

    connect(&m_serviceResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    QString userPattern = QString::fromLatin1("TEST_FILTER");
    QString realmPattern = QString::fromLatin1("*realm_filter*");
    AuthService::IdentityRegExp userRegexp(userPattern);
    AuthService::IdentityRegExp realmRegexp(realmPattern);

    QVERIFY(userPattern == userRegexp.pattern());
    QVERIFY(realmPattern == realmRegexp.pattern());
    QString patternCopy = realmRegexp.pattern();

    AuthService::IdentityFilter filter;
    filter.insert(AuthService::Username, userRegexp);
    filter.insert(AuthService::Realm, realmRegexp);
    service.queryIdentities(filter);

    service.queryIdentities();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_serviceResult.m_responseReceived != TestAuthServiceResult::InexistentResp,
             "A response was not received.");

    if (m_serviceResult.m_responseReceived == TestAuthServiceResult::NormalResp)
    {
        QListIterator<IdentityInfo> it(m_serviceResult.m_identities);
        while(it.hasNext())
        {
            IdentityInfo info = it.next();
            qDebug() << "Identity record: "
                    << "id:" << info.id()
                    << " username: " << info.userName()
                    << " caption: " << info.caption()
                    << " methods:";

            foreach (QString method, info.methods())
                qDebug() << QPair<QString, QStringList>(method, info.mechanisms(method));

        }
        QCOMPARE(m_serviceResult.m_identities.count(), filteredIdentitiesCount);
    }
    else
    {
        QString codeStr = errCodeAsStr(m_serviceResult.m_error);
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << codeStr;
        QFAIL("Should not have received error reply");
    }

    TEST_DONE
}

void SsoTestClient::queryAuthPluginACL()
{
    TEST_START
    QEventLoop loop;

    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    methods.insert("ssotest", QStringList() << "mech1");
    IdentityInfo info("test_caption_1",
                      "test_username_1",
                      methods);
    info.setSecret("test_password_1");
    info.setAccessControlList(QStringList() << ACL_TOKEN_0 << ACL_TOKEN_1 << ACL_TOKEN_2
                                            << ACL_TOKEN_3 << ACL_TOKEN_4 << ACL_TOKEN_5);

    Identity *id = Identity::newIdentity(info, this);

    connect(id,
            SIGNAL(credentialsStored(quint32)),
            &m_identityResult,
            SLOT(credentialsStored(quint32)));
    connect(id,
            SIGNAL(error(const SignOn::Error &)),
            &m_identityResult,
            SLOT(error(const SignOn::Error &)));

    connect(&m_identityResult,
            SIGNAL(testCompleted()),
            &loop,
            SLOT(quit()));

    id->storeCredentials();
    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    if (m_identityResult.m_responseReceived != TestIdentityResult::NormalResp) {
        qDebug() << "Error reply: " << m_identityResult.m_errMsg
                 << ".\nError code: " << errCodeAsStr(m_identityResult.m_error);
        QFAIL("Store identity failed.");
    }


    AuthSession *as = id->createSession(QLatin1String("ssotest"));
    connect(as,
            SIGNAL(response(const SignOn::SessionData &)),
            this,
            SLOT(response(const SignOn::SessionData &)));

    QSignalSpy errorSpy(as, SIGNAL(error(const SignOn::Error &)));
    QSignalSpy responseSpy(as, SIGNAL(response(const SignOn::SessionData &)));

    QObject::connect(as,
                     SIGNAL(response(const SignOn::SessionData &)),
                     &loop,
                     SLOT(quit()), Qt::QueuedConnection);
    QObject::connect(as, SIGNAL(error(const SignOn::Error &)), &loop, SLOT(quit()));
    QTimer::singleShot(10*1000, &loop, SLOT(quit()));

    SessionData inData;
    inData.setSecret("testSecret");
    inData.setUserName("testUsername");

    as->process(inData, "mech1");

    if (!errorSpy.count())
        loop.exec();


    QVERIFY(errorSpy.count() == 0);
    QVERIFY(responseSpy.count() == 1);

    QCOMPARE(m_tokenList.count(), 6);
    QVERIFY(m_tokenList.contains(ACL_TOKEN_5));

    TEST_DONE
}

void SsoTestClient::response(const SignOn::SessionData &data)
{
    m_tokenList = data.getAccessControlTokens();
}

void SsoTestClient::clear()
{
    QSKIP("Skipping until secure storage gets stabilized.", SkipSingle);
    TEST_START
    m_serviceResult.reset();

    AuthService service;

    QEventLoop loop;

    connect(
            &service,
            SIGNAL(cleared()),
            &m_serviceResult,
            SLOT(cleared()));
    connect(&service, SIGNAL(error(const SignOn::Error &)),
            &m_serviceResult, SLOT(error(const SignOn::Error &)));

    connect(&m_serviceResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    service.clear();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_serviceResult.m_responseReceived != TestAuthServiceResult::InexistentResp,
             "A response was not received.");

    if (m_serviceResult.m_responseReceived == TestAuthServiceResult::NormalResp) {
        connect(
                &service,
                SIGNAL(identities(const QList<SignOn::IdentityInfo> &)),
                &m_serviceResult,
                SLOT(identities(const QList<SignOn::IdentityInfo> &)));

        service.queryIdentities();

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();
        QVERIFY(m_serviceResult.m_identities.count() == 0);
    } else {
        QString codeStr = errCodeAsStr(m_serviceResult.m_error);
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << codeStr;

        QFAIL("Should not have received error reply");
    }

    TEST_DONE
}

bool SsoTestClient::testAddingNewCredentials(bool addMethods)
{
    m_identityResult.reset();

    QMap<MethodName, MechanismsList> methods;
    if (addMethods) {
        methods.insert("dummy", QStringList() << "mech1" << "mech2" << "mech3");
        methods.insert("dummy1", QStringList() << "mech11" << "mech12" << "mech13");
    }
    IdentityInfo info("TEST_CAPTION", "TEST_USERNAME", methods);
    info.setSecret("TEST_SECRET");
    info.setRealms(QStringList() << "TEST_REALM1" << "TEST_REALM2");

    Identity *identity = Identity::newIdentity(info, this);

    QEventLoop loop;

    connect(identity, SIGNAL(error(const SignOn::Error &)),
            &m_identityResult, SLOT(error(const SignOn::Error &)));

    connect(identity, SIGNAL(credentialsStored(const quint32)),
            &m_identityResult, SLOT(credentialsStored(const quint32)));
    connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    identity->storeCredentials();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    if (m_identityResult.m_responseReceived ==
        TestIdentityResult::InexistentResp) {
        qDebug() << "A response was not received.";
        return false;
    }

    if (m_identityResult.m_responseReceived == TestIdentityResult::NormalResp) {
        if (m_identityResult.m_id != identity->id()) {
            qDebug() << "Queried identity id does not match with stored data.";
            return false;
        }

        Identity *existingIdentity =
            Identity::existingIdentity(m_identityResult.m_id, this);
        if (existingIdentity == NULL) {
            qDebug() << "Could not create existing identity. '0' ID provided?";
            return false;
        }
        connect(existingIdentity, SIGNAL(info(const SignOn::IdentityInfo &)),
                &m_identityResult, SLOT(info(const SignOn::IdentityInfo &)));

        existingIdentity->queryInfo();

        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();
        delete existingIdentity;

        if (!TestIdentityResult::compareIdentityInfos(m_identityResult.m_idInfo,
                                                      info)) {
            qDebug() << "Compared identity infos are not the same.";
            return false;
        }
    } else {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << codeStr;
        return false;
    }
    return true;
}

bool SsoTestClient::testUpdatingCredentials(bool addMethods)
{
    // Test update credentials functionality

    Identity *existingIdentity = Identity::existingIdentity(m_identityResult.m_id, this);
    if (existingIdentity == NULL) {
        qDebug() << "Could not create existing identity. '0' ID provided?";
        return false;
    }

    QMap<MethodName, MechanismsList> methods;
    if (addMethods) {
        methods.insert("dummy1", QStringList() << "mech11" << "mech12" << "mech13");
        methods.insert("dummy2", QStringList() << "mech1_updated" << "mech2" << "mech1_updated2");
        methods.insert("dummy3", QStringList() << "mech1_updated" << "mech2" << "mech1_updated2");
    }

    IdentityInfo updateInfo("TEST_CAPTION", "TEST_USERNAME_UPDATED", methods);
    updateInfo.setSecret("TEST_SECRET_YES", false);

    do
    {
        QEventLoop loop;

        connect(existingIdentity, SIGNAL(error(const SignOn::Error &)),
                &m_identityResult, SLOT(error(const SignOn::Error &)));

        connect(existingIdentity, SIGNAL(credentialsStored(const quint32)),
                &m_identityResult, SLOT(credentialsStored(const quint32)));
        connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));

        existingIdentity->storeCredentials(updateInfo);
        qDebug();
        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();
    } while(0);

    qDebug();
    if (m_identityResult.m_responseReceived ==
        TestIdentityResult::InexistentResp) {
        qDebug() << "A response was not received.";
        return false;
    }

    if (m_identityResult.m_responseReceived == TestIdentityResult::NormalResp) {
        QEventLoop loop;
        connect(&m_identityResult, SIGNAL(testCompleted()), &loop, SLOT(quit()));
        connect(existingIdentity, SIGNAL(info(const SignOn::IdentityInfo &)),
                &m_identityResult, SLOT(info(const SignOn::IdentityInfo &)));

        existingIdentity->queryInfo();
        QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
        loop.exec();

        qDebug() << "ID:" << existingIdentity->id();
        if (!TestIdentityResult::compareIdentityInfos(m_identityResult.m_idInfo,
                                                      updateInfo)) {
            qDebug() << "Compared identity infos are not the same.";
            return false;
        }
    } else {
        QString codeStr = errCodeAsStr(m_identityResult.m_error);
        qDebug() << "Error reply: " << m_serviceResult.m_errMsg
                 << ".\nError code: " << codeStr;

        return false;
    }
    return true;
}

void SsoTestClient::emptyPasswordRegression()
{
    TEST_START

    m_identityResult.reset();

    QString myPassword("My password");
    //inserting some credentials
    QMap<MethodName, MechanismsList> methods;
    methods.insert("method1", QStringList() << "mech1" << "mech2");
    methods.insert("method2", QStringList() << "mech1" << "mech2" << "mech3");
    IdentityInfo info("TEST_CAPTION_1",
                      "TEST_USERNAME_1",
                      methods);
    info.setSecret(myPassword);
    info.setRealms(QStringList() << "test_realm");
    QStringList acl;
    acl << "*";
    info.setAccessControlList(acl);

    Identity *identity = Identity::newIdentity(info);

    QEventLoop loop;

    const char *errorSignature = SIGNAL(error(const SignOn::Error &));
    QSignalSpy errorSignal(identity, errorSignature);
    connect(identity, errorSignature, &loop, SLOT(quit()));

    const char *credentialsStoredSignature =
        SIGNAL(credentialsStored(const quint32));
    QSignalSpy credentialsStoredSignal(identity, credentialsStoredSignature);
    connect(identity, credentialsStoredSignature, &loop, SLOT(quit()));

    identity->storeCredentials();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QCOMPARE(errorSignal.count(), 0);
    QCOMPARE(credentialsStoredSignal.count(), 1);
    credentialsStoredSignal.clear();

    /* Verify that the password is the one set by signon UI */
    const char *secretVerifiedSignature = SIGNAL(secretVerified(const bool));
    QSignalSpy secretVerifiedSignal(identity, secretVerifiedSignature);
    connect(identity, secretVerifiedSignature, &loop, SLOT(quit()));

    identity->verifySecret(myPassword);

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QCOMPARE(secretVerifiedSignal.count(), 1);
    QCOMPARE(secretVerifiedSignal.at(0).at(0).toBool(), true);
    secretVerifiedSignal.clear();

    /* Now get the info, and re-store the identity */
    const char *infoSignature = SIGNAL(info(const SignOn::IdentityInfo &));
    connect(identity, infoSignature,
            &m_identityResult, SLOT(info(const SignOn::IdentityInfo &)));
    connect(identity, infoSignature, &loop, SLOT(quit()));

    identity->queryInfo();

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QVERIFY2(m_identityResult.m_responseReceived !=
             TestIdentityResult::InexistentResp,
             "A response was not received.");
    QCOMPARE(m_identityResult.m_idInfo.isStoringSecret(), true);
    QCOMPARE(m_identityResult.m_idInfo.secret(), QString());

    /* Write it back, and verify that the password doesn't change.
     * Change the username to make sure that this is not a no-op. */
    QString myUserName("Bob");
    m_identityResult.m_idInfo.setUserName(myUserName);

    identity->storeCredentials(m_identityResult.m_idInfo);

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    /* check that the store succeeded */
    QCOMPARE(errorSignal.count(), 0);
    QCOMPARE(credentialsStoredSignal.count(), 1);
    credentialsStoredSignal.clear();

    identity->verifySecret(myPassword);

    QTimer::singleShot(test_timeout, &loop, SLOT(quit()));
    loop.exec();

    QCOMPARE(secretVerifiedSignal.count(), 1);
    QCOMPARE(secretVerifiedSignal.at(0).at(0).toBool(), true);

    delete identity;

    TEST_DONE
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    SignOnUI signOnUI(QDBusConnection::sessionBus());
    int ret;

    SsoTestClient ssoTestClient(&signOnUI);
    ret = QTest::qExec(&ssoTestClient, argc, argv);
    if (ret != 0) return ret;

    TestAuthSession testAuthSession(&signOnUI);
    ret = QTest::qExec(&testAuthSession, argc, argv);
    return ret;
}
