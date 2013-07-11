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
#include "databasetest.h"
#include "signond/signoncommon.h"
#include <QDBusMessage>

#include "credentialsdb.h"
#include "signonidentityinfo.cpp"

const QString dbFile = QLatin1String("/tmp/signon_test.db");
const QString secretsDbFile = QLatin1String("/tmp/signon_test_secrets.db");

void TestDatabase::initTestCase()
{
    QFile::remove(dbFile);
    QFile::remove(secretsDbFile);
    m_secretsStorage = new DefaultSecretsStorage();
    m_db = new CredentialsDB(dbFile, m_secretsStorage);
    m_meta = m_db->metaDataDB;
    QVERIFY(m_db != 0);

    QStringList mechs = QStringList() <<
        QString::fromLatin1("Mech1") <<
        QString::fromLatin1("Mech2");
    testMethods.insert(QLatin1String("Method1"), mechs);
    testMethods.insert(QLatin1String("Method2"), mechs);
    testMethods.insert(QLatin1String("Method3"), QStringList());

    testRealms = QStringList() <<
        QLatin1String("Realm1.com") <<
        QLatin1String("Realm2.com") <<
        QLatin1String("Realm3.com");

    testAcl = QStringList() <<
        QLatin1String("AID::12345678") <<
        QLatin1String("AID::87654321") <<
        QLatin1String("test::property");
}

void TestDatabase::cleanupTestCase()
{
    delete m_db;
    m_db = NULL;
    delete m_secretsStorage;
    m_secretsStorage = 0;
    //remove database file
    //QFile::remove(dbFile);
}

void TestDatabase::init()
{
}

void TestDatabase::cleanup()
{
    if (m_db->isSecretsDBOpen())
        m_db->closeSecretsDB();
}

void TestDatabase::createTableStructureTest()
{
    QVERIFY(!m_meta->hasTables());
    m_db->init();
    QVERIFY(m_meta->hasTables());

    bool success = m_db->openSecretsDB(secretsDbFile);
    QVERIFY(success);
}

void TestDatabase::queryListTest()
{
    QString queryStr;
    /*create test table*/
    queryStr = QString::fromLatin1(
                    "CREATE TABLE TESTING"
                    "(identity_id INTEGER ,"
                    "realm TEXT,"
                    "hostname TEXT,"
                    "PRIMARY KEY (identity_id, realm, hostname))");
    QSqlQuery insertQuery = m_meta->exec(queryStr);
    m_meta->commit();

    queryStr = QString::fromLatin1(
            "INSERT INTO TESTING (identity_id, realm) "
            "VALUES('%1', '%2')")
            .arg(80).arg(QLatin1String("a"));
    insertQuery = m_meta->exec(queryStr);
    queryStr = QString::fromLatin1(
            "INSERT INTO TESTING (identity_id, realm) "
            "VALUES('%1', '%2')")
            .arg(80).arg(QLatin1String("b"));
    insertQuery = m_meta->exec(queryStr);
    QStringList list = m_meta->queryList(QString::fromLatin1(
            "SELECT realm FROM TESTING WHERE identity_id = 80"));
    QVERIFY(list.contains(QLatin1String("a")));
    QVERIFY(list.contains(QLatin1String("b")));
    QVERIFY(list.count() == 2);

    list = m_meta->queryList(QString::fromLatin1(
            "SELECT realm FROM TESTING WHERE identity_id = 81"));
    QVERIFY(list.count() == 0);
}

void TestDatabase::insertMethodsTest()
{
    //test empty list
    QMap<QString, QStringList> methods;
    m_meta->insertMethods(methods);
    QStringList list = m_meta->queryList(QString::fromLatin1(
            "SELECT method FROM METHODS"));
    QVERIFY(list.count() == 0);
    list = m_meta->queryList(QString::fromLatin1(
            "SELECT mechanism FROM MECHANISMS"));
    QVERIFY(list.count() == 0);

    //test real list
    QStringList mechs =
        QStringList() << QLatin1String("M1")<< QLatin1String("M2");
    methods.insert(QLatin1String("Test"), mechs);
    methods.insert(QLatin1String("Test2"), mechs);
    m_meta->insertMethods(methods);
    list = m_meta->queryList(QString::fromLatin1(
            "SELECT method FROM METHODS"));
    qDebug() << list;
    QVERIFY(list.contains(QLatin1String("Test")));
    QVERIFY(list.contains(QLatin1String("Test2")));
    QVERIFY(list.count() == 2);

    list = m_meta->queryList(QString::fromLatin1(
            "SELECT mechanism FROM MECHANISMS"));
    qDebug() << list;
    QVERIFY(list.contains(QLatin1String("M1")));
    QVERIFY(list.contains(QLatin1String("M2")));
    QVERIFY(list.count() == 2);
}

