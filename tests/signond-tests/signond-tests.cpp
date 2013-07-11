/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
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

#include "testpluginproxy.h"
#include "timeouts.h"
#include "backuptest.h"
#include "databasetest.h"

#ifdef CAM_UNIT_TESTS_FIXED
#include "credentialsaccessmanagertest.h"
#endif

#include <QCoreApplication>
#include <QtTest/QtTest>
#include <QtCore>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    int ret = 0;

    TestPluginProxy testPluginProxy;
    ret = QTest::qExec(&testPluginProxy, argc, argv);
    if (ret != 0) return ret;

    TimeoutsTest testTimeouts;
    ret = QTest::qExec(&testTimeouts, argc, argv);
    if (ret != 0) return ret;

#ifdef BACKUP_UNIT_TESTS_FIXED
    TestBackup testBackup;
    ret = QTest::qExec(&testBackup, argc, argv);
    if (ret != 0) return ret;
#endif

    TestDatabase testDatabase;
    ret = QTest::qExec(&testDatabase, argc, argv);
    if (ret != 0) return ret;

#ifdef CAM_UNIT_TESTS_FIXED
    CredentialsAccessManagerTest testCAM;
    ret = QTest::qExec(&testCAM, argc, argv);
    if (ret != 0) return ret;
#endif

    return ret;
}
