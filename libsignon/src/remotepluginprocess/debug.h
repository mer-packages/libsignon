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

#ifndef REMOTEPLUGINPROCESS_DEBUG_H
#define REMOTEPLUGINPROCESS_DEBUG_H

#include <QDebug>

#ifdef DEBUG_ENABLED
    extern int debugLevel;

    #define TRACE() \
        if (debugLevel >= 2) qDebug() << __FILE__ << __LINE__ << __func__
    #define BLAME() \
        if (debugLevel >= 1) qCritical() << __FILE__ << __LINE__ << __func__
#else
    #define TRACE() while (0) qDebug()
    #define BLAME() while (0) qDebug()
#endif

void debugInit();

#endif /* REMOTEPLUGINPROCESS_DEBUG_H */
