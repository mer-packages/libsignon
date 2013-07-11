/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2011 Nokia Corporation.
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
 * @file cryptomanager.h
 * Definition of the CryptoManager object.
 * @ingroup Accounts_and_SSO_Framework
 */

#ifndef SIGNON_CRYPTO_MANAGER_H
#define SIGNON_CRYPTO_MANAGER_H

#include <SignOn/AbstractCryptoManager>

#include <QObject>

#define MINUMUM_ENCRYPTED_FILE_SYSTEM_SIZE 4

/*!
 * @class CryptoManager
 * Encrypted file system manager. Uses cryptsetup and LUKS.
 * @ingroup Accounts_and_SSO_Framework
 */
class CryptoManager: public SignOn::AbstractCryptoManager
{
    Q_OBJECT

    // DO NOT change the order of the enum values!!!
    enum FileSystemMountState {
        Unmounted = 0,
        LoopSet,
        LoopLuksFormatted,
        LoopLuksOpened,
        Mounted
    };

    static const uint signonMinumumDbSize;
    static const char signonDefaultFileSystemName[];
    static const char signonDefaultFileSystemType[];

public:
    // reimplemented virtual methods
    bool initialize(const QVariantMap &configuration);

    /*!
     * @enum FileSystemType
     * Supported encrypted partion filesystem type.
     */
    enum FileSystemType {
        Ext2 = 0,
        Ext3,
        Ext4
    };

    /*!
     * Constructs a CryptoManager object with the given parent.
     * @param parent
     */
    CryptoManager(QObject *parent = 0);

    /*!
     * Destroys a CryptoManager object.
     */
    ~CryptoManager();

    /*!
     * Sets up an encrypted file system. This method is to be called only at
     * the file system creation/formatting.
     * Use mountFileSystem() on subsequent uses. This method handles also the
     * mounting so when using it, a call
     * to mountFileSystem() is not necessary.
     * @returns true, if successful, false otherwise.
     * @warning this method will always format the file system, use carefully.
     */
    bool setupFileSystem();

    /*!
     * Deletes the encrypted file system.
     * @returns true, if successful, false otherwise.
     * @warning use this carefully, this will lead to data loss.
     * @todo finish implemetation.
     */
    bool deleteFileSystem();

    /*!
     * Mounts the encrypted file system.
     * @returns true, if successful, false otherwise.
     */
    bool mountFileSystem();

    /*!
     * Unmounts the encrypted file system.
     * @returns true, if successful, false otherwise.
     */
    bool unmountFileSystem();

    /*!
     * @returns the path of the mounted file system.
     */
    QString fileSystemMountPath() const;

    /*!
     * @returns the list of files which need to be backed up.
     */
    QStringList backupFiles() const;

    /*!
     * @attention if the file system is not mounted and the encryption
     * key can access it, this method will cause the file system to be
     * mounted.
     * @returns whether the key `key` is occupying a keyslot in the encrypted
     * file system.
     */
    bool encryptionKeyInUse(const SignOn::Key &key);

    /*!
     * Adds an encryption key to one of the available keyslots of the LUKS
     * partition's header.
     * Use the `keyTag` parameter in order to store and keep track of the key.
     * @sa isEncryptionKey(const SignOn::Key &key)
     * @param key The key to be added/set.
     * @param existingKey An already existing key.
     * @returns true, if succeeded, false otherwise.
     */
    bool addEncryptionKey(const SignOn::Key &key,
                          const SignOn::Key &existingKey);

    /*!
     * Releases an existing used keyslot in the LUKS partition's header.
     * @param key The key to be removed.
     * @param remainingKey Another valid key
     * @attention The system cannot remain keyless.
     * @returns true, if succeeded, false otherwise.
     */
    bool removeEncryptionKey(const SignOn::Key &key,
                             const SignOn::Key &remainingKey);

private:
    bool setFileSystemType(const QString &type);
    bool setFileSystemSize(const quint32 size);
    void setFileSystemPath(const QString &path);

    void checkFileSystemSetup();
    void clearFileSystemResources();
    bool mountMappedDevice();
    bool unmountMappedDevice();
    void updateMountState(const FileSystemMountState state);

    static bool createPartitionFile(const QString &filePath);
    static bool formatMapFileSystem(const QString &fileSystemPath);

    const QString keychainFilePath() const;
    void addKeyToKeychain(const QByteArray &key) const;
    void removeKeyFromKeychain(const QByteArray &key) const;
    bool keychainContainsKey(const QByteArray &key) const;

private:
    //TODO remove this
    void serializeData();

private:
    QString m_fileSystemPath;
    QString m_fileSystemMapPath;
    QString m_fileSystemName;
    QString m_fileSystemMountPath;
    QString m_loopDeviceName;

    FileSystemMountState m_mountState;
    FileSystemType m_fileSystemType;
    quint32 m_fileSystemSize;
};

#endif // SIGNON_CRYPTOMANAGER_H
