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

#include "extension-interface.h"

using namespace SignOn;

AbstractKeyManager *
ExtensionInterface::keyManager(QObject *parent) const
{
    Q_UNUSED(parent);
    return 0;
}

AbstractKeyAuthorizer *
ExtensionInterface2::keyAuthorizer(KeyHandler *keyHandler,
                                   QObject *parent) const
{
    Q_UNUSED(keyHandler);
    Q_UNUSED(parent);
    return 0;
}

AbstractCryptoManager *
ExtensionInterface3::cryptoManager(QObject *parent) const
{
    Q_UNUSED(parent);
    return 0;
}

AbstractSecretsStorage *
ExtensionInterface3::secretsStorage(QObject *parent) const
{
    Q_UNUSED(parent);
    return 0;
}

AbstractAccessControlManager *
ExtensionInterface3::accessControlManager(QObject *parent) const
{
    Q_UNUSED(parent);
    return 0;
}
