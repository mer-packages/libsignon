/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
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
#ifndef SSOTEST2DATA_H
#define SSOTEST2DATA_H

#include "SignOn/sessiondata.h"

namespace SsoTest2PluginNS {

class SsoTest2Data: public SignOn::SessionData
{
public:
    SIGNON_SESSION_DECLARE_PROPERTY(QStringList, ChainOfStates);
    SIGNON_SESSION_DECLARE_PROPERTY(quint32, CurrentState);
    SIGNON_SESSION_DECLARE_PROPERTY(QStringList, ChainOfResults);
};

}  // namespace SsoTest2PluginNS

#endif // SSOTEST2DATA_H
