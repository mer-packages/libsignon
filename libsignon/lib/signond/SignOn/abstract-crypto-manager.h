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
 * @file cryptomanager.h
 * Definition of the AbstractCryptoManager object.
 * @ingroup Accounts_and_SSO_Framework
 */

#ifndef SIGNON_ABSTRACT_CRYPTO_MANAGER_H
#define SIGNON_ABSTRACT_CRYPTO_MANAGER_H

#include "SignOn/abstract-key-manager.h"

#include <QObject>
#include <QStringList>
#include <QVariantMap>

namespace SignOn {

class AbstractCryptoManagerPrivate;

/*!
 * @class AbstractCryptoManager
 * Manages the credentials storage file system encryption. The API is modeled
 * after a LUKS-based implementation.
 * @ingroup Accounts_and_SSO_Framework
 */
class SIGNON_EXPORT AbstractCryptoManager: public QObject
{
    Q_OBJECT

public:
    /*!
     * Constructs a AbstractCryptoManager object with the given parent.
     * @param parent
     */
    explicit AbstractCryptoManager(QObject *parent = 0);

    /*!
     * Destructor.
     */
    ~AbstractCryptoManager();

    /*!
     * Sets the encryption key.
     * @param key the encrypted file system key.
     */
    void setEncryptionKey(const SignOn::Key &key);

    /*!
     * @return The in use encryption key.
     */
    SignOn::Key encryptionKey() const;

    /*!
     * Initializes the object with the given configuration.
     * @param configuration A dictionary of configuration options, dependent on
     * the specific crypto-manager implementation.
     * @returns true if the configuration is valid.
    */
    virtual bool initialize(const QVariantMap &configuration);

    /*!
     * Sets up an encrypted file system. This method is to be called only at
     * the file system creation/foramtting.
     * Use mountFileSystem() on subsequent uses. This method handles also the
     * mounting so when using it, a call to mountFileSystem() is not necessary.
     *
     * @returns true if successful, false otherwise.
     * @warning this method might format the credentials file system, use
     * carefully.
     */
    virtual bool setupFileSystem();

    /*!
     * Deletes the encrypted file system.
     * @returns true if successful, false otherwise.
     * @warning use this carefully, this will lead to data loss.
     */
    virtual bool deleteFileSystem();

    /*!
     * @returns true if the file system is setup, false otherwise.
     */
    bool fileSystemIsSetup() const;

    /*!
     * Mounts the encrypted file system.
     * @returns true if successful, false otherwise.
     */
    virtual bool mountFileSystem();

    /*!
     * Unmounts the encrypted file system.
     * @returns true if successful, false otherwise.
     */
    virtual bool unmountFileSystem();

    /*!
     * @returns true if the file system is mounted, false otherwise.
     */
    bool fileSystemIsMounted() const;

    /*!
     * @returns the path of the mounted file system.
     */
    virtual QString fileSystemMountPath() const;

    /*!
     * @returns the list of files which need to be backed up.
     */
    virtual QStringList backupFiles() const;

    /*!
     * @attention if the file system is not mounted and the encryption key can
     * access it, this method will cause the file system to be mounted.
     * @returns whether the key @key is occupying a keyslot in the encrypted
     * file system.
     */
    virtual bool encryptionKeyInUse(const SignOn::Key &key);

    /*!
     * Adds an encryption key to one of the available keyslots of the LUKS
     * partition's header.
     * @sa encryptionKeyInUse()
     * @param key The key to be added/set.
     * @param existingKey An already existing key.
     * @returns true if succeeded, false otherwise.
     */
    virtual bool addEncryptionKey(const SignOn::Key &key,
                                  const SignOn::Key &existingKey);

    /*!
     * Releases an existing used keyslot in the LUKS partition's header.
     * @param key The key to be removed.
     * @param remainingKey Another valid key
     * @attention The system cannot remain keyless.
     * @returns true if succeeded, false otherwise.
    */
    virtual bool removeEncryptionKey(const SignOn::Key &key,
                                     const SignOn::Key &remainingKey);

Q_SIGNALS:
    void fileSystemMounted();
    void fileSystemUnmounting();

protected:
    void setFileSystemMounted(bool isMounted);
    void setFileSystemSetup(bool isSetup);

private:
    AbstractCryptoManagerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(AbstractCryptoManager)
};

} //namespace

#endif // SIGNON_ABSTRACT_CRYPTO_MANAGER_H
