/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of signon
 *
 * Copyright (C) 2011 Intel Corporation.
 *
 * Contact: Elena Reshetova <elena.reshetova@intel.com>
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

#include "mssfac-plugin.h"
#include "mssf-access-control-manager.h"

#include <QDebug>

using namespace SignOn;

MSSFAccessControlPlugin::MSSFAccessControlPlugin():
    QObject(0)
{
    setObjectName(QLatin1String("mssf-ac"));
}

AbstractAccessControlManager *
MSSFAccessControlPlugin::accessControlManager(QObject *parent) const
{
    qDebug() << Q_FUNC_INFO;
    return new MSSFAccessControlManager(parent);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(mssf-ac, MSSFAccessControlPlugin);
#endif
