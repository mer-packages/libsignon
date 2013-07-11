/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of signon
 *
 * Copyright (C) 2011 Canonical Ltd.
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

#include "default-secrets-storage.h"
#include "signond-common.h"

#define RETURN_IF_NOT_OPEN(retval) \
    if (!isOpen()) { \
        TRACE() << "Secrets DB is not available"; \
        SignOn::CredentialsDBError error(QLatin1String("Not open"), \
                                         SignOn::CredentialsDBError::NotOpen); \
        setLastError(error); return retval; \
    }

#define S(s) QLatin1String(s)

using namespace SignonDaemonNS;

bool SecretsDB::createTables()
{
    QStringList createTableQuery = QStringList()
        <<  QString::fromLatin1(
            "CREATE TABLE CREDENTIALS"
            "(id INTEGER NOT NULL UNIQUE,"
            "username TEXT,"
            "password TEXT,"
            "PRIMARY KEY (id))")
        <<  QString::fromLatin1(
            "CREATE TABLE STORE"
            "(identity_id INTEGER,"
            "method_id INTEGER,"
            "key TEXT,"
            "value BLOB,"
            "PRIMARY KEY (identity_id, method_id, key))")

        << QString::fromLatin1(
            // Cascading Delete
            "CREATE TRIGGER tg_delete_credentials "
            "BEFORE DELETE ON CREDENTIALS "
            "FOR EACH ROW BEGIN "
            "    DELETE FROM STORE WHERE STORE.identity_id = OLD.id; "
            "END; "
        );

   foreach (QString createTable, createTableQuery) {
        QSqlQuery query = exec(createTable);
        if (lastError().isValid()) {
            TRACE() << "Error occurred while creating the database.";
            return false;
        }
        query.clear();
        commit();
    }
    return true;
}

bool SecretsDB::clear()
{
    TRACE();

    QStringList clearCommands = QStringList()
        << QLatin1String("DELETE FROM CREDENTIALS")
        << QLatin1String("DELETE FROM STORE");

    return transactionalExec(clearCommands);
}

bool SecretsDB::updateCredentials(const quint32 id,
                                  const QString &username,
                                  const QString &password)
{
    if (!startTransaction()) {
        TRACE() << "Could not start transaction. Error inserting credentials.";
        return false;
    }
    QSqlQuery query = newQuery();

    TRACE() << "INSERT:" << id;
    query.prepare(S("INSERT OR REPLACE INTO CREDENTIALS "
                    "(id, username, password) "
                    "VALUES(:id, :username, :password)"));

    query.bindValue(S(":id"), id);
    query.bindValue(S(":username"), username);
    query.bindValue(S(":password"), password);

    exec(query);

    if (errorOccurred()) {
        rollback();
        TRACE() << "Error occurred while storing crendentials";
        return false;
    }
    return commit();
}

bool SecretsDB::removeCredentials(const quint32 id)
{
    TRACE();

    QStringList queries = QStringList()
        << QString::fromLatin1(
            "DELETE FROM CREDENTIALS WHERE id = %1").arg(id)
        << QString::fromLatin1(
            "DELETE FROM STORE WHERE identity_id = %1").arg(id);

    return transactionalExec(queries);
}

bool SecretsDB::loadCredentials(const quint32 id,
                                QString &username,
                                QString &password)
{
    TRACE();

    QString queryStr =
        QString::fromLatin1("SELECT username, password FROM credentials "
                            "WHERE id = %1").arg(id);
    QSqlQuery query = exec(queryStr);
    if (!query.first()) {
        TRACE() << "No result or invalid credentials query.";
        return false;
    }

    username = query.value(0).toString();
    password = query.value(1).toString();
    return true;
}

QVariantMap SecretsDB::loadData(quint32 id, quint32 method)
{
    TRACE();

    QSqlQuery q = newQuery();
    q.prepare(S("SELECT key, value "
                "FROM STORE WHERE identity_id = :id AND method_id = :method"));
    q.bindValue(S(":id"), id);
    q.bindValue(S(":method"), method);
    exec(q);
    if (errorOccurred())
        return QVariantMap();

    QVariantMap result;
    while (q.next()) {
        QByteArray array;
        array = q.value(1).toByteArray();
        QDataStream stream(array);
        QVariant data;
        stream >> data;
        result.insert(q.value(0).toString(), data);
    }
    return result;
}

