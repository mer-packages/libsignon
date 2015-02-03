/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2012 Canonical Ltd.
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

#include "credentialsdb.h"
#include "credentialsdb_p.h"
#include "signond-common.h"
#include "signonidentityinfo.h"
#include "signonsessioncoretools.h"

#define INIT_ERROR() ErrorMonitor errorMonitor(this)
#define RETURN_IF_NO_SECRETS_DB(retval) \
    if (!isSecretsDBOpen()) { \
        TRACE() << "Secrets DB is not available"; \
        _lastError = noSecretsDB; return retval; \
    }

#define S(s) QLatin1String(s)

namespace SignonDaemonNS {

static const QString driver = QLatin1String("QSQLITE");

bool SecretsCache::lookupCredentials(quint32 id,
                                     QString &username,
                                     QString &password) const
{
    QHash<quint32, AuthCache>::const_iterator i;

    i = m_cache.find(id);
    if (i == m_cache.end()) return false;

    username = i->m_username;
    password = i->m_password;
    return true;
}

QVariantMap SecretsCache::lookupData(quint32 id, quint32 method) const
{
    return m_cache.value(id).m_blobData.value(method);
}

void SecretsCache::updateCredentials(quint32 id,
                                     const QString &username,
                                     const QString &password,
                                     bool storePassword)
{
    if (id == 0) return;

    AuthCache &credentials = m_cache[id];
    credentials.m_username = username;
    credentials.m_password = password;
    credentials.m_storePassword = storePassword;
}

void SecretsCache::updateData(quint32 id, quint32 method,
                              const QVariantMap &data)
{
    if (id == 0) return;

    AuthCache &credentials = m_cache[id];
    credentials.m_blobData[method] = data;
}

void
SecretsCache::storeToDB(SignOn::AbstractSecretsStorage *secretsStorage) const
{
    if (m_cache.isEmpty()) return;

    TRACE() << "Storing cached credentials into permanent storage";

    QHash<quint32, AuthCache>::const_iterator i;
    for (i = m_cache.constBegin();
         i != m_cache.constEnd();
         i++) {
        quint32 id = i.key();
        const AuthCache &cache = i.value();

        /* Store the credentials */
        QString password = cache.m_storePassword ?
            cache.m_password : QString();
        if (!cache.m_username.isEmpty() || !password.isEmpty()) {
            secretsStorage->updateCredentials(id,
                                              cache.m_username,
                                              password);
        }

        /* Store any binary blobs */
        QHash<quint32, QVariantMap>::const_iterator j;
        for (j = cache.m_blobData.constBegin();
             j != cache.m_blobData.constEnd();
             j++) {
            quint32 method = j.key();
            secretsStorage->storeData(id, method, j.value());
        }
    }
}

void SecretsCache::clear()
{
    m_cache.clear();
}

SqlDatabase::SqlDatabase(const QString &databaseName,
                         const QString &connectionName,
                         int version):
    m_lastError(SignOn::CredentialsDBError()),
    m_version(version),
    m_database(QSqlDatabase::addDatabase(driver, connectionName))

{
    TRACE() << "Supported Drivers:" << this->supportedDrivers();
    TRACE() << "DATABASE NAME [" << databaseName << "]";

    m_database.setDatabaseName(databaseName);
}

SqlDatabase::~SqlDatabase()
{
    m_database.commit();
    m_database.close();
}

bool SqlDatabase::init()
{
    if (!connect())
        return false;

    TRACE() <<  "Database connection succeeded.";

    if (!hasTables()) {
        TRACE() << "Creating SQL table structure...";
        if (!createTables())
            return false;

        if (!SqlDatabase::updateDB(m_version))
            BLAME() << "Failed to set database version to: " << m_version
                    << ".This could lead to data loss.";
    } else {
        TRACE() << "SQL table structure already created...";
        // check the DB version
        QSqlQuery q = exec(S("PRAGMA user_version"));
        int oldVersion = q.first() ? q.value(0).toInt() : 0;
        if (oldVersion < m_version)
            updateDB(oldVersion);
    }

    return true;
}

bool SqlDatabase::updateDB(int version)
{
    TRACE() << "Update DB from version " << version << " to " << m_version;
    exec(QString::fromLatin1("PRAGMA user_version = %1").arg(m_version));
    return true;
}

bool SqlDatabase::connect()
{
    if (!m_database.open()) {
        TRACE() << "Could not open database connection.\n";
        setLastError(m_database.lastError());
        return false;
    }
    return true;
}

void SqlDatabase::disconnect()
{
    m_database.close();
}

bool SqlDatabase::startTransaction()
{
    return m_database.transaction();
}

bool SqlDatabase::commit()
{
    return m_database.commit();
}

void SqlDatabase::rollback()
{
    if (!m_database.rollback())
        TRACE() << "Rollback failed, db data integrity could be compromised.";
}

QSqlQuery SqlDatabase::exec(const QString &queryStr)
{
    QSqlQuery query(QString(), m_database);

    if (!query.prepare(queryStr))
        TRACE() << "Query prepare warning: " << query.lastQuery();

    if (!query.exec()) {
        TRACE() << "Query exec error: " << query.lastQuery();
        setLastError(query.lastError());
        TRACE() << errorInfo(query.lastError());
    } else
        m_lastError.clear();

    return query;
}

QSqlQuery SqlDatabase::exec(QSqlQuery &query)
{

    if (!query.exec()) {
        TRACE() << "Query exec error: " << query.lastQuery();
        setLastError(query.lastError());
        TRACE() << errorInfo(query.lastError());
    } else
        m_lastError.clear();

    return query;
}


bool SqlDatabase::transactionalExec(const QStringList &queryList)
{
    if (!startTransaction()) {
        setLastError(m_database.lastError());
        TRACE() << "Could not start transaction";
        return false;
    }

    bool allOk = true;
    foreach (QString queryStr, queryList) {
        TRACE() << QString::fromLatin1("TRANSACT Query [%1]").arg(queryStr);
        QSqlQuery query = exec(queryStr);

        if (errorOccurred()) {
            allOk = false;
            break;
        }
    }

    if (allOk && commit()) {
        TRACE() << "Commit SUCCEEDED.";
        return true;
    } else {
        rollback();
    }

    TRACE() << "Transactional exec FAILED!";
    return false;
}

SignOn::CredentialsDBError SqlDatabase::lastError() const
{
    return m_lastError;
}

void SqlDatabase::setLastError(const QSqlError &sqlError)
{
    if (sqlError.isValid()) {
        if (sqlError.type() == QSqlError::ConnectionError) {
            m_lastError.setType(SignOn::CredentialsDBError::ConnectionError);
        } else {
            m_lastError.setType(SignOn::CredentialsDBError::StatementError);
        }
        m_lastError.setText(sqlError.text());
    } else {
        m_lastError.clear();
    }
}

QString SqlDatabase::errorInfo(const QSqlError &error)
{
    if (!error.isValid())
        return QLatin1String("SQL Error invalid.");

    QString text;
    QTextStream stream(&text);
    stream << "SQL error description:";
    stream << "\n\tType: ";

    const char *errType;
    switch (error.type()) {
        case QSqlError::NoError: errType = "NoError"; break;
        case QSqlError::ConnectionError: errType = "ConnectionError"; break;
        case QSqlError::StatementError: errType = "StatementError"; break;
        case QSqlError::TransactionError: errType = "TransactionError"; break;
        case QSqlError::UnknownError:
            /* fall trough */
        default: errType = "UnknownError";
    }
    stream << errType;
    stream << "\n\tDatabase text: " << error.databaseText();
    stream << "\n\tDriver text: " << error.driverText();
    stream << "\n\tNumber: " << error.number();

    return text;
}

QStringList SqlDatabase::queryList(const QString &query_str)
{
    QSqlQuery query(QString(), m_database);
    if (!query.prepare(query_str))
        TRACE() << "Query prepare warning: " << query.lastQuery();
    return queryList(query);
}

QStringList SqlDatabase::queryList(QSqlQuery &q)
{
    QStringList list;
    QSqlQuery query = exec(q);
    if (errorOccurred()) return list;
    while (query.next()) {
        list.append(query.value(0).toString());
    }
    query.clear();
    return list;
}

QStringList MetaDataDB::tableUpdates2()
{
    QStringList tableUpdates = QStringList()
        <<  QString::fromLatin1(
            "CREATE TABLE OWNER"
            "(rowid INTEGER PRIMARY KEY AUTOINCREMENT,"
            "identity_id INTEGER CONSTRAINT fk_identity_id REFERENCES CREDENTIALS(id) ON DELETE CASCADE,"
            "token_id INTEGER CONSTRAINT fk_token_id REFERENCES TOKENS(id) ON DELETE CASCADE)")
        //added triggers for OWNER
        << QString::fromLatin1(
            // Foreign Key Preventing insert
            "CREATE TRIGGER fki_OWNER_token_id_TOKENS_id "
            "BEFORE INSERT ON [OWNER] "
            "FOR EACH ROW BEGIN "
            "  SELECT RAISE(ROLLBACK, 'insert on table OWNER violates foreign key constraint fki_OWNER_token_id_TOKENS_id') "
            "  WHERE NEW.token_id IS NOT NULL AND (SELECT id FROM TOKENS WHERE id = NEW.token_id) IS NULL; "
            "END; "
        )
        << QString::fromLatin1(
            // Foreign key preventing update
            "CREATE TRIGGER fku_OWNER_token_id_TOKENS_id "
            "BEFORE UPDATE ON [OWNER] "
            "FOR EACH ROW BEGIN "
            "    SELECT RAISE(ROLLBACK, 'update on table OWNER violates foreign key constraint fku_OWNER_token_id_TOKENS_id') "
            "      WHERE NEW.token_id IS NOT NULL AND (SELECT id FROM TOKENS WHERE id = NEW.token_id) IS NULL; "
            "END; "
        )
        << QString::fromLatin1(
            // Cascading Delete
            "CREATE TRIGGER fkdc_OWNER_token_id_TOKENS_id "
            "BEFORE DELETE ON TOKENS "
            "FOR EACH ROW BEGIN "
            "    DELETE FROM OWNER WHERE OWNER.token_id = OLD.id; "
            "END; "
        );

    return tableUpdates;
}

bool MetaDataDB::createTables()
{
    /* !!! Foreign keys support seems to be disabled, for the moment... */
    QStringList createTableQuery = QStringList()
        <<  QString::fromLatin1(
            "CREATE TABLE CREDENTIALS"
            "(id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "caption TEXT,"
            "username TEXT,"
            "flags INTEGER,"
            "type INTEGER)")
        <<  QString::fromLatin1(
            "CREATE TABLE METHODS"
            "(id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "method TEXT UNIQUE)")
        <<  QString::fromLatin1(
            "CREATE TABLE MECHANISMS"
            "(id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "mechanism TEXT UNIQUE)")
        <<  QString::fromLatin1(
            "CREATE TABLE TOKENS"
            "(id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "token TEXT UNIQUE)")
        <<  QString::fromLatin1(
            "CREATE TABLE REALMS"
            "(identity_id INTEGER CONSTRAINT fk_identity_id REFERENCES CREDENTIALS(id) ON DELETE CASCADE,"
            "realm TEXT,"
            "hostname TEXT,"
            "PRIMARY KEY (identity_id, realm, hostname))")
        <<  QString::fromLatin1(
            "CREATE TABLE ACL"
            "(rowid INTEGER PRIMARY KEY AUTOINCREMENT,"
            "identity_id INTEGER CONSTRAINT fk_identity_id REFERENCES CREDENTIALS(id) ON DELETE CASCADE,"
            "method_id INTEGER CONSTRAINT fk_method_id REFERENCES METHODS(id) ON DELETE CASCADE,"
            "mechanism_id INTEGER CONSTRAINT fk_mechanism_id REFERENCES MECHANISMS(id) ON DELETE CASCADE,"
            "token_id INTEGER CONSTRAINT fk_token_id REFERENCES TOKENS(id) ON DELETE CASCADE)")
        <<  QString::fromLatin1(
            "CREATE TABLE REFS"
            "(identity_id INTEGER CONSTRAINT fk_identity_id REFERENCES CREDENTIALS(id) ON DELETE CASCADE,"
            "token_id INTEGER CONSTRAINT fk_token_id REFERENCES TOKENS(id) ON DELETE CASCADE,"
            "ref TEXT,"
            "PRIMARY KEY (identity_id, token_id, ref))")

/*
* triggers generated with
* http://www.rcs-comp.com/site/index.php/view/Utilities-SQLite_foreign_key_trigger_generator
*/
        //insert triggers to force foreign keys support
        << QString::fromLatin1(
            // Foreign Key Preventing insert
            "CREATE TRIGGER fki_REALMS_identity_id_CREDENTIALS_id "
            "BEFORE INSERT ON [REALMS] "
            "FOR EACH ROW BEGIN "
            "  SELECT RAISE(ROLLBACK, 'insert on table REALMS violates foreign key constraint fki_REALMS_identity_id_CREDENTIALS_id') "
            "  WHERE NEW.identity_id IS NOT NULL AND (SELECT id FROM CREDENTIALS WHERE id = NEW.identity_id) IS NULL; "
            "END; "
        )
        << QString::fromLatin1(
            // Foreign key preventing update
            "CREATE TRIGGER fku_REALMS_identity_id_CREDENTIALS_id "
            "BEFORE UPDATE ON [REALMS] "
            "FOR EACH ROW BEGIN "
            "    SELECT RAISE(ROLLBACK, 'update on table REALMS violates foreign key constraint fku_REALMS_identity_id_CREDENTIALS_id') "
            "      WHERE NEW.identity_id IS NOT NULL AND (SELECT id FROM CREDENTIALS WHERE id = NEW.identity_id) IS NULL; "
            "END; "
        )
        << QString::fromLatin1(
            // Cascading Delete
            "CREATE TRIGGER fkdc_REALMS_identity_id_CREDENTIALS_id "
            "BEFORE DELETE ON CREDENTIALS "
            "FOR EACH ROW BEGIN "
            "    DELETE FROM REALMS WHERE REALMS.identity_id = OLD.id; "
            "END; "
        )
        << QString::fromLatin1(
            // Foreign Key Preventing insert
            "CREATE TRIGGER fki_ACL_identity_id_CREDENTIALS_id "
            "BEFORE INSERT ON [ACL] "
            "FOR EACH ROW BEGIN "
            "  SELECT RAISE(ROLLBACK, 'insert on table ACL violates foreign key constraint fki_ACL_identity_id_CREDENTIALS_id') "
            "  WHERE NEW.identity_id IS NOT NULL AND (SELECT id FROM CREDENTIALS WHERE id = NEW.identity_id) IS NULL; "
            "END;"
        )
        << QString::fromLatin1(
            // Foreign key preventing update
            "CREATE TRIGGER fku_ACL_identity_id_CREDENTIALS_id "
            "BEFORE UPDATE ON [ACL] "
            "FOR EACH ROW BEGIN "
            "    SELECT RAISE(ROLLBACK, 'update on table ACL violates foreign key constraint fku_ACL_identity_id_CREDENTIALS_id') "
            "      WHERE NEW.identity_id IS NOT NULL AND (SELECT id FROM CREDENTIALS WHERE id = NEW.identity_id) IS NULL; "
            "END; "
        )
        << QString::fromLatin1(
            // Cascading Delete
            "CREATE TRIGGER fkdc_ACL_identity_id_CREDENTIALS_id "
            "BEFORE DELETE ON CREDENTIALS "
            "FOR EACH ROW BEGIN "
             "   DELETE FROM ACL WHERE ACL.identity_id = OLD.id; "
            "END; "
        )
        << QString::fromLatin1(
            // Foreign Key Preventing insert
            "CREATE TRIGGER fki_ACL_method_id_METHODS_id "
            "BEFORE INSERT ON [ACL] "
            "FOR EACH ROW BEGIN "
            "  SELECT RAISE(ROLLBACK, 'insert on table ACL violates foreign key constraint fki_ACL_method_id_METHODS_id') "
            "  WHERE NEW.method_id IS NOT NULL AND (SELECT id FROM METHODS WHERE id = NEW.method_id) IS NULL; "
            "END; "
        )
        << QString::fromLatin1(
            // Foreign key preventing update
            "CREATE TRIGGER fku_ACL_method_id_METHODS_id "
            "BEFORE UPDATE ON [ACL] "
            "FOR EACH ROW BEGIN "
            "    SELECT RAISE(ROLLBACK, 'update on table ACL violates foreign key constraint fku_ACL_method_id_METHODS_id') "
            "      WHERE NEW.method_id IS NOT NULL AND (SELECT id FROM METHODS WHERE id = NEW.method_id) IS NULL; "
            "END; "
        )
        << QString::fromLatin1(
            // Cascading Delete
            "CREATE TRIGGER fkdc_ACL_method_id_METHODS_id "
            "BEFORE DELETE ON METHODS "
            "FOR EACH ROW BEGIN "
            "    DELETE FROM ACL WHERE ACL.method_id = OLD.id; "
            "END; "
        )
        << QString::fromLatin1(
            // Foreign Key Preventing insert
            "CREATE TRIGGER fki_ACL_mechanism_id_MECHANISMS_id "
            "BEFORE INSERT ON [ACL] "
            "FOR EACH ROW BEGIN "
            "  SELECT RAISE(ROLLBACK, 'insert on table ACL violates foreign key constraint fki_ACL_mechanism_id_MECHANISMS_id') "
            "  WHERE NEW.mechanism_id IS NOT NULL AND (SELECT id FROM MECHANISMS WHERE id = NEW.mechanism_id) IS NULL; "
            "END; "
        )
        << QString::fromLatin1(
            // Foreign key preventing update
            "CREATE TRIGGER fku_ACL_mechanism_id_MECHANISMS_id "
            "BEFORE UPDATE ON [ACL] "
            "FOR EACH ROW BEGIN "
            "    SELECT RAISE(ROLLBACK, 'update on table ACL violates foreign key constraint fku_ACL_mechanism_id_MECHANISMS_id') "
            "      WHERE NEW.mechanism_id IS NOT NULL AND (SELECT id FROM MECHANISMS WHERE id = NEW.mechanism_id) IS NULL; "
            "END; "
        )
        << QString::fromLatin1(
            // Cascading Delete
            "CREATE TRIGGER fkdc_ACL_mechanism_id_MECHANISMS_id "
            "BEFORE DELETE ON MECHANISMS "
            "FOR EACH ROW BEGIN "
            "    DELETE FROM ACL WHERE ACL.mechanism_id = OLD.id; "
            "END; "
        )
        << QString::fromLatin1(
            // Foreign Key Preventing insert
            "CREATE TRIGGER fki_ACL_token_id_TOKENS_id "
            "BEFORE INSERT ON [ACL] "
            "FOR EACH ROW BEGIN "
            "  SELECT RAISE(ROLLBACK, 'insert on table ACL violates foreign key constraint fki_ACL_token_id_TOKENS_id') "
            "  WHERE NEW.token_id IS NOT NULL AND (SELECT id FROM TOKENS WHERE id = NEW.token_id) IS NULL; "
            "END; "
        )
        << QString::fromLatin1(
            // Foreign key preventing update
            "CREATE TRIGGER fku_ACL_token_id_TOKENS_id "
            "BEFORE UPDATE ON [ACL] "
            "FOR EACH ROW BEGIN "
            "    SELECT RAISE(ROLLBACK, 'update on table ACL violates foreign key constraint fku_ACL_token_id_TOKENS_id') "
            "      WHERE NEW.token_id IS NOT NULL AND (SELECT id FROM TOKENS WHERE id = NEW.token_id) IS NULL; "
            "END; "
        )
        << QString::fromLatin1(
            // Cascading Delete
            "CREATE TRIGGER fkdc_ACL_token_id_TOKENS_id "
            "BEFORE DELETE ON TOKENS "
            "FOR EACH ROW BEGIN "
            "    DELETE FROM ACL WHERE ACL.token_id = OLD.id; "
            "END; "
        )
        << QString::fromLatin1(
            // Foreign Key Preventing insert
            "CREATE TRIGGER fki_REFS_identity_id_CREDENTIALS_id "
            "BEFORE INSERT ON [REFS] "
            "FOR EACH ROW BEGIN "
            "  SELECT RAISE(ROLLBACK, 'insert on table REFS violates foreign key constraint fki_REFS_identity_id_CREDENTIALS_id') "
            "  WHERE NEW.identity_id IS NOT NULL AND (SELECT id FROM CREDENTIALS WHERE id = NEW.identity_id) IS NULL; "
            "END; "
        )
        << QString::fromLatin1(
            // Foreign key preventing update
            "CREATE TRIGGER fku_REFS_identity_id_CREDENTIALS_id "
            "BEFORE UPDATE ON [REFS] "
            "FOR EACH ROW BEGIN "
            "    SELECT RAISE(ROLLBACK, 'update on table REFS violates foreign key constraint fku_REFS_identity_id_CREDENTIALS_id') "
            "      WHERE NEW.identity_id IS NOT NULL AND (SELECT id FROM CREDENTIALS WHERE id = NEW.identity_id) IS NULL; "
            "END; "
        )
        << QString::fromLatin1(
            // Cascading Delete
            "CREATE TRIGGER fkdc_REFS_identity_id_CREDENTIALS_id "
            "BEFORE DELETE ON CREDENTIALS "
            "FOR EACH ROW BEGIN "
            "    DELETE FROM REFS WHERE REFS.identity_id = OLD.id; "
            "END; "
        )
        << QString::fromLatin1(
            // Foreign Key Preventing insert
            "CREATE TRIGGER fki_REFS_token_id_TOKENS_id "
            "BEFORE INSERT ON [REFS] "
            "FOR EACH ROW BEGIN "
            "  SELECT RAISE(ROLLBACK, 'insert on table REFS violates foreign key constraint fki_REFS_token_id_TOKENS_id') "
            "  WHERE NEW.token_id IS NOT NULL AND (SELECT id FROM TOKENS WHERE id = NEW.token_id) IS NULL; "
            "END; "
        )
        << QString::fromLatin1(
            // Foreign key preventing update
            "CREATE TRIGGER fku_REFS_token_id_TOKENS_id "
            "BEFORE UPDATE ON [REFS] "
            "FOR EACH ROW BEGIN "
            "    SELECT RAISE(ROLLBACK, 'update on table REFS violates foreign key constraint fku_REFS_token_id_TOKENS_id') "
            "      WHERE NEW.token_id IS NOT NULL AND (SELECT id FROM TOKENS WHERE id = NEW.token_id) IS NULL; "
            "END; "
        )
        << QString::fromLatin1(
            // Cascading Delete
            "CREATE TRIGGER fkdc_REFS_token_id_TOKENS_id "
            "BEFORE DELETE ON TOKENS "
            "FOR EACH ROW BEGIN "
            "    DELETE FROM REFS WHERE REFS.token_id = OLD.id; "
            "END; "
        );
/*
end of generated code
*/
    //insert table updates
    createTableQuery << tableUpdates2();

    foreach (QString createTable, createTableQuery) {
        QSqlQuery query = exec(createTable);
        if (lastError().isValid()) {
            TRACE() << "Error occurred while creating the database.";
            return false;
        }
        query.clear();
        commit();
    }
    TRACE() << "Creation successful";

    return true;
}

bool MetaDataDB::updateDB(int version)
{
    if (version == m_version)
        return true;

    if (version < 1) {
        TRACE() << "Upgrading from version < 1 not supported. Clearing DB";
        QString fileName = m_database.databaseName();
        QString connectionName = m_database.connectionName();
        m_database.close();
        QFile::remove(fileName);
        m_database = QSqlDatabase(QSqlDatabase::addDatabase(driver,
                                                            connectionName));
        m_database.setDatabaseName(fileName);
        if (!connect())
            return false;

        if (!createTables())
            return false;
    }

    //convert from 1 to 2
    if (version == 1) {
        QStringList createTableQuery = tableUpdates2();
        foreach (QString createTable, createTableQuery) {
            QSqlQuery query = exec(createTable);
            if (lastError().isValid()) {
                TRACE() << "Error occurred while inseting new tables.";
                return false;
            }
            query.clear();
            commit();
        }
        TRACE() << "Table insert successful";

        //populate owner table from acl
        QSqlQuery ownerInsert = exec(S("INSERT OR IGNORE INTO OWNER "
                            "(identity_id, token_id) "
                            " SELECT identity_id, token_id FROM ACL"));
        if (!commit()){
            BLAME() << "Table copy failed.";
            rollback();
        }

    } else {
        return false;
    }

    return SqlDatabase::updateDB(version);
}

QStringList MetaDataDB::methods(const quint32 id, const QString &securityToken)
{
    QStringList list;
    if (securityToken.isEmpty()) {
        list = queryList(
                 QString::fromLatin1("SELECT DISTINCT METHODS.method FROM "
                        "( ACL JOIN METHODS ON ACL.method_id = METHODS.id ) "
                        "WHERE ACL.identity_id = '%1'").arg(id)
                 );
        return list;
    }
    QSqlQuery q = newQuery();
    q.prepare(S("SELECT DISTINCT METHODS.method FROM "
                "( ACL JOIN METHODS ON ACL.method_id = METHODS.id) "
                "WHERE ACL.identity_id = :id AND ACL.token_id = "
                "(SELECT id FROM TOKENS where token = :token)"));
    q.bindValue(S(":id"), id);
    q.bindValue(S(":token"), securityToken);
    list = queryList(q);

    return list;
}

quint32 MetaDataDB::methodId(const QString &method)
{
    TRACE() << "method:" << method;

    QSqlQuery q = newQuery();
    q.prepare(S("SELECT id FROM METHODS WHERE method = :method"));
    q.bindValue(S(":method"), method);
    exec(q);
    if (!q.first()) {
        TRACE() << "No result or invalid method query.";
        return 0;
    }

    return q.value(0).toUInt();
}

SignonIdentityInfo MetaDataDB::identity(const quint32 id)
{
    QString query_str;

    query_str = QString::fromLatin1(
            "SELECT caption, username, flags, type "
            "FROM credentials WHERE id = %1").arg(id);
    QSqlQuery query = exec(query_str);

    if (!query.first()) {
        TRACE() << "No result or invalid credentials query.";
        return SignonIdentityInfo();
    }

    QString caption = query.value(0).toString();
    QString username = query.value(1).toString();
    int flags = query.value(2).toInt();
    bool savePassword = flags & RememberPassword;
    bool validated =  flags & Validated;
    bool isUserNameSecret = flags & UserNameIsSecret;
    if (isUserNameSecret) username = QString();
    int type = query.value(3).toInt();

    query.clear();
    QStringList realms = queryList(
            QString::fromLatin1("SELECT realm FROM REALMS "
                    "WHERE identity_id = %1").arg(id));

    QStringList ownerTokens = queryList(
            QString::fromLatin1("SELECT token FROM TOKENS "
                                "WHERE id IN "
                                "(SELECT token_id FROM OWNER WHERE identity_id = '%1' )")
                                .arg(id));

    query_str = QString::fromLatin1("SELECT token FROM TOKENS "
            "WHERE id IN "
            "(SELECT token_id FROM ACL WHERE identity_id = '%1' )")
            .arg(id);
    query = exec(query_str);
    QStringList securityTokens;
    while (query.next()) {
        securityTokens.append(query.value(0).toString());
    }
    query.clear();
    MethodMap methods;
    query_str = QString::fromLatin1(
            "SELECT DISTINCT ACL.method_id, METHODS.method FROM "
            "( ACL JOIN METHODS ON ACL.method_id = METHODS.id ) "
            "WHERE ACL.identity_id = '%1'").arg(id);
    query = exec(query_str);
    while (query.next()) {
        QStringList mechanisms = queryList(
                QString::fromLatin1("SELECT DISTINCT MECHANISMS.mechanism FROM "
                        "( MECHANISMS JOIN ACL "
                        "ON ACL.mechanism_id = MECHANISMS.id ) "
                        "WHERE ACL.method_id = '%1' AND ACL.identity_id = '%2' ")
                        .arg(query.value(0).toInt()).arg(id));
            methods.insert(query.value(1).toString(), mechanisms);
    }
    query.clear();

    int refCount = 0;
    //TODO query for refcount

    SignonIdentityInfo info;
    info.setId(id);
    if (!isUserNameSecret)
        info.setUserName(username);
    info.setStorePassword(savePassword);
    info.setCaption(caption);
    info.setMethods(methods);
    info.setRealms(realms);
    info.setAccessControlList(securityTokens);
    info.setOwnerList(ownerTokens);
    info.setType(type);
    info.setRefCount(refCount);
    info.setValidated(validated);
    info.setUserNameSecret(isUserNameSecret);
    return info;
}

QList<SignonIdentityInfo> MetaDataDB::identities(const QMap<QString,
                                                 QString> &filter)
{
    TRACE();
    Q_UNUSED(filter)
    QList<SignonIdentityInfo> result;

    QString queryStr(QString::fromLatin1("SELECT id FROM credentials"));

    // TODO - process filtering step here !!!

    queryStr += QString::fromLatin1(" ORDER BY id");

    QSqlQuery query = exec(queryStr);
    if (errorOccurred()) {
        TRACE() << "Error occurred while fetching credentials from database.";
        return result;
    }

    while (query.next()) {
        SignonIdentityInfo info = identity(query.value(0).toUInt());
        if (errorOccurred())
            break;
        result << info;
    }

    query.clear();
    return result;
}

quint32 MetaDataDB::updateIdentity(const SignonIdentityInfo &info)
{
    if (!startTransaction()) {
        TRACE() << "Could not start transaction. Error inserting credentials.";
        return 0;
    }

    quint32 id = updateCredentials(info);
    if (id == 0) {
        rollback();
        return 0;
    }

    /* Methods inserts */
    insertMethods(info.methods());

    if (!updateRealms(id, info.realms(), info.isNew())) {
        TRACE() << "Error in updating realms";
        rollback();
        return 0;
    }

    /* Security tokens insert */
    foreach (QString token, info.accessControlList()) {
        QSqlQuery tokenInsert = newQuery();
        tokenInsert.prepare(S("INSERT OR IGNORE INTO TOKENS (token) "
                              "VALUES ( :token )"));
        tokenInsert.bindValue(S(":token"), token);
        exec(tokenInsert);
    }

    foreach (QString token, info.ownerList()) {
        if (!token.isEmpty()) {
            QSqlQuery tokenInsert = newQuery();
            tokenInsert.prepare(S("INSERT OR IGNORE INTO TOKENS (token) "
                                  "VALUES ( :token )"));
            tokenInsert.bindValue(S(":token"), token);
            exec(tokenInsert);
        }
    }

    if (!info.isNew()) {
        //remove acl
        QString queryStr = QString::fromLatin1(
                    "DELETE FROM ACL WHERE "
                    "identity_id = '%1'")
                    .arg(info.id());
        QSqlQuery insertQuery = exec(queryStr);
        insertQuery.clear();
        //remove owner
        queryStr = QString::fromLatin1(
                    "DELETE FROM OWNER WHERE "
                    "identity_id = '%1'")
                    .arg(info.id());
        insertQuery = exec(queryStr);
        insertQuery.clear();
    }

    /* ACL insert, this will do basically identity level ACL */
    QMapIterator<QString, QStringList> it(info.methods());
    while (it.hasNext()) {
        it.next();
        if (!info.accessControlList().isEmpty()) {
            foreach (QString token, info.accessControlList()) {
                foreach (QString mech, it.value()) {
                    QSqlQuery aclInsert = newQuery();
                    aclInsert.prepare(S("INSERT OR REPLACE INTO ACL "
                                        "(identity_id, method_id, mechanism_id, token_id) "
                                        "VALUES ( :id, "
                                        "( SELECT id FROM METHODS WHERE method = :method ),"
                                        "( SELECT id FROM MECHANISMS WHERE mechanism= :mech ), "
                                        "( SELECT id FROM TOKENS WHERE token = :token ))"));
                    aclInsert.bindValue(S(":id"), id);
                    aclInsert.bindValue(S(":method"), it.key());
                    aclInsert.bindValue(S(":mech"), mech);
                    aclInsert.bindValue(S(":token"), token);
                    exec(aclInsert);
                }
                //insert entires for empty mechs list
                if (it.value().isEmpty()) {
                    QSqlQuery aclInsert = newQuery();
                    aclInsert.prepare(S("INSERT OR REPLACE INTO ACL (identity_id, method_id, token_id) "
                                        "VALUES ( :id, "
                                        "( SELECT id FROM METHODS WHERE method = :method ),"
                                        "( SELECT id FROM TOKENS WHERE token = :token ))"));
                    aclInsert.bindValue(S(":id"), id);
                    aclInsert.bindValue(S(":method"), it.key());
                    aclInsert.bindValue(S(":token"), token);
                    exec(aclInsert);
                }
            }
        } else {
            foreach (QString mech, it.value()) {
                QSqlQuery aclInsert = newQuery();
                aclInsert.prepare(S("INSERT OR REPLACE INTO ACL "
                                    "(identity_id, method_id, mechanism_id) "
                                    "VALUES ( :id, "
                                    "( SELECT id FROM METHODS WHERE method = :method ),"
                                    "( SELECT id FROM MECHANISMS WHERE mechanism= :mech )"
                                    ")"));
                aclInsert.bindValue(S(":id"), id);
                aclInsert.bindValue(S(":method"), it.key());
                aclInsert.bindValue(S(":mech"), mech);
                exec(aclInsert);
            }
            //insert entires for empty mechs list
            if (it.value().isEmpty()) {
                QSqlQuery aclInsert = newQuery();
                aclInsert.prepare(S("INSERT OR REPLACE INTO ACL (identity_id, method_id) "
                                    "VALUES ( :id, "
                                    "( SELECT id FROM METHODS WHERE method = :method )"
                                    ")"));
                aclInsert.bindValue(S(":id"), id);
                aclInsert.bindValue(S(":method"), it.key());
                exec(aclInsert);
            }
        }
    }
    //insert acl in case where methods are missing
    if (info.methods().isEmpty()) {
        foreach (QString token, info.accessControlList()) {
            QSqlQuery aclInsert = newQuery();
            aclInsert.prepare(S("INSERT OR REPLACE INTO ACL "
                                "(identity_id, token_id) "
                                "VALUES ( :id, "
                                "( SELECT id FROM TOKENS WHERE token = :token ))"));
            aclInsert.bindValue(S(":id"), id);
            aclInsert.bindValue(S(":token"), token);
            exec(aclInsert);
        }
    }

    //insert owner list
    foreach (QString token, info.ownerList()) {
        if (!token.isEmpty()) {
            QSqlQuery ownerInsert = newQuery();
            ownerInsert.prepare(S("INSERT OR REPLACE INTO OWNER "
                            "(identity_id, token_id) "
                            "VALUES ( :id, "
                            "( SELECT id FROM TOKENS WHERE token = :token ))"));
            ownerInsert.bindValue(S(":id"), id);
            ownerInsert.bindValue(S(":token"), token);
            exec(ownerInsert);
        }
    }

    if (commit()) {
        return id;
    } else {
        rollback();
        TRACE() << "Credentials insertion failed.";
        return 0;
    }
}

bool MetaDataDB::removeIdentity(const quint32 id)
{
    TRACE();

    QStringList queries = QStringList()
        << QString::fromLatin1(
            "DELETE FROM CREDENTIALS WHERE id = %1").arg(id)
        << QString::fromLatin1(
            "DELETE FROM ACL WHERE identity_id = %1").arg(id)
        << QString::fromLatin1(
            "DELETE FROM REALMS WHERE identity_id = %1").arg(id)
        << QString::fromLatin1(
            "DELETE FROM owner WHERE identity_id = %1").arg(id);

    return transactionalExec(queries);
}

bool MetaDataDB::clear()
{
    TRACE();

    QStringList clearCommands = QStringList()
        << QLatin1String("DELETE FROM CREDENTIALS")
        << QLatin1String("DELETE FROM METHODS")
        << QLatin1String("DELETE FROM MECHANISMS")
        << QLatin1String("DELETE FROM ACL")
        << QLatin1String("DELETE FROM REALMS")
        << QLatin1String("DELETE FROM TOKENS")
        << QLatin1String("DELETE FROM OWNER");

    return transactionalExec(clearCommands);
}

QStringList MetaDataDB::accessControlList(const quint32 identityId)
{
    return queryList(QString::fromLatin1("SELECT token FROM TOKENS "
            "WHERE id IN "
            "(SELECT token_id FROM ACL WHERE identity_id = '%1' )")
            .arg(identityId));
}

QStringList MetaDataDB::ownerList(const quint32 identityId)
{
    return queryList(QString::fromLatin1("SELECT token FROM TOKENS "
            "WHERE id IN "
            "(SELECT token_id FROM OWNER WHERE identity_id = '%1' )")
            .arg(identityId));
}

bool MetaDataDB::addReference(const quint32 id,
                              const QString &token,
                              const QString &reference)
{
    if (!startTransaction()) {
        TRACE() << "Could not start transaction. Error inserting data.";
        return false;
    }

    TRACE() << "Storing:" << id << ", " << token << ", " << reference;
    /* Data insert */
    bool allOk = true;

    /* Security token insert */
    QSqlQuery tokenInsert = newQuery();
    tokenInsert.prepare(S("INSERT OR IGNORE INTO TOKENS (token) "
                          "VALUES ( :token )"));
    tokenInsert.bindValue(S(":token"), token);
    exec(tokenInsert);
    if (errorOccurred()) {
                allOk = false;
    }

    QSqlQuery refsInsert = newQuery();
    refsInsert.prepare(S("INSERT OR REPLACE INTO REFS "
                         "(identity_id, token_id, ref) "
                         "VALUES ( :id, "
                         "( SELECT id FROM TOKENS WHERE token = :token ),"
                         ":reference"
                         ")"));
    refsInsert.bindValue(S(":id"), id);
    refsInsert.bindValue(S(":token"), token);
    refsInsert.bindValue(S(":reference"), reference);
    exec(refsInsert);
    if (errorOccurred()) {
                allOk = false;
    }

    if (allOk && commit()) {
        TRACE() << "Data insertion ok.";
        return true;
    }
    rollback();
    TRACE() << "Data insertion failed.";
    return false;
}

bool MetaDataDB::removeReference(const quint32 id,
                                 const QString &token,
                                 const QString &reference)
{
    TRACE() << "Removing:" << id << ", " << token << ", " << reference;
    //check that there is references
    QStringList refs = references(id, token);
    if (refs.isEmpty())
        return false;
    if (!reference.isNull() && !refs.contains(reference))
        return false;

    if (!startTransaction()) {
        TRACE() << "Could not start transaction. Error removing data.";
        return false;
    }

    bool allOk = true;
    QSqlQuery refsDelete = newQuery();

    if (reference.isEmpty()) {
        refsDelete.prepare(S("DELETE FROM REFS "
                             "WHERE identity_id = :id AND "
                             "token_id = ( SELECT id FROM TOKENS WHERE token = :token )"));
        refsDelete.bindValue(S(":id"), id);
        refsDelete.bindValue(S(":token"), token);
    } else {
        refsDelete.prepare(S("DELETE FROM REFS "
                             "WHERE identity_id = :id AND "
                             "token_id = ( SELECT id FROM TOKENS WHERE token = :token ) "
                             "AND ref = :ref"));
        refsDelete.bindValue(S(":id"), id);
        refsDelete.bindValue(S(":token"), token);
        refsDelete.bindValue(S(":ref"), reference);
    }

    exec(refsDelete);
    if (errorOccurred()) {
                allOk = false;
    }

    if (allOk && commit()) {
        TRACE() << "Data delete ok.";
        return true;
    }
    rollback();
    TRACE() << "Data delete failed.";
    return false;
}

QStringList MetaDataDB::references(const quint32 id, const QString &token)
{
    if (token.isEmpty())
        return queryList(QString::fromLatin1("SELECT ref FROM REFS "
            "WHERE identity_id = '%1'")
            .arg(id));
    QSqlQuery q = newQuery();
    q.prepare(S("SELECT ref FROM REFS "
                "WHERE identity_id = :id AND "
                "token_id = (SELECT id FROM TOKENS WHERE token = :token )"));
    q.bindValue(S(":id"), id);
    q.bindValue(S(":token"), token);
    return queryList(q);
}

bool MetaDataDB::insertMethods(QMap<QString, QStringList> methods)
{
    bool allOk = true;

    if (methods.isEmpty()) return false;
    //insert (unique) method names
    QMapIterator<QString, QStringList> it(methods);
    while (it.hasNext()) {
        it.next();
        QSqlQuery methodInsert = newQuery();
        methodInsert.prepare(S("INSERT OR IGNORE INTO METHODS (method) "
                               "VALUES( :method )"));
        methodInsert.bindValue(S(":method"), it.key());
        exec(methodInsert);
        if (errorOccurred()) allOk = false;
        //insert (unique) mechanism names
        foreach (QString mech, it.value()) {
            QSqlQuery mechInsert = newQuery();
            mechInsert.prepare(S("INSERT OR IGNORE INTO MECHANISMS (mechanism) "
                                 "VALUES( :mech )"));
            mechInsert.bindValue(S(":mech"), mech);
            exec(mechInsert);
            if (errorOccurred()) allOk = false;
        }
    }
    return allOk;
}

quint32 MetaDataDB::insertMethod(const QString &method, bool *ok)
{
    QSqlQuery q = newQuery();
    q.prepare(S("INSERT INTO METHODS (method) VALUES(:method)"));
    q.bindValue(S(":method"), method);
    exec(q);

    if (errorOccurred()) {
        if (ok != 0) *ok = false;
        return 0;
    }
    return q.lastInsertId().toUInt(ok);
}

quint32 MetaDataDB::updateCredentials(const SignonIdentityInfo &info)
{
    quint32 id;
    QSqlQuery q = newQuery();

    int flags = 0;
    if (info.validated()) flags |= Validated;
    if (info.storePassword()) flags |= RememberPassword;
    if (info.isUserNameSecret()) flags |= UserNameIsSecret;

    if (!info.isNew()) {
        TRACE() << "UPDATE:" << info.id() ;
        q.prepare(S("UPDATE CREDENTIALS SET caption = :caption, "
                    "username = :username, "
                    "flags = :flags, "
                    "type = :type WHERE id = :id"));
        q.bindValue(S(":id"), info.id());
    } else {
        TRACE() << "INSERT:" << info.id();
        q.prepare(S("INSERT INTO CREDENTIALS "
                    "(caption, username, flags, type) "
                    "VALUES(:caption, :username, :flags, :type)"));
    }
    q.bindValue(S(":username"),
                info.isUserNameSecret() ? QString() : info.userName());
    q.bindValue(S(":caption"), info.caption());
    q.bindValue(S(":flags"), flags);
    q.bindValue(S(":type"), info.type());
    exec(q);
    if (errorOccurred()) {
        TRACE() << "Error occurred while updating crendentials";
        return 0;
    }

    if (info.isNew()) {
        /* Fetch id of the inserted credentials */
        QVariant idVariant = q.lastInsertId();
        if (!idVariant.isValid()) {
            TRACE() << "Error occurred while inserting crendentials";
            return 0;
        }
        id = idVariant.toUInt();
    } else {
        id = info.id() ;
    }

    return id;
}

bool MetaDataDB::updateRealms(quint32 id, const QStringList &realms, bool isNew)
{
    QString queryStr;

    if (!isNew) {
        //remove realms list
        queryStr = QString::fromLatin1(
            "DELETE FROM REALMS WHERE identity_id = '%1'")
            .arg(id);
        exec(queryStr);
    }

    /* Realms insert */
    QSqlQuery q = newQuery();
    q.prepare(S("INSERT OR IGNORE INTO REALMS (identity_id, realm) "
                "VALUES (:id, :realm)"));
    foreach (QString realm, realms) {
        q.bindValue(S(":id"), id);
        q.bindValue(S(":realm"), realm);
        exec(q);
        if (errorOccurred()) return false;
    }
    return true;
}

/* Error monitor class */

CredentialsDB::ErrorMonitor::ErrorMonitor(CredentialsDB *db)
{
    db->_lastError.setType(SignOn::CredentialsDBError::NoError);
    db->metaDataDB->clearError();
    if (db->secretsStorage != 0)
        db->secretsStorage->clearError();
    _db = db;
}

CredentialsDB::ErrorMonitor::~ErrorMonitor()
{
    /* If there's an error set on the CredentialsDB, just let it be and return.
     * If not, take the error from the SqlDatabase objects, if any.
     */
    if (_db->_lastError.isValid())
        return;

    if (_db->secretsStorage != 0 &&
        _db->secretsStorage->lastError().isValid()) {
        _db->_lastError = _db->secretsStorage->lastError();
        return;
    }

    _db->_lastError = _db->metaDataDB->lastError();
}

/*    -------   CredentialsDB  implementation   -------    */

CredentialsDB::CredentialsDB(const QString &metaDataDbName,
                             SignOn::AbstractSecretsStorage *secretsStorage):
    secretsStorage(secretsStorage),
    m_secretsCache(new SecretsCache),
    metaDataDB(new MetaDataDB(metaDataDbName))
{
    noSecretsDB = SignOn::CredentialsDBError(
        QLatin1String("Secrets DB not opened"),
        SignOn::CredentialsDBError::ConnectionError);
}

CredentialsDB::~CredentialsDB()
{
    TRACE();

    delete m_secretsCache;

    if (metaDataDB) {
        QString connectionName = metaDataDB->connectionName();
        delete metaDataDB;
        QSqlDatabase::removeDatabase(connectionName);
    }
}

bool CredentialsDB::init()
{
    return metaDataDB->init();
}

bool CredentialsDB::openSecretsDB(const QString &secretsDbName)
{
    QVariantMap configuration;
    configuration.insert(QLatin1String("name"), secretsDbName);
    if (!secretsStorage->initialize(configuration)) {
        TRACE() << "SecretsStorage initialization failed: " <<
            secretsStorage->lastError().text();
        return false;
    }

    m_secretsCache->storeToDB(secretsStorage);
    m_secretsCache->clear();
    return true;
}

bool CredentialsDB::isSecretsDBOpen()
{
    return secretsStorage != 0 && secretsStorage->isOpen();
}

void CredentialsDB::closeSecretsDB()
{
    if (secretsStorage != 0) secretsStorage->close();
}

SignOn::CredentialsDBError CredentialsDB::lastError() const
{
    return _lastError;
}

QStringList CredentialsDB::methods(const quint32 id,
                                   const QString &securityToken)
{
    INIT_ERROR();
    return metaDataDB->methods(id, securityToken);
}

bool CredentialsDB::checkPassword(const quint32 id,
                                  const QString &username,
                                  const QString &password)
{
    INIT_ERROR();
    RETURN_IF_NO_SECRETS_DB(false);
    SignonIdentityInfo info = metaDataDB->identity(id);
    if (info.isUserNameSecret()) {
        return secretsStorage->checkPassword(id, username, password);
    } else {
        return username == info.userName() &&
            secretsStorage->checkPassword(id, QString(), password);
    }
}

SignonIdentityInfo CredentialsDB::credentials(const quint32 id,
                                              bool queryPassword)
{
    TRACE() << "id:" << id << "queryPassword:" << queryPassword;
    INIT_ERROR();
    SignonIdentityInfo info = metaDataDB->identity(id);
    if (queryPassword && !info.isNew()) {
        QString username, password;
        if (info.storePassword() && isSecretsDBOpen()) {
            TRACE() << "Loading credentials from DB.";
            secretsStorage->loadCredentials(id, username, password);
        } else {
            TRACE() << "Looking up credentials from cache.";
            m_secretsCache->lookupCredentials(id, username, password);
        }
        if (info.isUserNameSecret())
            info.setUserName(username);
        info.setPassword(password);

#ifdef DEBUG_ENABLED
        if (password.isEmpty()) {
            TRACE() << "Password is empty";
        }
#endif
    }
    return info;
}

QList<SignonIdentityInfo>
CredentialsDB::credentials(const QMap<QString, QString> &filter)
{
    INIT_ERROR();
    return metaDataDB->identities(filter);
}

quint32 CredentialsDB::insertCredentials(const SignonIdentityInfo &info)
{
    SignonIdentityInfo newInfo = info;
    if (!info.isNew())
        newInfo.setNew();
    return updateCredentials(newInfo);
}

quint32 CredentialsDB::updateCredentials(const SignonIdentityInfo &info)
{
    INIT_ERROR();
    quint32 id = metaDataDB->updateIdentity(info);
    if (id == 0) return id;

    if (info.hasSecrets()) {
        QString password = info.password();
        QString userName;
        if (info.isUserNameSecret())
            userName = info.userName();

        if (info.storePassword() && isSecretsDBOpen()) {
            secretsStorage->updateCredentials(id, userName, password);
        } else {
            /* Cache username and password in memory */
            m_secretsCache->updateCredentials(id, userName, password,
                                              info.storePassword());
        }
    }

    Q_EMIT credentialsUpdated(id);

    return id;
}

bool CredentialsDB::removeCredentials(const quint32 id)
{
    INIT_ERROR();

    /* We don't allow removing the credentials if the secrets DB is not
     * available */
    RETURN_IF_NO_SECRETS_DB(false);

    return secretsStorage->removeCredentials(id) &&
        metaDataDB->removeIdentity(id);
}

bool CredentialsDB::clear()
{
    TRACE();

    INIT_ERROR();

    /* We don't allow clearing the DB if the secrets DB is not available */
    RETURN_IF_NO_SECRETS_DB(false);

    return secretsStorage->clear() && metaDataDB->clear();
}

QVariantMap CredentialsDB::loadData(const quint32 id, const QString &method)
{
    TRACE() << "Loading:" << id << "," << method;

    INIT_ERROR();
    if (id == 0) return QVariantMap();

    quint32 methodId = metaDataDB->methodId(method);
    if (methodId == 0) return QVariantMap();

    if (isSecretsDBOpen()) {
        return secretsStorage->loadData(id, methodId);
    } else {
        TRACE() << "Looking up data from cache";
        return m_secretsCache->lookupData(id, methodId);
    }
}

bool CredentialsDB::storeData(const quint32 id, const QString &method,
                              const QVariantMap &data)
{
    TRACE() << "Storing:" << id << "," << method;

    INIT_ERROR();
    if (id == 0) return false;

    quint32 methodId = metaDataDB->methodId(method);
    if (methodId == 0) {
        bool ok = false;
        methodId = metaDataDB->insertMethod(method, &ok);
        if (!ok)
            return false;
    }

    if (isSecretsDBOpen()) {
        return secretsStorage->storeData(id, methodId, data);
    } else {
        TRACE() << "Storing data into cache";
        m_secretsCache->updateData(id, methodId, data);
        return true;
    }
}

bool CredentialsDB::removeData(const quint32 id, const QString &method)
{
    TRACE() << "Removing:" << id << "," << method;

    INIT_ERROR();
    RETURN_IF_NO_SECRETS_DB(false);
    if (id == 0) return false;

    quint32 methodId;
    if (!method.isEmpty()) {
        methodId = metaDataDB->methodId(method);
        if (methodId == 0) return false;
    } else {
        methodId = 0;
    }

    return secretsStorage->removeData(id, methodId);
}

QStringList CredentialsDB::accessControlList(const quint32 identityId)
{
    INIT_ERROR();
    return metaDataDB->accessControlList(identityId);
}

QStringList CredentialsDB::ownerList(const quint32 identityId)
{
    INIT_ERROR();
    return metaDataDB->ownerList(identityId);
}

QString CredentialsDB::credentialsOwnerSecurityToken(const quint32 identityId)
{
    //return first owner token
    QStringList owners = ownerList(identityId);
    return owners.count() ? owners.at(0) : QString();
}

bool CredentialsDB::addReference(const quint32 id,
                                 const QString &token,
                                 const QString &reference)
{
    INIT_ERROR();
    return metaDataDB->addReference(id, token, reference);
}

bool CredentialsDB::removeReference(const quint32 id,
                                    const QString &token,
                                    const QString &reference)
{
    INIT_ERROR();
    return metaDataDB->removeReference(id, token, reference);
}

QStringList CredentialsDB::references(const quint32 id, const QString &token)
{
    INIT_ERROR();
    return metaDataDB->references(id, token);
}

} //namespace SignonDaemonNS
