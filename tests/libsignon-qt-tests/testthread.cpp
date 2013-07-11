/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 *
 * Contact: Aurel Popirtac <ext-aurel.popirtac@nokia.com>
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

#include "testthread.h"

#include "SignOn/authservice.h"

#include <QTimer>
#include <QEventLoop>

using namespace SignOn;

void TestThread::run()
{
    /*
       Just call any method on the SignOn API so that the session bus
       connection is created in this thread, a different one from the
       main thread.
    */
    AuthService service;
    QEventLoop loop;

    connect(&service,
            SIGNAL(methodsAvailable(const QStringList &)),
            SIGNAL(testCompleted()));
    connect(&service, SIGNAL(error(const SignOn::Error &)),
            SIGNAL(testCompleted()));

    connect(this, SIGNAL(testCompleted()), &loop, SLOT(quit()));

    service.queryMethods();

    QTimer::singleShot(g_testThreadTimeout, &loop, SLOT(quit()));
    loop.exec();
}
