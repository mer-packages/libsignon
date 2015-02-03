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

#include "plugin.h"
#include "access-control-manager.h"

#include <QDebug>

using namespace SignOn;

Plugin::Plugin(QObject *parent):
    QObject(parent)
{
    setObjectName(QLatin1String("mock-ac"));
}

AbstractAccessControlManager *
Plugin::accessControlManager(QObject *parent) const
{
    qDebug() << Q_FUNC_INFO;
    return new AccessControlManager(parent);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(mock-ac, Plugin);
#endif
