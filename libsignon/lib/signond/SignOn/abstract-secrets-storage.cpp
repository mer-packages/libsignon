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

#include "abstract-secrets-storage.h"
#include "debug.h"

using namespace SignOn;

namespace SignOn {

class AbstractSecretsStoragePrivate
{
    Q_DECLARE_PUBLIC(AbstractSecretsStorage)

public:
    AbstractSecretsStoragePrivate(AbstractSecretsStorage *secretsStorage);
    ~AbstractSecretsStoragePrivate() {};

private:
    mutable AbstractSecretsStorage *q_ptr;
    bool m_isOpen;
    CredentialsDBError m_lastError;
};
};

AbstractSecretsStoragePrivate::AbstractSecretsStoragePrivate(
    AbstractSecretsStorage *secretsStorage):
    q_ptr(secretsStorage),
    m_isOpen(false)
{
}

AbstractSecretsStorage::AbstractSecretsStorage(QObject *parent):
    QObject(parent),
    d_ptr(new AbstractSecretsStoragePrivate(this))
{
}

AbstractSecretsStorage::~AbstractSecretsStorage()
{
    delete d_ptr;
}

bool AbstractSecretsStorage::close()
{
    setIsOpen(false);
    return true;
}

bool AbstractSecretsStorage::isOpen() const
{
    return d_ptr->m_isOpen;
}

bool AbstractSecretsStorage::checkPassword(const quint32 id,
                                           const QString &username,
                                           const QString &password)
{
    QString storedUsername, storedPassword;

    if (!loadCredentials(id, storedUsername, storedPassword))
        return false;

    return storedUsername == username && storedPassword == password;
}

CredentialsDBError AbstractSecretsStorage::lastError() const
{
    return d_ptr->m_lastError;
}

void AbstractSecretsStorage::clearError()
{
    d_ptr->m_lastError.clear();
}

void AbstractSecretsStorage::setIsOpen(bool isOpen)
{
    d_ptr->m_isOpen = isOpen;
}

void AbstractSecretsStorage::setLastError(const CredentialsDBError &error)
{
    d_ptr->m_lastError = error;
}
