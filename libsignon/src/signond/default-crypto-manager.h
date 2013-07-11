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
 * Definition of the DefaultCryptoManager object.
 * @ingroup Accounts_and_SSO_Framework
 */

#ifndef SIGNON_DEFAULT_CRYPTO_MANAGER_H
#define SIGNON_DEFAULT_CRYPTO_MANAGER_H

#include "SignOn/abstract-crypto-manager.h"

#include <QObject>

namespace SignonDaemonNS {

/*!
 * @class DefaultCryptoManager
 * Dummy implementation of a manager for the credentials storage file system
 * encryption.
 * It always reports that the file system is mounted, without actually doing
 * anything.
 * @ingroup Accounts_and_SSO_Framework
 */
class DefaultCryptoManager: public SignOn::AbstractCryptoManager
{
    Q_OBJECT

public:
    explicit DefaultCryptoManager(QObject *parent = 0);
    ~DefaultCryptoManager();

    /* reimplemented virtual methods */
    virtual bool initialize(const QVariantMap &configuration);
    QString fileSystemMountPath() const;

private:
    QString m_storagePath;
};

} //namespace

#endif // SIGNON_DEFAULT_CRYPTO_MANAGER_H
