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


#include "abstract-crypto-manager.h"
#include "debug.h"

using namespace SignOn;

namespace SignOn {

class AbstractCryptoManagerPrivate
{
    Q_DECLARE_PUBLIC(AbstractCryptoManager)

public:
    AbstractCryptoManagerPrivate(AbstractCryptoManager *cryptoManager);
    ~AbstractCryptoManagerPrivate() {};

private:
    mutable AbstractCryptoManager *q_ptr;
    Key m_encryptionKey;
    bool m_fileSystemIsSetup;
    bool m_fileSystemIsMounted;
};
};

AbstractCryptoManagerPrivate::AbstractCryptoManagerPrivate(
    AbstractCryptoManager *cryptoManager):
    q_ptr(cryptoManager),
    m_fileSystemIsSetup(false),
    m_fileSystemIsMounted(false)
{
}

AbstractCryptoManager::AbstractCryptoManager(QObject *parent):
    QObject(parent),
    d_ptr(new AbstractCryptoManagerPrivate(this))
{
}

AbstractCryptoManager::~AbstractCryptoManager()
{
    delete d_ptr;
}

void AbstractCryptoManager::setEncryptionKey(const SignOn::Key &key)
{
    if (fileSystemIsMounted()) {
        BLAME() << "File system already mounted";
        return;
    }

    d_ptr->m_encryptionKey = key;
}

SignOn::Key AbstractCryptoManager::encryptionKey() const
{
    return d_ptr->m_encryptionKey;
}

bool AbstractCryptoManager::initialize(const QVariantMap &configuration)
{
    Q_UNUSED(configuration);
    return true;
}

bool AbstractCryptoManager::setupFileSystem()
{
    setFileSystemSetup(true);
    return true;
}

bool AbstractCryptoManager::deleteFileSystem()
{
    setFileSystemSetup(false);
    return true;
}

bool AbstractCryptoManager::fileSystemIsSetup() const
{
    return d_ptr->m_fileSystemIsSetup;
}

bool AbstractCryptoManager::mountFileSystem()
{
    setFileSystemMounted(true);
    return true;
}

bool AbstractCryptoManager::unmountFileSystem()
{
    setFileSystemMounted(false);
    return true;
}

bool AbstractCryptoManager::fileSystemIsMounted() const
{
    return d_ptr->m_fileSystemIsMounted;
}

QString AbstractCryptoManager::fileSystemMountPath() const
{
    return QString();
}

QStringList AbstractCryptoManager::backupFiles() const
{
    return QStringList();
}

bool AbstractCryptoManager::encryptionKeyInUse(const SignOn::Key &key)
{
    Q_UNUSED(key);
    return true;
}

bool AbstractCryptoManager::addEncryptionKey(const SignOn::Key &key,
                                             const SignOn::Key &existingKey)
{
    Q_UNUSED(key);
    Q_UNUSED(existingKey);
    return true;
}

bool AbstractCryptoManager::removeEncryptionKey(const SignOn::Key &key,
                                                const SignOn::Key &remainingKey)
{
    Q_UNUSED(key);
    Q_UNUSED(remainingKey);
    return true;
}

void AbstractCryptoManager::setFileSystemMounted(bool isMounted)
{
    Q_D(AbstractCryptoManager);
    if (isMounted != d->m_fileSystemIsMounted) {
        if (d->m_fileSystemIsMounted) {
            Q_EMIT fileSystemUnmounting();
        }
        d->m_fileSystemIsMounted = isMounted;
        if (isMounted) {
            Q_EMIT fileSystemMounted();
        }
    }
}

void AbstractCryptoManager::setFileSystemSetup(bool isSetup)
{
    Q_D(AbstractCryptoManager);
    if (isSetup != d->m_fileSystemIsSetup) {
        d->m_fileSystemIsSetup = isSetup;
    }
}

