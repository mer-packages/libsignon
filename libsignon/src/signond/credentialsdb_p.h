/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2012 Canonical Ltd.
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

#ifndef CREDENTIALS_DB_P_H
#define CREDENTIALS_DB_P_H

#include <QObject>
#include <QtSql>

#include "SignOn/abstract-secrets-storage.h"
#include "signonidentityinfo.h"

#define SSO_METADATADB_VERSION 2
#define SSO_SECRETSDB_VERSION 1

class TestDatabase;

namespace SignonDaemonNS {

/*!
 * @class SecretsCache
 * Caches credentials or BLOB authentication data.
 */
class SecretsCache
{
    friend class ::TestDatabase;
public:
    class AuthCache
    {
        friend class SecretsCache;

    private:
        QString m_username;
        QString m_password;
        bool m_storePassword;
        QHash<quint32,QVariantMap> m_blobData;
    };

    SecretsCache() {};
    ~SecretsCache() {};

    bool lookupCredentials(quint32 id,
                           QString &username,
                           QString &password) const;
    QVariantMap lookupData(quint32 id, quint32 method) const;

    void updateCredentials(quint32 id,
                           const QString &username,
                           const QString &password,
                           bool storePassword);
    void updateData(quint32 id, quint32 method, const QVariantMap &data);

    void storeToDB(SignOn::AbstractSecretsStorage *secretsStorage) const;
    void clear();

private:
    QHash<quint32, AuthCache> m_cache;
};

/*!
 * @class SqlDatabase
 * Will be used manage the SQL database interaction.
 * @ingroup Accounts_and_SSO_Framework
 */
class SqlDatabase
{
    friend class ::TestDatabase;
public:
    /*!
     * Constructs a SqlDatabase object using the given hostname.
     * @param hostname
     */
    SqlDatabase(const QString &hostname, const QString &connectionName,
                int version);

    /*!
     * Destroys the SqlDatabase object, closing the database connection.
     */
    virtual ~SqlDatabase();

    /*!
     * Connects to the DB and if necessary creates the tables
     */
    bool init();

    virtual bool createTables() = 0;
    virtual bool clear() = 0;
    virtual bool updateDB(int version);

    /*!
     * Creates the database connection.
     * @returns true if successful, false otherwise.
     */
    bool connect();
    /*!
     * Destroys the database connection.
     */
    void disconnect();

    bool startTransaction();
    bool commit();
    void rollback();

    /*!
     * @returns true if database connection is opened, false otherwise.
     */
    bool connected() { return m_database.isOpen(); }

    /*!
     * Sets the database name.
     * @param databseName
     */
    void setDatabaseName(const QString &databaseName) {
        m_database.setDatabaseName(databaseName);
    }

    /*!
     * Sets the username for the database connection.
     * @param username
     */
    void setUsername(const QString &username) {
        m_database.setUserName(username);
    }

    /*!
     * Sets the password for the database connection.
     * @param password
     */
    void setPassword(const QString &password) {
        m_database.setPassword(password);
    }

    /*!
     * @returns the database name.
     */
    QString databaseName() const { return m_database.databaseName(); }

    /*!
     * @returns the username for the database connection.
     */
    QString username() const { return m_database.userName(); }

    /*!
     * @returns the password for the database connection.
     */
    QString password() const { return m_database.password(); }

    QSqlQuery newQuery() const { return QSqlQuery(m_database); }

    /*!
     * Executes a specific database query.
     * If an error occurres the lastError() method can be used for handling
     * decissions.
     * @param query, the query string.
     * @returns the resulting sql query, which can be process in the case of a
     * 'SELECT' statement.
     */
    QSqlQuery exec(const QString &query);

    /*!
     * Executes a specific database query.
     * If an error occurres the lastError() method can be used for handling
     * decissions.
     * @param query, the query.
     * @returns the resulting sql query, which can be process in the case of a
     * 'SELECT' statement.
     */
    QSqlQuery exec(QSqlQuery &query);

    /*!
     * Executes a specific database set of queryes (INSERTs, UPDATEs, DELETEs)
     * in a transaction context (No nested transactions supported - sqlite
     * reasons).
     * If an error occurres the lastError() method can be used for handling
     * decissions.
     * @param queryList, the query list to be executed.
     * @returns true if the transaction commits successfully, false otherwise.
     */
    bool transactionalExec(const QStringList &queryList);

    /*!
     * @returns true, if the database has any tables created, false otherwise.
     */
    bool hasTables() const {
        return m_database.tables().count() > 0 ? true : false;
    }

    /*!
     * @returns a list of the supported drivers on the specific OS.
     */
    static QStringList supportedDrivers() { return QSqlDatabase::drivers(); }

    /*!
     * @returns the last occurred error if any. If not error occurred on the
     * last performed operation the error object is invalid.
     */
    SignOn::CredentialsDBError lastError() const;
    bool errorOccurred() const { return lastError().isValid(); };
    void clearError() { m_lastError.clear(); }

    /*!
     * Serializes a SQL error into a string.
     * @param error, method will fail if an error object is passed.
     * @returns the error information as string.
     */
    static QString errorInfo(const QSqlError &error);

    QString connectionName() const { return m_database.connectionName(); }

protected:
    QStringList queryList(const QString &query_str);
    QStringList queryList(QSqlQuery &query);
    void setLastError(const QSqlError &sqlError);

private:
    SignOn::CredentialsDBError m_lastError;

protected:
    int m_version;
    QSqlDatabase m_database;

    friend class CredentialsDB;
};

class MetaDataDB: public SqlDatabase
{
    friend class ::TestDatabase;
public:
    MetaDataDB(const QString &name):
        SqlDatabase(name, QLatin1String("SSO-metadata"),
                    SSO_METADATADB_VERSION) {}

    bool createTables();
    bool updateDB(int version);

    QStringList methods(const quint32 id,
                        const QString &securityToken = QString());
    quint32 insertMethod(const QString &method, bool *ok = 0);
    quint32 methodId(const QString &method);
    SignonIdentityInfo identity(const quint32 id);
    QList<SignonIdentityInfo> identities(const QMap<QString, QString> &filter);

    quint32 updateIdentity(const SignonIdentityInfo &info);
    bool removeIdentity(const quint32 id);

    bool clear();

    QStringList accessControlList(const quint32 identityId);
    QStringList ownerList(const quint32 identityId);

    bool addReference(const quint32 id,
                      const QString &token,
                      const QString &reference);
    bool removeReference(const quint32 id,
                         const QString &token,
                         const QString &reference = QString());
    QStringList references(const quint32 id, const QString &token = QString());

private:
    bool insertMethods(QMap<QString, QStringList> methods);
    quint32 updateCredentials(const SignonIdentityInfo &info);
    bool updateRealms(quint32 id, const QStringList &realms, bool isNew);
    QStringList tableUpdates2();
};

} // namespace SignonDaemonNS

#endif // CREDENTIALSDB_P_H
