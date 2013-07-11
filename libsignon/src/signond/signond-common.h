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

#ifndef SIGNOND_COMMON_H_
#define SIGNOND_COMMON_H_

#include "signond/signoncommon.h"

#ifdef TRACE
    #undef TRACE
#endif

#ifdef BLAME
    #undef BLAME
#endif

#ifdef SIGNOND_TRACE
    #define SIGNON_TRACE
    #include "SignOn/Debug"
#endif

#ifdef TESTS_TRACE
    #define TRACE() \
        qDebug() << __FILE__ << __LINE__ << __func__
    #define BLAME() \
        qCritical() << __FILE__ << __LINE__ << __func__
#endif

/*
 * Idle timeout for remote identities and their plugin processes
 * */
#define SIGNOND_MAX_IDLE_TIME 300

/*
 * Signon UI DBUS defs
 * */
#define SIGNON_UI_SERVICE           QLatin1String("com.nokia.singlesignonui")
#define SIGNON_UI_DAEMON_OBJECTPATH QLatin1String("/SignonUi")

/*
 * Signon Daemon default configuration values
 */
const char signonDefaultDbName[] = "signon.db";
const char signonDefaultSecretsDbName[] = "signon-secrets.db";
const bool signonDefaultUseEncryption = true;
const char signonDefaultStoragePath[] = "~/.config/signond";

#endif // SIGNOND_COMMON_H_
