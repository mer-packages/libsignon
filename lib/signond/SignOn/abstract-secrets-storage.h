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

#ifndef SIGNON_ABSTRACT_SECRETS_STORAGE_H
#define SIGNON_ABSTRACT_SECRETS_STORAGE_H

#include <SignOn/export.h>

#include <QObject>
#include <QVariantMap>

namespace SignOn {

class SIGNON_EXPORT CredentialsDBError
{
public:
    enum ErrorType {
        NoError,
        NotOpen,                /*!< The DB is not open */
        ConnectionError,        /*!< The DB is disconnected */
        StatementError,         /*!< The last command failed */
        UnknownError
    };
    CredentialsDBError(const QString &text = QString(),
                       ErrorType type = NoError) {
        m_text = text; m_type = type;
    }
    ~CredentialsDBError() {};

    void setType(ErrorType type) { m_type = type; }
    void setText(const QString &text) { m_text = text; }
    void clear() { m_type = NoError; m_text.clear(); }

    QString text() const { return m_text; }
    ErrorType type() const { return m_type; }
    bool isValid() const { return m_type != NoError; }

private:
    QString m_text;
    ErrorType m_type;
};

class AbstractSecretsStoragePrivate;

/*!
 * @class AbstractSecretsStorage
 * @headerfile SignOn/abstract-secrets-storage.h SignOn/AbstractSecretsStorage
 * @brief Interface for secrets storage implementations.
 *
 * @ingroup Accounts_and_SSO_Framework
 */
class SIGNON_EXPORT AbstractSecretsStorage: public QObject
{
public:
    /*!
     * Constructor.
     * @param parent Parent object.
     */
    explicit AbstractSecretsStorage(QObject *parent = 0);
    ~AbstractSecretsStorage();

    /*!
     * Initializes and opens the secrets DB. The implementation should take
     * care of creating the DB, if it doesn't exist.
     * @param configuration A dictionary of configuration options, dependent on
     * the specific SecretsStorage implementation.
     * @returns true if successful, false otherwise.
     */
    virtual bool initialize(const QVariantMap &configuration) = 0;

    /*!
     * Closes the secrets DB. To reopen it, call initialize().
     * @returns true if successful, false otherwise.
     */
    virtual bool close();

    bool isOpen() const;

    /*!
     * Removes all stored secrets.
     * @returns true if successful, false otherwise.
     */
    virtual bool clear() = 0;

    /*!
     * Stores/updates the credentials for the given identity.
     * @param id the identity whose credentials are being updated.
     * @param username the username.
     * @param password the password.
     * @returns true if successful, false otherwise.
     */
    virtual bool updateCredentials(const quint32 id,
                                   const QString &username,
                                   const QString &password) = 0;
    /*!
     * Remove the credentials for the given identity.
     * @param id the identity whose credentials are being deleted.
     * @returns true if successful, false otherwise.
     */
    virtual bool removeCredentials(const quint32 id) = 0;

    /*!
     * Loads the credentials.
     * @param id the identity whose credentials are being loaded.
     * @param username string for holding the loaded username.
     * @param password string for holding the loaded password.
     * @returns true if successful, false otherwise.
     */
    virtual bool loadCredentials(const quint32 id,
                                 QString &username,
                                 QString &password) = 0;

    /*!
     * Checks whether the given username and passwords are correct for the
     * given identity.
     * @param id the identity whose credentials are being checked.
     * @param username the username.
     * @param password the password.
     * @returns true if username and password match.
     */
    virtual bool checkPassword(const quint32 id,
                               const QString &username,
                               const QString &password);

    /*!
     * Loads extra secret data.
     * @param id the identity whose data are being loaded.
     * @param method the authentication method the data is used for.
     * @returns a dictionary with the data.
     */
    virtual QVariantMap loadData(quint32 id, quint32 method) = 0;

    /*!
     * Stores extra secret data. Calling this method replaces any data which
     * was previously stored for the given id/method.
     * @param id the identity whose data are being stored.
     * @param method the authentication method the data is used for.
     * @param data a dictionary with the data.
     * @returns true if successful, false otherwise.
     */
    virtual bool storeData(quint32 id, quint32 method, const QVariantMap &data) = 0;

    /*!
     * Removed extra secret data.
     * @param id the identity whose data are being removed.
     * @param method the authentication method the data is used for.
     * @returns true if successful, false otherwise.
     */
    virtual bool removeData(quint32 id, quint32 method) = 0;

    /*!
     * Get the last error.
     */
    CredentialsDBError lastError() const;

    /*!
     * Clear the last error.
     */
    void clearError();

protected:
    void setIsOpen(bool isOpen);
    void setLastError(const CredentialsDBError &error);

private:
    AbstractSecretsStoragePrivate *d_ptr;
    Q_DECLARE_PRIVATE(AbstractSecretsStorage)
};

} // namespace

#endif // SIGNON_ABSTRACT_SECRETS_STORAGE_H
