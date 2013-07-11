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

#ifndef SIGNON_DEBUG_H
#define SIGNON_DEBUG_H

#include <SignOn/export.h>

#ifdef SIGNON_TRACE
    #ifdef TRACE
        #undef TRACE
    #endif

    #ifdef BLAME
        #undef BLAME
    #endif

    #include <QDebug>

    #ifdef DEBUG_ENABLED
        /* 0 - fatal, 1 - critical(default), 2 - info/debug */
        extern int signonLoggingLevel;

        static inline bool debugEnabled()
        {
            return signonLoggingLevel >= 2;
        }

        static inline bool criticalsEnabled()
        {
            return signonLoggingLevel >= 1;
        }

        #define TRACE() \
            if (debugEnabled()) qDebug() << __FILE__ << __LINE__ << __func__
        #define BLAME() \
            if (criticalsEnabled()) qCritical() << __FILE__ << __LINE__ << __func__
    #else
        static inline bool debugEnabled() { return false; }
        static inline bool criticalsEnabled() { return false; }
        #define TRACE() while (0) qDebug()
        #define BLAME() while (0) qDebug()
    #endif
#endif

namespace SignOn {

void setLoggingLevel(int level);

};

#endif // SIGNON_DEBUG_H