void TestDatabase::methodsTest()
{
    quint32 id;

    //insert complete
    SignonIdentityInfo info =
        SignonIdentityInfo(0,
                           QLatin1String("User"),
                           QLatin1String("Pass"), true,
                           QLatin1String("Caption"),
                           testMethods,
                           testRealms,
                           testAcl,
                           testAcl);

    id = m_db->insertCredentials(info);

    QStringList meths = m_db->methods(id);
    QVERIFY(meths.contains(QLatin1String("Method1")));
    QVERIFY(meths.contains(QLatin1String("Method2")));
    QVERIFY(meths.contains(QLatin1String("Method3")));
    QVERIFY(meths.count() == 3);
}

void TestDatabase::checkPasswordTest()
{
    quint32 id;

    m_db->openSecretsDB(secretsDbFile);

    //insert complete
    SignonIdentityInfo info =
        SignonIdentityInfo(0,
                           QLatin1String("User"),
                           QLatin1String("Pass"), true,
                           QLatin1String("Caption"),
                           testMethods,
                           testRealms,
                           testAcl);

    id = m_db->insertCredentials(info);

    QVERIFY(m_db->checkPassword(id, info.userName(), info.password()));
    QVERIFY(!m_db->checkPassword(id, info.userName(), QLatin1String("PassWd")));
    QVERIFY(!m_db->checkPassword(id, QLatin1String("User2"), info.password()));
    QVERIFY(!m_db->checkPassword(id, info.userName(), "' or password like '%"));
}

void TestDatabase::credentialsTest()
{
    m_db->openSecretsDB(secretsDbFile);
    m_db->clear();
    QMap<QString, QString> filter;
    QList<SignonIdentityInfo> creds = m_db->credentials(filter);
    QVERIFY(creds.count() == 0);

    //insert complete
    SignonIdentityInfo info =
        SignonIdentityInfo(0,
                           QLatin1String("User"),
                           QLatin1String("Pass"), true,
                           QLatin1String("Caption"),
                           testMethods,
                           testRealms,
                           testAcl);

    m_db->insertCredentials(info);
    creds = m_db->credentials(filter);
    QVERIFY(creds.count() == 1);
    m_db->insertCredentials(info);
    creds = m_db->credentials(filter);
    QVERIFY(creds.count() == 2);
    foreach(SignonIdentityInfo info, creds) {
        qDebug() << info.id() << info.caption();
    }
    //TODO check filtering when implemented
}

void TestDatabase::insertCredentialsTest()
{
    SignonIdentityInfo info;
    SignonIdentityInfo info2;
    SignonIdentityInfo retInfo;
    quint32 id;

    //insert empty
    id = m_db->insertCredentials(info);
    retInfo = m_db->credentials(id, false);
    QVERIFY(id != info.id());
    info.setId(id);
    QVERIFY(retInfo == info);

    //insert with empty acl
    info =
        SignonIdentityInfo(0,
                           QLatin1String("User"),
                           QLatin1String("Pass"), true,
                           QLatin1String("Caption"),
                           testMethods,
                           testRealms);

    id = m_db->insertCredentials(info);
    retInfo = m_db->credentials(id, false);
    QVERIFY(id != info.id());
    info.setId(id);
    retInfo.setPassword(info.password());
    QVERIFY(retInfo == info);

    //insert with empty methods
    MethodMap methods2;
    info =
        SignonIdentityInfo(0,
                           QLatin1String("User"),
                           QLatin1String("Pass"), true,
                           QLatin1String("Caption"),
                           methods2,
                           testRealms);

    id = m_db->insertCredentials(info2);
    retInfo = m_db->credentials(id, false);
    QVERIFY(id != info2.id());
    info2.setId(id);
    retInfo.setPassword(info2.password());
    QVERIFY(retInfo == info2);

    //insert complete
    info =
        SignonIdentityInfo(0,
                           QLatin1String("User"),
                           QLatin1String("Pass"), true,
                           QLatin1String("Caption"),
                           testMethods,
                           testRealms,
                           testAcl);

    id = m_db->insertCredentials(info);
    retInfo = m_db->credentials(id, false);
    QVERIFY(id != info.id());
    info.setId(id);
    QVERIFY(info.password() != retInfo.password());
    retInfo.setPassword(info.password());

    qDebug() << info.methods();
    qDebug() << retInfo.methods();
    QVERIFY (info.methods() == retInfo.methods());

    QVERIFY(retInfo == info);

    //with password and secrets DB disabled
    id = m_db->insertCredentials(info);
    retInfo = m_db->credentials(id, true);
    QVERIFY(id != info.id());
    info.setId(id);
    QCOMPARE(retInfo, info);

    //with password and secrets DB enabled
    bool success = m_db->openSecretsDB(secretsDbFile);
    QVERIFY(success);
    id = m_db->insertCredentials(info);
    retInfo = m_db->credentials(id, true);
    QVERIFY(id != info.id());
    info.setId(id);
    QVERIFY(retInfo == info);
}

