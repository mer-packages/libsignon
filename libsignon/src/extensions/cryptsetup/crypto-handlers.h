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
 * @file cryptohandlers.h
 * Definition of the CryptsetupHandler object.
 * @ingroup Accounts_and_SSO_Framework
 */

#ifndef SIGNON_CRYPTSETUP_HANDLER_H
#define SIGNON_CRYPTSETUP_HANDLER_H

#include <QProcess>
#include <QMap>
#include <QFile>


/*!
 * @class SystemCommandLineCallHandler
 * Handles calls to system command line tools
 */
class SystemCommandLineCallHandler: public QObject
{
    Q_OBJECT

public:
    /*!
     * Basic constructor
     */
    SystemCommandLineCallHandler();

    /*!
     * Destructor
     */
    ~SystemCommandLineCallHandler();

    /*!
     * Executes the application at appPath in a separate child process.
     * @param appPath Path of the application to be executed.
     * @param args List of arguments for the executed application.
     * @param readOutput Flag whether to save or not the std::out output of the
     * executed application.
     * @returns true upon success, false otherwise.
     */
    bool makeCall(const QString &appPath,
                  const QStringList &args,
                  bool readOutput = false);

    /*!
     * @returns the raw untrimmed output of the last process called with
     * makeCall and readOutput set to true
     */
    QByteArray output() const { return m_output; }

private Q_SLOTS:
    void error(QProcess::ProcessError err);

private:
    QProcess m_process;
    QByteArray m_output;

    Q_DISABLE_COPY(SystemCommandLineCallHandler);
};

/*!
 * @class MountHandler
 * Handles mounting and unmounting of file systems
 */
struct PartitionHandler
{
    enum {
        Ext2 = 0,
        Ext3,
        Ext4
    };
    /*!
     * Creates a random data file of fileSize Mb.
     * @param fileName The name of the file to be created.
     * @param fileSize The size of the file to be created (Mb)
     */
    static bool createPartitionFile(const QString &fileName,
                                    const quint32 fileSize);

    /*!
     * Formats a file (block device) for a specific file system type
     * (ext2,ext3,ext4)
     * @param fileName Name of the file to be formatted.
     * @param fileSystemType Type of the file syste
     * @returns true upon success, false otherwise.
     */
    static bool formatPartitionFile(const QString &fileName,
                                    const quint32 fileSystemType);

private:
    /*!
     * Construction and copying disabled.
     */
    PartitionHandler();
    Q_DISABLE_COPY(PartitionHandler)
};


/*!
 * @class MountHandler
 */
struct MountHandler
{
    /*!
     * Mounts a block device to a specific location.
     * @param toMount File system to be mounted.
     * @param mounthPath Path to where the file system will be mounted.
     * @param fileSystemType The type of the file system to be mounted.
     * @returns true upon success, false otherwise.
     */
    static bool mount(const QString &source,
                      const QString &target,
                      const QString &fileSystemType = QLatin1String("ext2"));
    /*!
     * Unmounts a block device from a specific location.
     * @param mounthPath Path of the file system to be unmounted.
     * @returns true upon success, false otherwise.
     */
    static bool umount(const QString &target);

private:
    /*!
     * Construction and copying disabled.
     */
    MountHandler();
    Q_DISABLE_COPY(MountHandler)
};


/*!
 * @class LosetupHandler
 * Handles mounting, unmounting of loopback devices.
 * Also helps finding unused loopback devices.
 */
struct LosetupHandler
{
    /*!
     * Mounts a block device to loopback device.
     * @param deviceName Loopback device to pe set up.
     * @param blockDevice Block device to be loopback mounted.
     */
    static bool setupDevice(const QString &deviceName,
                            const QString &blockDevice);

    /*!
     * Finds an available loopback device.
     * @return the name of a spare device or a null string if none found.
     */
    static QString findAvailableDevice();

    /*!
     * Releases a used loopback device.
     * @param deviceName Loopback device to be released
     * @returns true upon success, false otherwise
     */
    static bool releaseDevice(const QString &deviceName);

private:
    /*!
     * Construction and copying disabled.
     */
    LosetupHandler();
    Q_DISABLE_COPY(LosetupHandler)
};


/*!
 * @class CryptsetupHandler
 * Wraps the libcryptsetup API functionality.
 * @ingroup Accounts_and_SSO_Framework
 */

struct CryptsetupHandler
{
    /*!
     * Formats the file system.
     * @param  key, key of the ecrypted file system
     * @param  deviceName, name of the loop device LUKS formatted.
     */
    static bool formatFile(const QByteArray &key, const QString &deviceName);

    /*!
     * Opens the file system.
     * @param  key, key of the ecrypted file system
     * @param  deviceName, name of the loop device to be opened.
     * @param  deviceMap, name of the device mapper mapped device.
     */
    static bool openFile(const QByteArray &key,
                         const QString &deviceName,
                         const QString &deviceMap);

    /*!
     * Closes the file system.
     * @param  deviceName, name of the mapped device to be closed.
     */
    static bool closeFile(const QString &deviceName);

    /*!
     * Removes the file system.
     * @param  deviceName, name of the device.
     * @todo implement this
     */
    static bool removeFile(const QString &deviceName);

    /*!
     * Adds a key to a free encryption header slot. This operation is to be
     * executed if at least one key is already set in the LUKS header.
     * @param  deviceName, name of the device.
     * @param  key, the key to be added.
     * @param  existingKey, an already existing key.
     * @returns whether the key was successfully added or not.
     * @todo implement this
     */
    static bool addKeySlot(const QString &deviceName,
                           const QByteArray &key,
                           const QByteArray &existingKey);

    /*!
     * Removes a key ocupying an encryption header slot
     * @param  deviceName, name of the device.
     * @param  key, the key to be removed.
     * @returns whether the key was successfully removed or not.
     * @todo implement this
     */
    static bool removeKeySlot(const QString &deviceName,
                              const QByteArray &key,
                              const QByteArray &remainingKey);

    /*!
     * Loads the `dm_mod` kernel module
     * @returns whether the dm_mod was successfully loaded or not.
     */
    static bool loadDmMod();

    /*!
     * @returns the last error as string.
     */
    static QString error();

private:
    /*!
     * Construction and copying disabled.
     */
    CryptsetupHandler();
    Q_DISABLE_COPY(CryptsetupHandler)
};

#endif // SIGNON_CRYPTSETUP_HANDLER_H
