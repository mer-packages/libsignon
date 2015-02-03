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
#include "backuptest.h"
#include "signond/signoncommon.h"
#include <QDBusMessage>
#include <unistd.h>

void TestBackup::initTestCase()
{
    /*
    daemonProcess = new QProcess();
    daemonProcess->start("signond");
    daemonProcess->waitForStarted(10 * 1000);
    */
}

void TestBackup::cleanupTestCase()
{
/*
    daemonProcess->kill();
    daemonProcess->waitForFinished();
    delete daemonProcess;
*/
}
void TestBackup::init()
{
    //wait a bit between tests
    sleep(5);
}
void TestBackup::cleanup()
{
    sleep(1);
}

void TestBackup::backupTest()
{
    QDBusConnection conn (SIGNOND_BUS);

    //remove backup files
    QFile::remove("/home/user/.signon/signondb.bin");

    QDBusMessage msg =
        QDBusMessage::createMethodCall(SIGNOND_SERVICE + ".Backup",
                                       SIGNOND_DAEMON_OBJECTPATH + "/Backup",
                                       "com.nokia.backupclient",
                                       "backupStarts");
    QList<QVariant> args;
    msg.setArguments(args);

    QDBusMessage reply = conn.call(msg, QDBus::Block, 10*1000);

    QVERIFY(reply.type() == QDBusMessage::ReplyMessage);

    //check if backup file was created successfully
    QVERIFY(QFile::exists("/home/user/.signon/signondb.bin"));
    QFile::copy("/home/user/.signon/signondb.bin",
                "/home/user/.signon/signondb.bin2");

    msg = QDBusMessage::createMethodCall(SIGNOND_SERVICE + ".Backup",
                                         SIGNOND_DAEMON_OBJECTPATH + "/Backup",
                                         "com.nokia.backupclient",
                                         "backupFinished");
    reply = conn.call(msg, QDBus::Block, 10*1000);

    QVERIFY(reply.type() == QDBusMessage::ReplyMessage);
    QVERIFY(QFile::exists("/home/user/.signon/signondb.bin") == false);

}
void TestBackup::restoreTest()
{
    QDBusConnection conn (SIGNOND_BUS);

    QVERIFY(QFile::exists("/home/user/.signon/signondb.bin2"));
    QFile::rename("/home/user/.signon/signondb.bin2",
                  "/home/user/.signon/signondb.bin");

    QDBusMessage msg =
        QDBusMessage::createMethodCall(SIGNOND_SERVICE + ".Backup",
                                       SIGNOND_DAEMON_OBJECTPATH + "/Backup",
                                       "com.nokia.backupclient",
                                       "restoreStarts");
    QList<QVariant> args;
    msg.setArguments(args);

    QDBusMessage reply = conn.call(msg, QDBus::Block, 10*1000);

    QVERIFY(reply.type() == QDBusMessage::ReplyMessage);

    //check if backup file was created successfully

    msg = QDBusMessage::createMethodCall(SIGNOND_SERVICE + ".Backup",
                                         SIGNOND_DAEMON_OBJECTPATH + "/Backup",
                                         "com.nokia.backupclient",
                                         "restoreFinished");
    reply = conn.call(msg, QDBus::Block, 10*1000);

    QVERIFY(QFile::exists("/home/user/.signon/signondb.bin") == false);
    QVERIFY(QFile::exists("/home/user/.signon/signon.db"));

    QVERIFY(reply.type() == QDBusMessage::ReplyMessage);

}

void TestBackup::backupNormalTest()
{

    daemonProcess = new QProcess();
    daemonProcess->start("signond");
    daemonProcess->waitForStarted(10 * 1000);

    backupTest();

    //check that daemon is still running normally after backup close signal
    QVERIFY(daemonProcess->state()==QProcess::Running);
    daemonProcess->kill();
    daemonProcess->waitForFinished();
    delete daemonProcess;
    daemonProcess = NULL;
}
void TestBackup::restoreNormalTest()
{

    daemonProcess = new QProcess();
    daemonProcess->start("signond");
    daemonProcess->waitForStarted(10 * 1000);

    restoreTest();

    //check that daemon is still running normally after backup close signal
    QVERIFY(daemonProcess->state()==QProcess::Running);
    daemonProcess->kill();
    daemonProcess->waitForFinished();
    delete daemonProcess;
    daemonProcess = NULL;

}

QTEST_MAIN(TestBackup)
