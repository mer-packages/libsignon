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

#include <QByteArray>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QEventLoop>
#include <QProcess>
#include <QSignalSpy>
#include <QTest>
#include <SignOn/Identity>

#define IDENTITY_TOOL "./identity-tool"

using namespace SignOn;

namespace SignOnTests {

class AccessControlTest: public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void onInfo(const SignOn::IdentityInfo &info);

private Q_SLOTS:
    void testOwner();
    void testAccessDenied();
    void testAccessDeniedSession();
    void testAccessAllowed();
    void testAccessRequestAllowed();
    void testAccessRequestAllowedSession();

private:
    IdentityInfo m_info;
};

} // namespace

using namespace SignOnTests;

void AccessControlTest::onInfo(const SignOn::IdentityInfo &info)
{
    qDebug() << "Owner:" <<info.owner();
    m_info = info;
}

void AccessControlTest::testOwner()
{
    QEventLoop loop;

    QMap<MethodName,MechanismsList> methods;
    methods.insert("dummy", QStringList() << "mech1" << "mech2");
    IdentityInfo info = IdentityInfo(QLatin1String("ac test"),
                                     QLatin1String("ac@test"),
                                     methods);
    Identity *identity = Identity::newIdentity(info, this);
    QVERIFY(identity != NULL);

    QObject::connect(identity, SIGNAL(error(const SignOn::Error&)),
                     &loop, SLOT(quit()));
    QObject::connect(identity, SIGNAL(credentialsStored(const quint32)),
                     &loop, SLOT(quit()));
    identity->storeCredentials();
    loop.exec();
    QVERIFY(identity->id() != 0);

    QObject::connect(identity, SIGNAL(info(const SignOn::IdentityInfo&)),
                     this, SLOT(onInfo(const SignOn::IdentityInfo&)));
    QObject::connect(identity, SIGNAL(info(const SignOn::IdentityInfo&)),
                     &loop, SLOT(quit()));
    identity->queryInfo();
    loop.exec();
    QCOMPARE(m_info.owner(), QCoreApplication::arguments().at(0));

    delete identity;
}

void AccessControlTest::testAccessDenied()
{
    /* Create an identity from another process */
    QProcess identityTool;
    identityTool.start(IDENTITY_TOOL,
                       QStringList() << "--caption" << "no-acl");
    QVERIFY(identityTool.waitForFinished());

    uint id = identityTool.readAll().toUInt();
    qDebug() << "Identity was created:" << id;
    Identity *identity = Identity::existingIdentity(id);
    QVERIFY(identity != NULL);

    QEventLoop loop;
    QSignalSpy infoSpy(identity, SIGNAL(info(const SignOn::IdentityInfo&)));
    QSignalSpy errorSpy(identity, SIGNAL(error(const SignOn::Error&)));
    QObject::connect(identity, SIGNAL(error(const SignOn::Error&)),
                     &loop, SLOT(quit()));
    QObject::connect(identity, SIGNAL(info(const SignOn::IdentityInfo&)),
                     &loop, SLOT(quit()));
    identity->queryInfo();
    loop.exec();

    QCOMPARE(infoSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 1);
    SignOn::Error error = errorSpy.at(0).at(0).value<SignOn::Error>();
    QCOMPARE(error.type(), int(SignOn::Error::PermissionDenied));

    delete identity;
}

void AccessControlTest::testAccessDeniedSession()
{
    /* Create an identity from another process */
    QProcess identityTool;
    identityTool.start(IDENTITY_TOOL,
                       QStringList() << "--caption" << "no-acl");
    QVERIFY(identityTool.waitForFinished());

    uint id = identityTool.readAll().toUInt();
    qDebug() << "Identity was created:" << id;
    AuthSession *session = new AuthSession(id, "dummy");
    QVERIFY(session != NULL);

    QEventLoop loop;
    QSignalSpy errorSpy(session, SIGNAL(error(const SignOn::Error&)));
    QObject::connect(session, SIGNAL(error(const SignOn::Error&)),
                     &loop, SLOT(quit()));
    session->process(SessionData());
    loop.exec();

    QCOMPARE(errorSpy.count(), 1);

    SignOn::Error error = errorSpy.at(0).at(0).value<SignOn::Error>();
    QCOMPARE(error.type(), int(SignOn::Error::PermissionDenied));

    delete session;
}