void TestDatabase::updateCredentialsTest()
{
    SignonIdentityInfo retInfo;
    quint32 id;

    bool success = m_db->openSecretsDB(secretsDbFile);
    QVERIFY(success);

    //insert complete
    SignonIdentityInfo info =
        SignonIdentityInfo(0,
                           QLatin1String("User"),
                           QLatin1String("Pass"), true,
                           QLatin1String("Caption"),
                           testMethods,
                           testRealms,
                           testAcl);

    id = m_db->insertCredentials(info);
    retInfo = m_db->credentials(id, true);
    QVERIFY(id != info.id());
    info.setId(id);
    QVERIFY(retInfo == info);

    //update complete
    QStringList urealms = QStringList() <<
        QLatin1String("URealm1.com") <<
        QLatin1String("URealm2.com") <<
        QLatin1String("Realm3.com");

    MethodMap umethods;
    QStringList umechs =
        QStringList() << QString::fromLatin1("UMech1") <<
        QString::fromLatin1("Mech2") ;
    umethods.insert(QLatin1String("Method1"), umechs);
    umethods.insert(QLatin1String("UMethod2"), umechs);
    umethods.insert(QLatin1String("Method3"), QStringList());

    SignonIdentityInfo updateInfo =
        SignonIdentityInfo(id,
                           QLatin1String("UpUser"),
                           QLatin1String("UpdatedPass"), true,
                           QLatin1String("Updated Caption"),
                           umethods,
                           urealms,
                           testAcl,
                           testAcl,
                           2, 0, false);

    QVERIFY(m_db->updateCredentials(updateInfo));

    retInfo = m_db->credentials(id, true);

    QVERIFY(!(retInfo == info));
    QVERIFY((retInfo == updateInfo));
}

void TestDatabase::removeCredentialsTest()
{
    SignonIdentityInfo retInfo;
    quint32 id;

    bool success = m_db->openSecretsDB(secretsDbFile);
    QVERIFY(success);

    //insert complete
    SignonIdentityInfo info =
        SignonIdentityInfo(0,
                           QLatin1String("User"),
                           QLatin1String("Pass"), true,
                           QLatin1String("Caption"),
                           testMethods,
                           testRealms,
                           testAcl);

    id = m_db->insertCredentials(info);
    retInfo = m_db->credentials(id, true);
    QVERIFY(id != info.id());
    info.setId(id);
    QVERIFY(retInfo == info);

    QSqlQuery query;
    QString queryStr = QString::fromLatin1(
            "SELECT * FROM ACL WHERE identity_id = '%1'")
           .arg(id);
    query = m_meta->exec(queryStr);
    QVERIFY(query.first());

    QVERIFY(m_db->removeCredentials(id));

    retInfo = m_db->credentials(id, true);
    QVERIFY(!(retInfo == info));
    QVERIFY(retInfo.id() == 0);

    //check that db is cleared
    queryStr = QString::fromLatin1(
            "SELECT * FROM ACL WHERE identity_id = '%1'")
           .arg(id);
    query = m_meta->exec(queryStr);
    QVERIFY(!query.first());

    queryStr = QString::fromLatin1(
            "SELECT * FROM REALMS WHERE identity_id = '%1'")
           .arg(id);
    query = m_meta->exec(queryStr);
    QVERIFY(!query.first());

    queryStr = QString::fromLatin1(
            "SELECT * FROM REFS WHERE identity_id = '%1'")
           .arg(id);
    query = m_meta->exec(queryStr);
    QVERIFY(!query.first());
}

