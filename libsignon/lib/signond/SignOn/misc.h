/*
 * This file is part of signon
 *
 * Copyright (C) 2011 Nokia Corporation.
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

#ifndef SIGNON_MISC_H
#define SIGNON_MISC_H

#include <QFile>

namespace SignOn {

/* Desired permissions for the files created by signond */
const QFile::Permissions signonFilePermissions =
    QFile::WriteUser | QFile::ReadUser;

/* Sets user UID and GID for a file. */
bool setUserOwnership(const QString &filePath);

bool setFilePermissions(const QString &filePath,
                        const QFile::Permissions permissions,
                        bool keepExisting = true);
}

#endif // SIGNON_MISC_H
