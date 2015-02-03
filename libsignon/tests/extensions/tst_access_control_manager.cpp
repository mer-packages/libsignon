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

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QSignalSpy>
#include <QTest>

#include <SignOn/AbstractAccessControlManager>

using namespace SignOn;

class AccessControlManagerTest: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test_abstract();
    void test_abstractRequest();
};

void AccessControlManagerTest::test_abstract()
{
    AbstractAccessControlManager *acm = new AbstractAccessControlManager(this);

    /* forge a QDBusMessage and a connection */
    QDBusMessage msg =
        QDBusMessage::createMethodCall(":0.3", "/", "my.interface", "hi");
    QDBusConnection conn(QLatin1String("test-connection"));

    QCOMPARE(acm->isPeerAllowedToAccess(conn, msg, QLatin1String("any")),
             true);

    QCOMPARE(acm->appIdOfPeer(conn, msg), QString());

    QCOMPARE(acm->keychainWidgetAppId(), QString());

    delete acm;
}

void AccessControlManagerTest::test_abstractRequest()
{
    AbstractAccessControlManager *acm = new AbstractAccessControlManager(this);

    /* forge a QDBusMessage and a connection */
    QDBusMessage msg =
        QDBusMessage::createMethodCall(":0.3", "/", "my.interface", "hi");
    QDBusConnection conn(QLatin1String("test-connection"));

    AccessRequest request;
    request.setPeer(conn, msg);
    request.setIdentity(4);

    QCOMPARE(request.peerConnection().name(), conn.name());
    QCOMPARE(request.peerMessage().member(), msg.member());
    QCOMPARE(request.identity(), quint32(4));

    AccessReply *reply = acm->handleRequest(request);
    QVERIFY(reply != 0);
    QVERIFY(!reply->isAccepted());

    QSignalSpy finished(reply, SIGNAL(finished()));
    QTest::qWait(10);
    QCOMPARE(finished.count(), 1);
    QVERIFY(reply->isAccepted());
    QCOMPARE(reply->request().identity(), request.identity());
    delete reply;

    delete acm;
}

QTEST_MAIN(AccessControlManagerTest)
#include "tst_access_control_manager.moc"
