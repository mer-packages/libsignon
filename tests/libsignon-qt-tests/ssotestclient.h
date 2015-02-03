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

#ifndef SSOTESTCLIENT_H
#define SSOTESTCLIENT_H

#include <QObject>

#include "testauthsession.h"
#include "testidentityresult.h"
#include "testauthserviceresult.h"

#include "SignOn/signonerror.h"

class SignOnUI;

class SsoTestClient: public QObject
{
    Q_OBJECT

public:
    SsoTestClient(SignOnUI *signOnUi, QObject *parent = 0);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    /*
     * AuthService tests
     */
    void queryIdentities();
    void queryMethods();
    void queryMechanisms();
    void clear();

    /*
     * Identity tests
     */
    void queryAvailableMetods();
    void storeCredentials();
    void requestCredentialsUpdate();
    void queryInfo();
    void addReference();
    void removeReference();
    void verifyUser();
    void verifySecret();
    void signOut();
    void remove();
    void storeCredentialsWithoutAuthMethodsTest();
    void sessionTest();
    void multipleRemove();
    void removeStoreRemove();
    void queryAuthPluginACL();
    void emptyPasswordRegression();

private:
    void clearDB();

    void initAuthServiceTest();

    void queryIdentitiesWithFilter();

    /*
     * Subtests
     * */
    bool testUpdatingCredentials(bool addMethods = true);
    bool testAddingNewCredentials(bool addMethods = true);

    /*
     * Helpers
     * */
    static QString errCodeAsStr(const Error::ErrorType);
    bool storeCredentialsPrivate(const SignOn::IdentityInfo &info);
    QString pluginsDir() const;

protected Q_SLOTS:
    void response(const SignOn::SessionData &data);

private:
    SignOnUI *m_signOnUI;
    QStringList m_tokenList;
    int m_expectedNumberOfMethods;
    QStringList m_expectedMechanisms;
    int m_numberOfInsertedCredentials;

    quint32 m_storedIdentityId;
    IdentityInfo m_storedIdentityInfo;
    QString m_methodToQueryMechanisms;

    TestIdentityResult m_identityResult;
    TestAuthServiceResult m_serviceResult;
};

#endif // SSOTESTCLIENT_H
