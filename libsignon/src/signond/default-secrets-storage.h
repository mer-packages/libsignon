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

/*!
 * @file secrets-storage.h
 * Definition of the DefaultSecretsStorage object.
 * @ingroup Accounts_and_SSO_Framework
 */

#ifndef SIGNON_DEFAULT_SECRETS_STORAGE_H
#define SIGNON_DEFAULT_SECRETS_STORAGE_H

#include "SignOn/abstract-secrets-storage.h"
#include "credentialsdb.h"
#include "credentialsdb_p.h"

#include <QObject>

namespace SignonDaemonNS {

class SecretsDB: public SqlDatabase
{
    friend class ::TestDatabase;
public:
    SecretsDB(const QString &name):
        SqlDatabase(name, QLatin1String("SSO-secrets"), SSO_SECRETSDB_VERSION) {}

    bool createTables();
    bool clear();

    bool updateCredentials(const quint32 id,
                           const QString &username,
                           const QString &password);
    bool removeCredentials(const quint32 id);
    bool loadCredentials(const quint32 id,
                         QString &username,
                         QString &password);

    QVariantMap loadData(quint32 id, quint32 method);
    bool storeData(quint32 id, quint32 method, const QVariantMap &data);
    bool removeData(quint32 id, quint32 method);
};


/*!
 * @class DefaultSecretsStorage
 * SQLite-based implementation of the AbstractSecretsStorage interface. The
 * secrets are stored in a SQLite DB, unencrypted. To achieve encryption, you
 * can pair this class with a CryptoManager which provide an encrypted
 * filesystem.
 * @ingroup Accounts_and_SSO_Framework
 */
class DefaultSecretsStorage: public SignOn::AbstractSecretsStorage
{
    Q_OBJECT

public:
    explicit DefaultSecretsStorage(QObject *parent = 0);
    ~DefaultSecretsStorage();

    /* reimplemented virtual methods */
    bool initialize(const QVariantMap &configuration);
    bool close();
    bool clear();
    bool updateCredentials(const quint32 id,
                           const QString &username,
                           const QString &password);
    bool removeCredentials(const quint32 id);
    bool loadCredentials(const quint32 id,
                         QString &username,
                         QString &password);
    QVariantMap loadData(quint32 id, quint32 method);
    bool storeData(quint32 id, quint32 method, const QVariantMap &data);
    bool removeData(quint32 id, quint32 method);

private:
    SecretsDB *m_secretsDB;
};

} //namespace

#endif // SIGNON_DEFAULT_SECRETS_STORAGE_H
