/*
 * This file is part of signon
 *
 * Copyright (C) 2013 Canonical Ltd.
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

#ifndef SIGNON_MOCK_PLUGIN_H
#define SIGNON_MOCK_PLUGIN_H

#include <QObject>
#include <SignOn/ExtensionInterface>

class Plugin: public QObject, public SignOn::ExtensionInterface3
{
    Q_OBJECT
    Q_INTERFACES(SignOn::ExtensionInterface3)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "com.nokia.SingleSignOn.ExtensionInterface/3.0")
#endif

public:
    Plugin(QObject *parent = 0);

    SignOn::AbstractAccessControlManager *
        accessControlManager(QObject *parent = 0) const;
};

#endif // SIGNON_MOCK_PLUGIN_H
