/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of signon
 *
 * Copyright (C) 2009-2010 Nokia Corporation.
 * Copyright (C) 2013 Canonical Ltd.
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

#ifndef TESTAUTHSESSION_H_
#define TESTAUTHSESSION_H_

#include <QtTest/QtTest>
#include <QtCore>

#include <sys/types.h>
#include <errno.h>

#include <signal.h>

#include "signond/signoncommon.h"
#include "SignOn/authservice.h"

#include "ssotest2data.h"
#include "SignOn/uisessiondata.h"

/*
 * here we test the implementation because of difficulties of having
 *  access to protected/private functions
 * */
#include "SignOn/authsessionimpl.h"

using namespace SignOn;

/*
 * test timeout 10 seconds
 * */
#define test_timeout 10000

class SignOnUI;

class TestAuthSession: public QObject
{
    Q_OBJECT

public:
    TestAuthSession(SignOnUI *signOnUi, QObject *parent = 0);

private Q_SLOTS:
    /*
     * Start the signon daemon
     */
    void initTestCase();

    /*
     * End the signon daemon
     */
    void cleanupTestCase();

    /*
     * UIless
     * AuthSession API related test cases
     */
    void sessionData();
    void queryMechanisms_existing_method();
    void queryMechanisms_nonexisting_method();

    void process_with_new_identity();
    void process_with_existing_identity();
    void process_with_nonexisting_type();
    void process_with_nonexisting_method();
    void process_with_unauthorized_method();
    void process_from_other_process();
    void process_many_times_after_auth();
    void process_many_times_before_auth();
    void process_with_big_session_data();
    void process_after_timeout();

    void cancel_immediately();
    void cancel_with_delay();
    void cancel_without_process();

    void handle_destroyed_signal();

    void multi_thread_test();

    void processUi_with_existing_identity();
    void processUi_and_cancel();
    void windowId();

private Q_SLOTS:
    void cancel();
    void response(const SignOn::SessionData &data);

private:
    SignOnUI *m_signOnUI;
};

/*
 * The SlotMachine class is used by process_from_other_process
 * test to receive signals regarding the completion of D-Bus
 * calls. The QSignalSpy classes cannot be used because the
 * class for the D-Bus methods must have both response method
 * and error method.
 */
class SlotMachine: public QObject
{
    Q_OBJECT
public:
    SlotMachine(): m_responseReceived(false) {}
public Q_SLOTS:
    void authenticationSlot(const QString &path) {
        m_path = path;
        emit done();
    }

    void errorSlot(const QDBusError &err) {
        m_errorMessage = err.message();
        m_errorName = err.name();
        emit done();
    }

    void responseSlot(const QVariantMap &response) {
        m_response = response;
        m_responseReceived = true;
        emit done();
    }

Q_SIGNALS:
    void done();

public:
    QString m_path;
    QString m_errorMessage;
    QString m_errorName;
    QVariantMap m_response;
    bool m_responseReceived;
};

#endif
