/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of signon
 *
 * Copyright (C) 2010 Nokia Corporation.
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

#include "cryptsetup-plugin.h"
#include "crypto-manager.h"

#include <QDebug>

using namespace SignOn;

CryptsetupPlugin::CryptsetupPlugin():
    QObject(0)
{
    setObjectName(QLatin1String("cryptsetup"));
}

AbstractCryptoManager *CryptsetupPlugin::cryptoManager(QObject *parent) const
{
    qDebug() << Q_FUNC_INFO;
    return new CryptoManager(parent);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(cryptsetup, CryptsetupPlugin);
#endif
