/*
 * This file is part of signon
 *
 * Copyright (C) 2010 Nokia Corporation.
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

#include "abstract-key-manager.h"

using namespace SignOn;

AbstractKeyManager::AbstractKeyManager(QObject *parent):
    QObject(parent)
{
}

AbstractKeyManager::~AbstractKeyManager()
{
}

void AbstractKeyManager::authorizeKey(const Key &key,
                                      const QString &message)
{
    Q_UNUSED(message);
    emit keyAuthorized(key, false);
}

void AbstractKeyManager::queryKeys()
{
    emit keyInserted(QByteArray());
}

QString AbstractKeyManager::describeKey(const Key &key)
{
    Q_UNUSED(key);
    return QString();
}

bool AbstractKeyManager::supportsKeyAuthorization() const
{
    return false;
}