void TestDatabase::clearTest()
{
    /* we expact this to fail, because the secrets DB is not available */
    QVERIFY(!m_db->clear());
    QSqlQuery query = m_meta->exec(QLatin1String("SELECT * FROM credentials"));
    QVERIFY(query.first());

    bool success = m_db->openSecretsDB(secretsDbFile);
    QVERIFY(success);

    QVERIFY(m_db->clear());
    query = m_meta->exec(QLatin1String("SELECT * FROM credentials"));
    QVERIFY(!query.first());
}

void TestDatabase::dataTest()
{
    quint32 id;
    QString method = QLatin1String("Method1");
    //insert complete
    SignonIdentityInfo info =
        SignonIdentityInfo(0,
                           QLatin1String("User"),
                           QLatin1String("Pass"), true,
                           QLatin1String("Caption"),
                           testMethods,
                           testRealms,
                           testAcl);

    id = m_db->insertCredentials(info);

    /* no secrets DB: data will be cached in memory */
    QVariantMap cachedData;
    cachedData.insert(QLatin1String("James"), QLatin1String("Bond"));
    bool ret = m_db->storeData(id, method, cachedData);
    QVERIFY(ret);

    /* verify that the data is cached */
    QCOMPARE(cachedData, m_db->loadData(id, method));
    QCOMPARE(m_db->m_secretsCache->m_cache.count(), 1);

    /* load the secrets DB */
    ret = m_db->openSecretsDB(secretsDbFile);
    QVERIFY(ret);

    /* the cached data should have been stored into the DB and the
     * cache should be clear */
    QCOMPARE(cachedData, m_db->loadData(id, method));
    QVERIFY(m_db->m_secretsCache->m_cache.isEmpty());

    /* now store more data, with the secrets DB active */
    QVariantMap result;
    QVariantMap data;
    data.insert(QLatin1String("token"), QLatin1String("tokenval"));
    ret = m_db->storeData(id, method, data);
    QVERIFY(ret);
    result = m_db->loadData(id, method);
    qDebug() << result;
    QCOMPARE(result, data);

    data.insert(QLatin1String("token"), QLatin1String("tokenvalupdated"));
    data.insert(QLatin1String("token2"), QLatin1String("tokenval2"));
    ret = m_db->storeData(id, method, data);
    QVERIFY(ret);
    result = m_db->loadData(id, method);
    QCOMPARE(result, data);


    data.insert(QLatin1String("token"), QVariant());
    data.insert(QLatin1String("token2"), QVariant());
    ret = m_db->storeData(id, method, data);
    QVERIFY(ret);
    result = m_db->loadData(id, method);
    qDebug() << data;
    QVERIFY(result.isEmpty());


    ret = m_db->storeData(id, method, QVariantMap());
    QVERIFY(ret);
    result = m_db->loadData(id, method);
    qDebug() << data;
    QVERIFY(result.isEmpty());


    data.clear();
    for ( int i = 1000; i <1000+(SSO_MAX_TOKEN_STORAGE/10) +1 ; i++) {
        data.insert(QString::fromLatin1("t%1").arg(i), QLatin1String("12345"));
    }
    ret = m_db->storeData(id, method, data);
    QVERIFY(!ret);
    result = m_db->loadData(id, method);
    QVERIFY(result != data);


    data.clear();
    QVariantMap map;
    map.insert("key1",QLatin1String("string"));
    map.insert("key2",qint32(12));
    data.insert("key", map);
    ret = m_db->storeData(0, method, data);
    QVERIFY(!ret);
    result = m_db->loadData(0, method);
    QVERIFY(result.isEmpty());

    data.insert(QLatin1String("token"), QLatin1String("tokenvalupdated"));
    data.insert(QLatin1String("token2"), QLatin1String("tokenval2"));
    ret = m_db->storeData(id, method, data);
    QVERIFY(ret);
    ret = m_db->removeData(id, method);
    QVERIFY(ret);

    result = m_db->loadData(id, method);
    QVERIFY(result.isEmpty());

}


