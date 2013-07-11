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

#ifndef DATABASETEST_H_
#define DATABASETEST_H_

#include <QtTest/QtTest>
#include <QtCore>

#include "signond/signoncommon.h"
#include "credentialsdb.h"
#include "default-secrets-storage.h"
#include "signonidentityinfo.h"

using namespace SignOn;
using namespace SignonDaemonNS;

class TestDatabase: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void createTableStructureTest();
    void queryListTest();
    void insertMethodsTest();

    void methodsTest();
    void checkPasswordTest();
    void credentialsTest();
    void insertCredentialsTest();
    void updateCredentialsTest();
    void removeCredentialsTest();
    void clearTest();

    void dataTest();
    void referenceTest();
    void cacheTest();

    void accessControlListTest();
    void credentialsOwnerSecurityTokenTest();

private:
    CredentialsDB *m_db;
    DefaultSecretsStorage *m_secretsStorage;
    MetaDataDB *m_meta;
    MethodMap testMethods;
    QStringList testRealms;
    QStringList testAcl;
};

#endif //DATABASETEST_H_
