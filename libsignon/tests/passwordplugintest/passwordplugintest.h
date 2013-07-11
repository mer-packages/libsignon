/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef PASSWORD_PLUGIN_TEST
#define PASSWORD_PLUGIN_TEST

#include <QString>
#include "passwordplugin.h"

using namespace PasswordPluginNS;

class PasswordPluginTest: public QObject
{
    Q_OBJECT

public slots:
    void result(const SignOn::SessionData &data);
    void pluginError(const SignOn::Error &err);
    void uiRequest(const SignOn::UiSessionData &data);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    //test cases
    void testPlugin();
    void testPluginType();
    void testPluginMechanisms();
    void testPluginCancel();
    void testPluginProcess();
    void testPluginUserActionFinished();
    void testPluginRefresh();
    //end test cases

private:
    PasswordPlugin *m_testPlugin;
    int m_error;
    SignOn::SessionData m_response;
    SignOn::UiSessionData m_uiResponse;
    QEventLoop m_loop;
};

#endif //PASSWORD_PLUGIN_TEST
