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

#include "abstract-key-authorizer.h"

using namespace SignOn;

namespace SignOn {

class AbstractKeyAuthorizerPrivate
{
    Q_DECLARE_PUBLIC(AbstractKeyAuthorizer)

public:
    AbstractKeyAuthorizerPrivate(KeyHandler *keyHandler,
                                 AbstractKeyAuthorizer *authorizer);
    ~AbstractKeyAuthorizerPrivate() {};

private:
    mutable AbstractKeyAuthorizer *q_ptr;
    KeyHandler *m_keyHandler;
};
};

AbstractKeyAuthorizerPrivate::AbstractKeyAuthorizerPrivate(
    KeyHandler *keyHandler, AbstractKeyAuthorizer *authorizer):
    q_ptr(authorizer),
    m_keyHandler(keyHandler)
{
}

AbstractKeyAuthorizer::AbstractKeyAuthorizer(KeyHandler *keyHandler,
                                             QObject *parent):
    QObject(parent),
    d_ptr(new AbstractKeyAuthorizerPrivate(keyHandler, this))
{
}

AbstractKeyAuthorizer::~AbstractKeyAuthorizer()
{
    delete d_ptr;
}

KeyHandler *AbstractKeyAuthorizer::keyHandler() const
{
    Q_D(const AbstractKeyAuthorizer);
    return d->m_keyHandler;
}

void AbstractKeyAuthorizer::queryKeyAuthorization(const SignOn::Key &key,
                                                  Reason reason)
{
    Q_UNUSED(reason);
    emit keyAuthorizationQueried(key, Denied);
}

