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
/*!
 * @copyright Copyright (C) 2011 Nokia Corporation.
 * @license LGPL
 */

#ifndef SIGNON_DEFAULT_KEY_AUTHORIZER_H
#define SIGNON_DEFAULT_KEY_AUTHORIZER_H

#include "SignOn/AbstractKeyAuthorizer"

#include <QObject>

namespace SignonDaemonNS {

/*!
 * @class DefaultKeyAuthorizer
 *
 * Implements a default key authorizer, which authorizes all given keys.
 */
class DefaultKeyAuthorizer: public SignOn::AbstractKeyAuthorizer
{
    Q_OBJECT

public:
    explicit DefaultKeyAuthorizer(SignOn::KeyHandler *keyHandler,
                                  QObject *parent = 0);
    ~DefaultKeyAuthorizer();

    /* reimplemented from KeyAuthorizer */
    void queryKeyAuthorization(const SignOn::Key &key, Reason reason);
};

} // namespace

#endif // SIGNON_DEFAULT_KEY_AUTHORIZER_H
