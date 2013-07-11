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

#include <QtTest/QtTest>

#include "passwordplugin.h"
#include "passwordplugin.cpp"

#include "passwordplugintest.h"

using namespace PasswordPluginNS;

#define TEST_START qDebug("\n\n\n\n ----------------- %s ----------------\n\n",  __func__);

#define TEST_DONE  qDebug("\n\n ----------------- %s DONE ----------------\n\n",  __func__);

void PasswordPluginTest::initTestCase()
{
    TEST_START
    qRegisterMetaType<SignOn::SessionData>();
    qRegisterMetaType<SignOn::UiSessionData>();

    TEST_DONE
}

void PasswordPluginTest::cleanupTestCase()
{
    TEST_START

    TEST_DONE
}

//prepare each test by creating new plugin
void PasswordPluginTest::init()
{
    m_testPlugin = new PasswordPlugin();
}

//finnish each test by deleting plugin
void PasswordPluginTest::cleanup()
{
    delete m_testPlugin;
    m_testPlugin=NULL;
}

//slot for receiving result
void PasswordPluginTest::result(const SignOn::SessionData &data)
{
    qDebug() << "got result";
    m_response = data;
    m_loop.exit();
}

//slot for receiving error
void PasswordPluginTest::pluginError(const SignOn::Error &err)
{
    qDebug() << "got error" << err.type() << ": " << err.message();
    m_error = err.type();
    m_loop.exit();
}

//slot for receiving result
void PasswordPluginTest::uiRequest(const SignOn::UiSessionData &data)
{
    qDebug() << "got ui request";
    m_uiResponse = data;
    m_loop.exit();
}
//test cases

void PasswordPluginTest::testPlugin()
{
    TEST_START

    qDebug() << "Checking plugin integrity.";
    QVERIFY(m_testPlugin);

    TEST_DONE
}

void PasswordPluginTest::testPluginType()
{
    TEST_START

    qDebug() << "Checking plugin type.";
    QCOMPARE(m_testPlugin->type(), QString("password"));

    TEST_DONE
}

void PasswordPluginTest::testPluginMechanisms()
{
    TEST_START

    qDebug() << "Checking plugin mechanisms.";
    QStringList mechs = m_testPlugin->mechanisms();
    QVERIFY(!mechs.isEmpty());
    QVERIFY(mechs.contains(QString("password")));
    qDebug() << mechs;

    TEST_DONE
}

void PasswordPluginTest::testPluginCancel()
{
    TEST_START
//no cancel functionality
    TEST_DONE
}

void PasswordPluginTest::testPluginProcess()
{
    TEST_START

    SignOn::SessionData info;
    SignOn::SessionData info2;

    QObject::connect(m_testPlugin, SIGNAL(result(const SignOn::SessionData&)),
                     this, SLOT(result(const SignOn::SessionData&)),
                     Qt::QueuedConnection);
    QObject::connect(m_testPlugin, SIGNAL(error(const SignOn::Error & )),
                     this, SLOT(pluginError(const SignOn::Error & )),
                     Qt::QueuedConnection);
    QObject::connect(m_testPlugin,
                     SIGNAL(userActionRequired(const SignOn::UiSessionData&)),
                     this,  SLOT(uiRequest(const SignOn::UiSessionData&)),
                     Qt::QueuedConnection);
    QTimer::singleShot(10*1000, &m_loop, SLOT(quit()));

    //try with password
    info.setSecret(QString("pass"));

    m_testPlugin->process(info);
    m_loop.exec();
    QVERIFY(m_response.Secret() == QString("pass"));
    QVERIFY(m_response.UserName().isEmpty());

    //try with username
    info.setUserName(QString("user"));

    m_testPlugin->process(info);
    m_loop.exec();
    QVERIFY(m_response.Secret() == QString("pass"));
    QVERIFY(m_response.UserName() == QString("user"));

    //try without params to open ui
    m_testPlugin->process(info2);
    m_loop.exec();
    QVERIFY(m_uiResponse.QueryUserName() );
    QVERIFY(m_uiResponse.QueryPassword() );

    //try without password to open ui
    info2.setUserName(QString("user"));

    m_testPlugin->process(info2);
    m_loop.exec();
    QVERIFY(!m_uiResponse.QueryUserName() );
    QVERIFY(m_uiResponse.QueryPassword() );

    TEST_DONE
}

void PasswordPluginTest::testPluginUserActionFinished()
{
    TEST_START

    SignOn::UiSessionData info;

    QObject::connect(m_testPlugin, SIGNAL(result(const SignOn::SessionData&)),
                     this, SLOT(result(const SignOn::SessionData&)),
                     Qt::QueuedConnection);
    QObject::connect(m_testPlugin, SIGNAL(error(const SignOn::Error & )),
                     this, SLOT(pluginError(const SignOn::Error & )),
                     Qt::QueuedConnection);
    QObject::connect(m_testPlugin,
                     SIGNAL(userActionRequired(const SignOn::UiSessionData&)),
                     this, SLOT(uiRequest(const SignOn::UiSessionData&)),
                     Qt::QueuedConnection);
    QTimer::singleShot(10*1000, &m_loop, SLOT(quit()));

    //empty data
    m_testPlugin->userActionFinished(info);
    m_loop.exec();
    //fails because of ui error 0 is returned if not defined
//    QVERIFY(m_error == PLUGIN_ERROR_USER_INTERACTION);

    //correct values
    info.setQueryErrorCode(QUERY_ERROR_NONE);
    info.setUserName(QString("user") );
    info.setSecret(QString("pass") );
    m_testPlugin->userActionFinished(info);
    m_loop.exec();
    QVERIFY(m_response.UserName() == QString("user") );
    QVERIFY(m_response.Secret() == QString("pass") );

    //user canceled
    info.setQueryErrorCode(QUERY_ERROR_CANCELED);
    m_testPlugin->userActionFinished(info);
    m_loop.exec();
    QVERIFY(m_error == Error::SessionCanceled);

    //error in ui request
    info.setQueryErrorCode(QUERY_ERROR_GENERAL);
    m_testPlugin->userActionFinished(info);
    m_loop.exec();
    QVERIFY(m_error == Error::UserInteraction);

    TEST_DONE
}

void PasswordPluginTest::testPluginRefresh()
{
    TEST_START
    SignOn::UiSessionData info;

    QObject::connect(m_testPlugin, SIGNAL(error(const SignOn::Error &)),
                     this, SLOT(pluginError(const SignOn::Error &)),
                     Qt::QueuedConnection);
    QObject::connect(m_testPlugin,
                     SIGNAL(refreshed(const SignOn::UiSessionData&)),
                     this, SLOT(uiRequest(const SignOn::UiSessionData&)),
                     Qt::QueuedConnection);
    QTimer::singleShot(10*1000, &m_loop, SLOT(quit()));

    m_testPlugin->refresh(info);
    m_loop.exec();

    TEST_DONE
}

//end test cases

QTEST_MAIN(PasswordPluginTest)