void AccessControlTest::testAccessAllowed()
{
    /* Create an identity from another process */
    QProcess identityTool;
    identityTool.start(IDENTITY_TOOL,
                       QStringList() << "--caption" << "with-acl" <<
                       "--acl" << QCoreApplication::arguments().at(0));
    QVERIFY(identityTool.waitForFinished());

    uint id = identityTool.readAll().toUInt();
    qDebug() << "Identity was created:" << id;
    Identity *identity = Identity::existingIdentity(id);
    QVERIFY(identity != NULL);

    QEventLoop loop;
    QSignalSpy infoSpy(identity, SIGNAL(info(const SignOn::IdentityInfo&)));
    QSignalSpy errorSpy(identity, SIGNAL(error(const SignOn::Error&)));
    QObject::connect(identity, SIGNAL(error(const SignOn::Error&)),
                     &loop, SLOT(quit()));
    QObject::connect(identity, SIGNAL(info(const SignOn::IdentityInfo&)),
                     &loop, SLOT(quit()));
    identity->queryInfo();
    loop.exec();

    QCOMPARE(infoSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);

    delete identity;
}

void AccessControlTest::testAccessRequestAllowed()
{
    /* Create an identity from another process; use the "allow" caption to tell
     * the mock-ac-plugin to allow requests to this identity.
     */
    QProcess identityTool;
    identityTool.start(IDENTITY_TOOL,
                       QStringList() << "--caption" << "allow");
    QVERIFY(identityTool.waitForFinished());

    uint id = identityTool.readAll().toUInt();
    qDebug() << "Identity was created:" << id;
    Identity *identity = Identity::existingIdentity(id);
    QVERIFY(identity != NULL);

    QEventLoop loop;
    QSignalSpy infoSpy(identity, SIGNAL(info(const SignOn::IdentityInfo&)));
    QSignalSpy errorSpy(identity, SIGNAL(error(const SignOn::Error&)));
    QObject::connect(identity, SIGNAL(error(const SignOn::Error&)),
                     &loop, SLOT(quit()));
    QObject::connect(identity, SIGNAL(info(const SignOn::IdentityInfo&)),
                     &loop, SLOT(quit()));
    identity->queryInfo();
    loop.exec();

    QCOMPARE(infoSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);

    /* Verify that our process has been added to the ACL (by the
     * mock-ac-plugin, after seeing our special caption).
     */
    SignOn::IdentityInfo info =
        infoSpy.at(0).at(0).value<SignOn::IdentityInfo>();
    QCOMPARE(info.caption(), QLatin1String("allow"));
    QCOMPARE(info.accessControlList(),
             QStringList() << QCoreApplication::arguments().at(0));

    delete identity;
}

void AccessControlTest::testAccessRequestAllowedSession()
{
    /* Create an identity from another process; use the "allow" caption to tell
     * the mock-ac-plugin to allow requests to this identity.
     */
    QProcess identityTool;
    identityTool.start(IDENTITY_TOOL,
                       QStringList() << "--caption" << "allow");
    QVERIFY(identityTool.waitForFinished());

    uint id = identityTool.readAll().toUInt();
    qDebug() << "Identity was created:" << id;
    AuthSession *session = new AuthSession(id, "dummy");
    QVERIFY(session != NULL);

    QEventLoop loop;
    QSignalSpy errorSpy(session, SIGNAL(error(const SignOn::Error&)));
    QObject::connect(session, SIGNAL(error(const SignOn::Error&)),
                     &loop, SLOT(quit()));
    session->process(SessionData());
    loop.exec();

    QCOMPARE(errorSpy.count(), 1);

    /* The error should be about the unknown "dummy" method, and not a
     * permission denied */
    SignOn::Error error = errorSpy.at(0).at(0).value<SignOn::Error>();
    QCOMPARE(error.type(), int(SignOn::Error::MethodNotAvailable));

    delete session;
}

QTEST_MAIN(AccessControlTest)
#include "tst_access_control.moc"