bool SecretsDB::storeData(quint32 id, quint32 method, const QVariantMap &data)
{
    TRACE();

    if (!startTransaction()) {
        TRACE() << "Could not start transaction. Error inserting data.";
        return false;
    }

    /* first, remove existing data */
    QSqlQuery q = newQuery();
    q.prepare(S("DELETE FROM STORE WHERE identity_id = :id "
                "AND method_id = :method"));
    q.bindValue(S(":id"), id);
    q.bindValue(S(":method"), method);
    exec(q);
    if (errorOccurred()) {
        rollback();
        TRACE() << "Data removal failed.";
        return false;
    }

    bool allOk = true;
    qint32 dataCounter = 0;
    if (!(data.keys().empty())) {
        QMapIterator<QString, QVariant> it(data);
        while (it.hasNext()) {
            it.next();

            QByteArray array;
            QDataStream stream(&array, QIODevice::WriteOnly);
            stream << it.value();

            dataCounter += it.key().size() +array.size();
            if (dataCounter >= SSO_MAX_TOKEN_STORAGE) {
                BLAME() << "storing data max size exceeded";
                allOk = false;
                break;
            }
            /* Key/value insert/replace/delete */
            QSqlQuery query = newQuery();
            if (!it.value().isValid() || it.value().isNull()) {
                continue;
            }
            TRACE() << "insert";
            query.prepare(S(
                "INSERT OR REPLACE INTO STORE "
                "(identity_id, method_id, key, value) "
                "VALUES(:id, :method, :key, :value)"));
            query.bindValue(S(":value"), array);
            query.bindValue(S(":id"), id);
            query.bindValue(S(":method"), method);
            query.bindValue(S(":key"), it.key());
            exec(query);
            if (errorOccurred()) {
                allOk = false;
                break;
            }
        }
    }

    if (allOk && commit()) {
        TRACE() << "Data insertion ok.";
        return true;
    }
    rollback();
    TRACE() << "Data insertion failed.";
    return false;
}

bool SecretsDB::removeData(quint32 id, quint32 method)
{
    TRACE();

    if (!startTransaction()) {
        TRACE() << "Could not start transaction. Error removing data.";
        return false;
    }

    QSqlQuery q = newQuery();
    if (method == 0) {
        q.prepare(S("DELETE FROM STORE WHERE identity_id = :id"));
    } else {
        q.prepare(S("DELETE FROM STORE WHERE identity_id = :id "
                    "AND method_id = :method"));
        q.bindValue(S(":method"), method);
    }
    q.bindValue(S(":id"), id);
    exec(q);
    if (!errorOccurred() && commit()) {
        TRACE() << "Data removal ok.";
        return true;
    } else {
        rollback();
        TRACE() << "Data removal failed.";
        return false;
    }
}

DefaultSecretsStorage::DefaultSecretsStorage(QObject *parent):
    AbstractSecretsStorage(parent)
{
}

DefaultSecretsStorage::~DefaultSecretsStorage()
{
    close();
}

bool DefaultSecretsStorage::initialize(const QVariantMap &configuration)
{
    if (isOpen()) {
        TRACE() << "Initializing open DB; closing first...";
        close();
    }

    QString name = configuration.value(QLatin1String("name")).toString();

    m_secretsDB = new SecretsDB(name);
    if (!m_secretsDB->init()) {
        setLastError(m_secretsDB->lastError());
        delete m_secretsDB;
        m_secretsDB = 0;
        return false;
    }

    setIsOpen(true);
    return true;
}

bool DefaultSecretsStorage::close()
{
    if (m_secretsDB != 0) {
        QString connectionName = m_secretsDB->connectionName();
        delete m_secretsDB;
        QSqlDatabase::removeDatabase(connectionName);
        m_secretsDB = 0;
    }
    return AbstractSecretsStorage::close();
}

bool DefaultSecretsStorage::clear()
{
    RETURN_IF_NOT_OPEN(false);

    return m_secretsDB->clear();
}

bool DefaultSecretsStorage::updateCredentials(const quint32 id,
                                              const QString &username,
                                              const QString &password)
{
    RETURN_IF_NOT_OPEN(false);

    return m_secretsDB->updateCredentials(id, username, password);
}

bool DefaultSecretsStorage::removeCredentials(const quint32 id)
{
    RETURN_IF_NOT_OPEN(false);

    return m_secretsDB->removeCredentials(id);
}

bool DefaultSecretsStorage::loadCredentials(const quint32 id,
                                            QString &username,
                                            QString &password)
{
    RETURN_IF_NOT_OPEN(false);

    return m_secretsDB->loadCredentials(id, username, password);
}

QVariantMap DefaultSecretsStorage::loadData(quint32 id, quint32 method)
{
    RETURN_IF_NOT_OPEN(QVariantMap());

    return m_secretsDB->loadData(id, method);
}

bool DefaultSecretsStorage::storeData(quint32 id, quint32 method,
                                      const QVariantMap &data)
{
    RETURN_IF_NOT_OPEN(false);

    return m_secretsDB->storeData(id, method, data);
}

bool DefaultSecretsStorage::removeData(quint32 id, quint32 method)
{
    RETURN_IF_NOT_OPEN(false);

    return m_secretsDB->removeData(id, method);
}
