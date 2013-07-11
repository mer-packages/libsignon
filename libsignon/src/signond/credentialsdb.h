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

/*!
 * @file credentialsdb.h
 * Definition of the CredentialsDB object.
 * @ingroup Accounts_and_SSO_Framework
 */

#ifndef CREDENTIALS_DB_H
#define CREDENTIALS_DB_H

#include <QObject>
#include <QtSql>

#include "SignOn/abstract-secrets-storage.h"
#include "signonidentityinfo.h"

#define SSO_MAX_TOKEN_STORAGE (4*1024) // 4 kB for token store/identity/method

class TestDatabase;

namespace SignonDaemonNS {

/*!
 * @enum IdentityFlags
 * Flags to be stored into database
 */
enum IdentityFlags {
    Validated = 0x0001,
    RememberPassword = 0x0002,
    UserNameIsSecret = 0x0004,
};

class MetaDataDB;
class SecretsCache;

/*!
 * @class CredentialsDB
 * Manages the credentials I/O.
 * @ingroup Accounts_and_SSO_Framework
 */

class CredentialsDB: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CredentialsDB)

    friend class ::TestDatabase;

    class ErrorMonitor
    {
    public:
        /* The constructor clears the errors in CredentialsDB, MetaDataDB and
         * SecretsDB. */
        ErrorMonitor(CredentialsDB *db);
        /* The destructor collects the errors and sets
         * CredentialsDB::_lastError to the appropriate value. */
        ~ErrorMonitor();
    private:
        CredentialsDB *_db;
    };
    friend class ErrorMonitor;

public:
    CredentialsDB(const QString &metaDataDbName,
                  SignOn::AbstractSecretsStorage *secretsStorage);
    ~CredentialsDB();

    bool init();
    /*!
     * This method will open the DB file containing the user secrets.
     * If this method is not called, or if it fails, the secrets will not be
     * available.
     */
    bool openSecretsDB(const QString &secretsDbName);
    bool isSecretsDBOpen();
    void closeSecretsDB();

    SignOn::CredentialsDBError lastError() const;
    bool errorOccurred() const { return lastError().isValid(); };

    QStringList methods(const quint32 id,
                        const QString &securityToken = QString());
    bool checkPassword(const quint32 id,
                       const QString &username, const QString &password);
    SignonIdentityInfo credentials(const quint32 id, bool queryPassword = true);
    QList<SignonIdentityInfo> credentials(const QMap<QString, QString> &filter);

    quint32 insertCredentials(const SignonIdentityInfo &info);
    quint32 updateCredentials(const SignonIdentityInfo &info);
    bool removeCredentials(const quint32 id);

    bool clear();

    QStringList accessControlList(const quint32 identityId);
    QStringList ownerList(const quint32 identityId);
    QString credentialsOwnerSecurityToken(const quint32 identityId);

    QVariantMap loadData(const quint32 id, const QString &method);
    bool storeData(const quint32 id,
                   const QString &method,
                   const QVariantMap &data);
    bool removeData(const quint32 id, const QString &method = QString());

    bool addReference(const quint32 id,
                      const QString &token,
                      const QString &reference);
    bool removeReference(const quint32 id,
                         const QString &token,
                         const QString &reference = QString());
    QStringList references(const quint32 id,
                           const QString &token = QString());

private:
    SignOn::AbstractSecretsStorage *secretsStorage;
    SecretsCache *m_secretsCache;
    MetaDataDB *metaDataDB;
    SignOn::CredentialsDBError _lastError;
    SignOn::CredentialsDBError noSecretsDB;
};

} // namespace SignonDaemonNS

#endif // CREDENTIALSDB_H
