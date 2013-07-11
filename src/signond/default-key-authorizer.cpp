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

#include "default-key-authorizer.h"

using namespace SignonDaemonNS;

DefaultKeyAuthorizer::DefaultKeyAuthorizer(SignOn::KeyHandler *keyHandler,
                                           QObject *parent):
    AbstractKeyAuthorizer(keyHandler, parent)
{
}

DefaultKeyAuthorizer::~DefaultKeyAuthorizer()
{
}

void DefaultKeyAuthorizer::queryKeyAuthorization(const SignOn::Key &key,
                                                 Reason reason)
{
    Q_UNUSED(reason);

    int result = keyHandler()->canAddKeyAuthorization() ?
        Approved : Exclusive;

    emit keyAuthorizationQueried(key, result);
}