void TestDatabase::referenceTest()
{
    quint32 id;
    //insert complete
    SignonIdentityInfo info =
        SignonIdentityInfo(0,
                           QLatin1String("User"),
                           QLatin1String("Pass"), true,
                           QLatin1String("Caption"),
                           testMethods,
                           testRealms,
                           testAcl);

    id = m_db->insertCredentials(info);

    //add reference
    bool ret = m_db->addReference(id, QLatin1String("AID::12345678"),
                                  QLatin1String("ref1"));
    QVERIFY(ret);
    QStringList refs = m_db->references(id);
    qDebug() << refs;
    QVERIFY(refs.contains(QLatin1String("ref1")));
    refs = m_db->references(id,QLatin1String("AID::12345678"));
    QVERIFY(refs.contains(QLatin1String("ref1")));
    refs = m_db->references(id,QLatin1String("AID::1234567"));
    QVERIFY(!refs.contains(QLatin1String("ref1")));

    //remove references
    ret = m_db->removeReference(id, QLatin1String("AID::12345678"));
    QVERIFY(ret);
    refs = m_db->references(id);
    qDebug() << refs;
    QVERIFY(!refs.contains(QLatin1String("ref1")));

    //add new and remove nonexisting reference
    ret = m_db->addReference(id, QLatin1String("AID::12345678"),
                             QLatin1String("ref1"));
    QVERIFY(ret);
    ret = m_db->removeReference(id, QLatin1String("AID::12345678"),
                                QLatin1String("ref2"));
    QVERIFY(!ret);

    refs = m_db->references(id);
    qDebug() << refs;
    QVERIFY(refs.contains(QLatin1String("ref1")));

    //remove specific reference
    ret = m_db->removeReference(id, QLatin1String("AID::12345678"),
                                QLatin1String("ref1"));
    QVERIFY(ret);

    refs = m_db->references(id);
    qDebug() << refs;
    QVERIFY(!refs.contains(QLatin1String("ref1")));

}

void TestDatabase::cacheTest()
{
    quint32 idWithStore, idWithoutStore;

    SignonIdentityInfo info =
        SignonIdentityInfo(0,
                           QLatin1String("User"),
                           QLatin1String("Pass"), true,
                           QLatin1String("Caption"),
                           testMethods,
                           testRealms,
                           testAcl);

    /* no secrets DB: data will be cached in memory */

    idWithStore = m_db->insertCredentials(info);
    QVERIFY(idWithStore != 0);

    info.setPassword("Pass2");
    info.setStorePassword(false);
    idWithoutStore = m_db->insertCredentials(info);
    QVERIFY(idWithoutStore != 0);

    /* verify that the password is cached */
    info = m_db->credentials(idWithStore, true);
    QCOMPARE(info.password(), QLatin1String("Pass"));
    info = m_db->credentials(idWithoutStore, true);
    QCOMPARE(info.password(), QLatin1String("Pass2"));

    /* load the secrets DB */
    int ret = m_db->openSecretsDB(secretsDbFile);
    QVERIFY(ret);

    /* the cached data should have been stored into the DB, but not for
     * idWithoutStore, which has storeSecret set to false.
     */
    QString username, password;
    bool ok;
    ok = m_db->secretsStorage->loadCredentials(idWithStore,
                                               username, password);
    QVERIFY(ok);
    QCOMPARE(password, QLatin1String("Pass"));

    ok = m_db->secretsStorage->loadCredentials(idWithoutStore,
                                               username, password);
    QVERIFY(!ok);
}

void TestDatabase::accessControlListTest()
{
    quint32 id;

    //insert complete
    SignonIdentityInfo info =
        SignonIdentityInfo(0,
                           QLatin1String("User"),
                           QLatin1String("Pass"), true,
                           QLatin1String("Caption"),
                           testMethods,
                           testRealms,
                           testAcl);

    id = m_db->insertCredentials(info);

    QStringList acl = m_db->accessControlList(id);
    qDebug() << acl;
    QVERIFY(acl == info.accessControlList());
}

void TestDatabase::credentialsOwnerSecurityTokenTest()
{
    quint32 id;

    //insert complete
    SignonIdentityInfo info =
        SignonIdentityInfo(0,
                           QLatin1String("User"),
                           QLatin1String("Pass"), true,
                           QLatin1String("Caption"),
                           testMethods,
                           testRealms,
                           testAcl,
                           testAcl);

    id = m_db->insertCredentials(info);

    QString token = m_db->credentialsOwnerSecurityToken(id);
    qDebug() << token;
    QVERIFY(token == QLatin1String("AID::12345678"));
    QStringList tokens = m_db->ownerList(id);
    qDebug() << tokens;
    QVERIFY(tokens == testAcl);

}

#if !defined(SSO_CI_TESTMANAGEMENT)
    QTEST_MAIN(TestDatabase)
#endif
