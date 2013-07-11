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
#include "testauthserviceresult.h"

#include <QDebug>


#define SIGNOND_TEST_REPLY_RECEIVED qDebug() << "Reply from SIGNON DAEMON---------------------------------" << __FUNCTION__;

TestAuthServiceResult::TestAuthServiceResult()
{
    reset();
}

void TestAuthServiceResult::reset()
{
    m_responseReceived = InexistentResp;
    m_error = Error::Unknown;
    m_errMsg = QString();

    m_identities.clear();
    m_methods.clear();
    m_mechanisms.first = QString();
    m_mechanisms.second.clear();
    m_cleared = false;
}

void TestAuthServiceResult::error(const SignOn::Error &error)
{
    SIGNOND_TEST_REPLY_RECEIVED
    m_responseReceived = ErrorResp;
    m_error = (Error::ErrorType)error.type();
    m_errMsg = error.message();

    qDebug() << "Error:" << m_error << ", Message:" << m_errMsg;

    emit testCompleted();
}

void TestAuthServiceResult::methodsAvailable(const QStringList &methods)
{
    SIGNOND_TEST_REPLY_RECEIVED
    m_responseReceived = NormalResp;
    m_methods = methods;

    emit testCompleted();
}

void TestAuthServiceResult::mechanismsAvailable(const QString &method,
                                                const QStringList &mechanisms)
{
    SIGNOND_TEST_REPLY_RECEIVED
    m_responseReceived = NormalResp;
    m_mechanisms.first = method;
    m_mechanisms.second = mechanisms;
    m_queriedMechsMethod = method;

    emit testCompleted();
}

void TestAuthServiceResult::identities(
                               const QList<SignOn::IdentityInfo> &identityList)
{
    SIGNOND_TEST_REPLY_RECEIVED
    m_responseReceived = NormalResp;
    m_identities = identityList;

    emit testCompleted();
}

void TestAuthServiceResult::cleared()
{
    SIGNOND_TEST_REPLY_RECEIVED
    m_responseReceived = NormalResp;
    m_cleared = true;

    emit testCompleted();
}
