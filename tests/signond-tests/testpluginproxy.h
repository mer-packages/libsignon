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

#ifndef TESTPLUGINPROXY_H_
#define TESTPLUGINPROXY_H_

#include <QtTest/QtTest>
#include <QtCore>

#include "signond/signoncommon.h"
#include "SignOn/sessiondata.h"
#include "SignOn/authpluginif.h"
#include "pluginproxy.h"

using namespace SignonDaemonNS;
using namespace SignOn;

class TestPluginProxy: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void create_nonexisting();
    void create_dummy();
    void type_for_dummy();
    void mechanisms_for_dummy();
    void process_for_dummy();
    void processUi_for_dummy();
    void process_wrong_mech_for_dummy();
    void process_and_cancel_for_dummy();
    void wrong_user_for_dummy();

private:
    PluginProxy *m_proxy;
};

#endif //TESTPLUGINPROXY_H_
